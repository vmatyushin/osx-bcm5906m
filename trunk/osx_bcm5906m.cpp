#include "osx_bcm5906m.h"
#include "osx_bcm5906m_reg.h"

#define super IOEthernetController

OSDefineMetaClassAndStructors(BCM5906MEthernet, IOEthernetController);

#pragma mark -
#pragma mark Start/Stop/Free
#pragma mark -

bool BCM5906MEthernet::start(IOService *provider)
{
    bool success = false;
    bool started = false;

    do
    {
        if (super::start(provider) != true)
        {
            DLOG("super::start() failed.");
            break;
        }
        started = true;

        mPCIDevice = OSDynamicCast(IOPCIDevice, provider);
        if (mPCIDevice == 0)
            break;

        mPCIDevice->retain();
        if (mPCIDevice->open(this) == false)
            break;

        if (initDriverObjects(provider) == false)
        {
            DLOG("initDriverObjects() failed.");
            break;
        }

        if (allocateTxMemory() == false)
        {
            DLOG("Can't allocate TX memory.");
            break;
        }

        if (allocateRxMemory() == false)
        {
            DLOG("Can't allocate RX memory.");
            break;
        }

        if (allocateStatusBlockMemory() == false)
        {
            DLOG("Can't allocate status block memory.");
            break;
        }

        initPCIConfigSpace(mPCIDevice);

        mMemoryMap = mPCIDevice->mapDeviceMemoryWithRegister(kIOPCIConfigBaseAddress0, kIOMapInhibitCache);
        if (mMemoryMap == NULL)
        {
            DLOG("mapDeviceMemoryWithRegister() failed.");
            break;
        }
        mNICBaseAddr = mMemoryMap->getVirtualAddress();

        resetChip();
        initChip();

        if (readEthernetAddress() == false)
        {
            DLOG("Failed to read MAC address.");
            break;
        }

        if (phyInit() == false)
        {
            DLOG("Failed to init PHY.");
            break;
        }

        publishMedium();
        if (publishMediumDictionary(mMediumDict) == false)
        {
            DLOG("Failed to publish media support.");
            break;
        }

        success = true;
    } while (0);

    do
    {
        if (success == false)
            break;
        success = false;
        if (attachInterface((IONetworkInterface **) &mNetworkInterface, true) == false)
            break;
        success = true;
    } while (0);

    if (started && !success)
        stop(provider);

    return true;
}

void BCM5906MEthernet::stop(IOService *provider)
{
    stopChip();
    resetChip();
    super::stop(provider);
}

void BCM5906MEthernet::free(void)
{
    if (mWorkLoop && mInterruptSrc)
    {
        mInterruptSrc->disable();
        mWorkLoop->removeEventSource(mInterruptSrc);
    }

    if (mWorkLoop && mTimerSrc)
    {
        mTimerSrc->cancelTimeout();
        mWorkLoop->removeEventSource(mTimerSrc);
    }

    if (mPCIDevice)
        mPCIDevice->close(this);

    for (int i = 0; i < mMediumTableSize; i++)
        if (mMediumTable[i] != 0)
            mMediumTable[i]->release();

    RELEASE(mMediumDict);
    RELEASE(mInterruptSrc);
    RELEASE(mTimerSrc);
    RELEASE(mMemoryCursor);
    RELEASE(mMemoryMap);
    RELEASE(mNetworkInterface);
    RELEASE(mPCIDevice);
    RELEASE(mWorkLoop);

    releaseTxMemory();
    releaseRxMemory();
    releaseStatusBlockMemory();

    return super::free();
}

#pragma mark -
#pragma mark Enable/Disable
#pragma mark -

IOReturn BCM5906MEthernet::enable(IONetworkInterface *netif)
{
    initBlock();

    mTransmitQueue->setCapacity(1024);
    mTransmitQueue->start();

    initTxRing();
    if (!initRxRing())
        return kIOReturnIOError;

    selectMedium(getCurrentMedium());
    phyGetLinkStatus(true);

    mTimerSrc->setTimeoutMS(timerInterval);

    // Start autonegotiation.
    UInt32 miiControl = miiReadReg(BGE_MII_CTL);
    miiControl |= BGE_MII_CTL_AUTONEG_ENABLE;
    miiControl |= BGE_MII_CTL_RESTART_AUTONEG;
    miiWriteReg(BGE_MII_CTL, miiControl);
    miiWriteReg(BGE_MII_INTERRUPT, 0xFF00);

    return kIOReturnSuccess;
}

IOReturn BCM5906MEthernet::disable(IONetworkInterface *netif)
{
    mTimerSrc->cancelTimeout();

    resetChip();
    initChip();

    setLinkStatus(kIONetworkLinkValid);
    IOSleep(20);

    mTransmitQueue->setCapacity(0);
    mTransmitQueue->flush();
    mTransmitQueue->stop();

    return kIOReturnSuccess;
}

#pragma mark -
#pragma mark Driver initialization
#pragma mark -

void BCM5906MEthernet::initPCIConfigSpace(IOPCIDevice *pci)
{
    UInt16 cmd = pci->configRead16(kIOPCIConfigCommand);

    cmd |= kIOPCICommandBusMaster | kIOPCICommandMemorySpace;
    // Disable IO space.
    cmd &= ~kIOPCICommandIOSpace;
    // Disable memory write invalidate.
    cmd &= ~kIOPCICommandMemWrInvalidate;
    pci->configWrite16(kIOPCIConfigCommand, cmd);
}

bool BCM5906MEthernet::initDriverObjects(IOService *provider)
{
    // When transmit ring is full, packets are queued here.
    mTransmitQueue = getOutputQueue();
    if (mTransmitQueue == 0)
    {
        DLOG ("No transmit queue was created.");
        return false;
    }

    // Get our work loop.
    IOWorkLoop *workLoop = (IOWorkLoop *) getWorkLoop();
    if (!workLoop)
    {
        DLOG("No work loop was created.");
        return false;
    }

    // Create a mbuf cursor.
    mMemoryCursor = IOMbufNaturalMemoryCursor::withSpecification(BGE_MAX_FRAMELEN, 1);
    if (!mMemoryCursor)
    {
        DLOG("Mbuf cursor allocation error.");
        return false;
    }

    // Attach an interrupt event source to our work loop.
    mInterruptSrc = IOFilterInterruptEventSource::filterInterruptEventSource(this,
                                                                             OSMemberFunctionCast(IOInterruptEventSource::Action,
                                                                                                  this,
                                                                                                  &BCM5906MEthernet::interruptHandler),
                                                                             OSMemberFunctionCast(IOFilterInterruptEventSource::Filter,
                                                                                                  this,
                                                                                                  &BCM5906MEthernet::interruptFilter),
                                                                             provider);
    if (!mInterruptSrc || (workLoop->addEventSource(mInterruptSrc) != kIOReturnSuccess))
    {
        DLOG("IOInterruptEventSource error.");
        return false;
    }
    mInterruptSrc->enable();

    // Attach a timer event source to our work loop.
    mTimerSrc = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action,
                                                                                this,
                                                                                &BCM5906MEthernet::timeoutHandler));
    if (!mTimerSrc || (workLoop->addEventSource(mTimerSrc) != kIOReturnSuccess))
    {
        DLOG("IOTimerEventSource error.");
        return false;
    }

    mMediumDict = OSDictionary::withCapacity(6);
    if(!mMediumDict)
    {
        DLOG("Failed to create medium dictionary.");
        return false;
    }

    return true;
}

bool BCM5906MEthernet::configureInterface(IONetworkInterface *netif)
{
    IONetworkData *data;

    if (super::configureInterface(netif) == false)
        return false;

    // Get the generic network statistics structure.
    data = netif->getParameter(kIONetworkStatsKey);
    if (!data || !(mNetStats = (IONetworkStats *) data->getBuffer()))
        return false;

    // Get the Ethernet statistics structure.
    data = netif->getParameter(kIOEthernetStatsKey);
    if (!data || !(mEtherStats = (IOEthernetStats *) data->getBuffer()))
        return false;

    return true;
}

IOOutputQueue *BCM5906MEthernet::createOutputQueue(void)
{
    return IOGatedOutputQueue::withTarget(this, getWorkLoop());
}

bool BCM5906MEthernet::createWorkLoop(void)
{
    mWorkLoop = IOWorkLoop::workLoop();
    return mWorkLoop != NULL;
}

IOWorkLoop *BCM5906MEthernet::getWorkLoop(void) const
{
    return mWorkLoop;
}

void BCM5906MEthernet::getPacketBufferConstraints(IOPacketBufferConstraints *constraints) const
{
    constraints->alignStart  = kIOPacketBufferAlign1;
    constraints->alignLength = kIOPacketBufferAlign1;
    return;
}

#pragma mark -
#pragma mark Vendor/Model
#pragma mark -

const OSString *BCM5906MEthernet::newVendorString(void) const
{
    return OSString::withCString("Broadcom Corporation");
}

const OSString *BCM5906MEthernet::newModelString(void) const
{
    return OSString::withCString("NetLink BCM5906M Fast Ethernet PCI Express");
}

#pragma mark -
#pragma mark Status block
#pragma mark -

bool BCM5906MEthernet::allocateStatusBlockMemory()
{
    mStatusBlockDesc = IOBufferMemoryDescriptor::withOptions(kIOMemoryPhysicallyContiguous,
                                                             sizeof(bcmStatusBlock));
    if (mStatusBlockDesc == 0)
    {
        DLOG("No memory for status block.");
        return false;
    }

    if (mStatusBlockDesc->prepare() != kIOReturnSuccess)
    {
        DLOG("Status block memory prepare() failed.");
        mStatusBlockDesc->release();
        mStatusBlockDesc = 0;
        return false;
    }

    mStatusBlockPhysAddr = mStatusBlockDesc->getPhysicalSegment(0, 0);
    if (mStatusBlockPhysAddr == 0)
    {
        DLOG("Status block getPhysicalSegment() failed.");
        return false;
    }

    mStatusBlockAddr = (bcmStatusBlock *) mStatusBlockDesc->getBytesNoCopy();
    memset(mStatusBlockAddr, 0, sizeof(bcmStatusBlock));

    return true;
}

void BCM5906MEthernet::releaseStatusBlockMemory()
{
    if (mStatusBlockDesc)
    {
        mStatusBlockDesc->complete();
        mStatusBlockDesc->release();
        mStatusBlockDesc = 0;
        mStatusBlockAddr = 0;
    }
}

#pragma mark -
#pragma mark MAC Address
#pragma mark -

IOReturn BCM5906MEthernet::getHardwareAddress(IOEthernetAddress *addr)
{
    memcpy(addr, &mEtherAddr, sizeof(*addr));
    return kIOReturnSuccess;
}

IOReturn BCM5906MEthernet::setHardwareAddress(const IOEthernetAddress *addr)
{
    IOReturn success = kIOReturnSuccess;
    memcpy(&mEtherAddr, addr, sizeof(mEtherAddr));
    return success;
}

bool BCM5906MEthernet::readNVRAMByte(UInt32 offset, UInt8 *dest)
{
    UInt32 access, byte = 0;
    int i;

    // Lock.
    writeNICMem(BGE_NVRAM_SWARB, BGE_NVRAMSWARB_SET1);
    for (i = 0; i < 8000; i++)
    {
        if (readNICMem(BGE_NVRAM_SWARB) & BGE_NVRAMSWARB_GNT1)
            break;
        IODelay(20);
    }
    if (i == 8000)
        return false;

    // Enable access.
    access = readNICMem(BGE_NVRAM_ACCESS);
    writeNICMem(BGE_NVRAM_ACCESS, access | BGE_NVRAMACC_ENABLE);

    writeNICMem(BGE_NVRAM_ADDR, offset & 0xfffffffc);
    writeNICMem(BGE_NVRAM_CMD, BGE_NVRAM_READCMD);
    for (i = 0; i < BGE_TIMEOUT * 10; i++)
    {
        IODelay(10);
        if (readNICMem(BGE_NVRAM_CMD) & BGE_NVRAMCMD_DONE)
        {
            IODelay(10);
            break;
        }
    }

    if (i == BGE_TIMEOUT * 10)
        return false;

    // Get result.
    byte = readNICMem(BGE_NVRAM_RDDATA);
    *dest = (OSSwapInt32(byte) >> ((offset % 4) * 8)) & 0xFF;

    // Disable access.
    writeNICMem(BGE_NVRAM_ACCESS, access);

    // Unlock.
    writeNICMem(BGE_NVRAM_SWARB, BGE_NVRAMSWARB_CLR1);
    readNICMem(BGE_NVRAM_SWARB);

    return true;
}

bool BCM5906MEthernet::readEthernetAddress()
{
    bool success;
    for (int i = 0; i < 6; i++)
    {
        success = readNVRAMByte(BGE_EE_MAC_OFFSET_5906 + 2 + i, &(mEtherAddr.bytes[i]));
        if (!success)
            break;
    }
    return success;
}

#pragma mark -
#pragma mark Interrupt/Timer
#pragma mark -

void BCM5906MEthernet::interruptHandler(OSObject *owner, IOInterruptEventSource *sender, int count)
{
    // Read Interrupt Mailbox 0 in order to flush any posted writes in the PCI chipset.
    readRegisterMailbox(BGE_MBX_IRQ0_LO);

    // Disable interrupts.
    writeRegisterMailbox(BGE_MBX_IRQ0_LO, 1);

    // Nothing to examine.
    if (!(mStatusBlockAddr->statusWord & BGE_STATUSWORD_WAS_UPDATED))
        return;

    // Clear the "statusblock was updated" bit as the NIC expects that from us.
    mStatusBlockAddr->statusWord &= ~BGE_STATUSWORD_WAS_UPDATED;

    if (mStatusBlockAddr->statusWord & BGE_STATUSWORD_LINK_CHANGED)
    {
        phyGetLinkStatus(false);
        writeNICMem(BGE_MAC_STS, 0xFFFFFFFF);
    }

    serviceRxInterrupt();
    serviceTxInterrupt();

    // Enable interrupts.
    writeRegisterMailbox(BGE_MBX_IRQ0_LO, 0);

    // Read Interrupt Mailbox 0 in order to flush any posted writes in the PCI chipset.
    readRegisterMailbox(BGE_MBX_IRQ0_LO);

    // There is more work to do. Force another interrupt.
    if (mStatusBlockAddr->statusWord & BGE_STATUSWORD_WAS_UPDATED)
        bcmSetBit(BGE_MISC_LOCAL_CTL, 0x2);
}

bool BCM5906MEthernet::interruptFilter(OSObject *owner, IOFilterInterruptEventSource *sender)
{
    return true;
}

void BCM5906MEthernet::timeoutHandler(OSObject *owner, IOTimerEventSource *sender)
{
    updateStatistics();
    mTimerSrc->setTimeoutMS(timerInterval);
}

void BCM5906MEthernet::updateStatistics()
{
    mNetStats->collisions = readNICMem(BFE_STAT_TX_ETHER_STATS_COLLISIONS);

    mEtherStats->dot3StatsEntry.alignmentErrors = readNICMem(BFE_STAT_RX_DOT3_STATS_ALIGNMENT_ERRORS);
    mEtherStats->dot3StatsEntry.deferredTransmissions = readNICMem(BFE_STAT_TX_DOT3_STATS_DEFERRED_TRANSMISSIONS);
    mEtherStats->dot3StatsEntry.excessiveCollisions = readNICMem(BFE_STAT_TX_DOT3_STATS_EXCESSIVE_COLLISIONS);
    mEtherStats->dot3StatsEntry.fcsErrors = readNICMem(BFE_STAT_RX_DOT3_STATS_FCS_ERRORS);
    mEtherStats->dot3StatsEntry.frameTooLongs = readNICMem(BFE_STAT_RX_DOT3_STATS_FRAMES_TOO_LONG);
    mEtherStats->dot3StatsEntry.internalMacTransmitErrors = readNICMem(BFE_STAT_TX_DOT3_STATS_INTERNAL_MAC_TRANSMIT_ERRORS);
    mEtherStats->dot3StatsEntry.lateCollisions = readNICMem(BFE_STAT_TX_DOT3_STATS_LATE_COLLISIONS);
    mEtherStats->dot3StatsEntry.multipleCollisionFrames = readNICMem(BFE_STAT_TX_DOT3_STATS_MULTIPLE_COLLISION_FRAMES);
    mEtherStats->dot3StatsEntry.singleCollisionFrames = readNICMem(BFE_STAT_TX_DOT3_STATS_SINGLE_COLLISION_FRAMES);
    mEtherStats->dot3RxExtraEntry.frameTooShorts = readNICMem(BFE_STAT_RX_ETHER_STATS_UNDERSIZE_PKTS);
    mEtherStats->dot3TxExtraEntry.jabbers = readNICMem(BFE_STAT_RX_ETHER_STATS_JABBERS);
}

#pragma mark -
#pragma mark Promiscuous/Multicast
#pragma mark -

IOReturn BCM5906MEthernet::setPromiscuousMode(bool active)
{
    if (active)
        bcmSetBit(BGE_RX_MODE, BGE_RXMODE_RX_PROMISC);
    else
        bcmClrBit(BGE_RX_MODE, BGE_RXMODE_RX_PROMISC);

    return kIOReturnSuccess;
}

IOReturn BCM5906MEthernet::setMulticastMode(bool active)
{
    return kIOReturnSuccess;
}

IOReturn BCM5906MEthernet::setMulticastList(IOEthernetAddress *addrList, UInt32 count)
{
    UInt32 hashes[4] = {0};
    UInt32 index, pos, crc;

    if (count)
    {
        for (int i = 0; i < count; i++)
        {
            crc = computeEthernetCRC32(&(addrList[i]));
            pos = ~crc & 0x7f;
            index = (pos & 0x60) >> 5;
            pos &= 0x1f;
            hashes[index] |= (1 << pos);
        }

        writeNICMem(BGE_MAR0, hashes[0]);
        writeNICMem(BGE_MAR1, hashes[1]);
        writeNICMem(BGE_MAR2, hashes[2]);
        writeNICMem(BGE_MAR3, hashes[3]);
    }

    return kIOReturnSuccess;
}

IOReturn BCM5906MEthernet::getChecksumSupport(UInt32 *checksumMask, UInt32 checksumFamily, bool isOutput)
{
    if (checksumFamily != kChecksumFamilyTCPIP)
        return kIOReturnUnsupported;

    *checksumMask = kChecksumIP | kChecksumTCP | kChecksumUDP;

    return kIOReturnSuccess;
}

UInt32 BCM5906MEthernet::computeEthernetCRC32(IOEthernetAddress *addr)
{
    UInt32 reg = 0xffffffff;
    UInt32 tmp;

    for (int i = 0; i < kIOEthernetAddressSize; i++)
    {
        reg ^= addr->bytes[i];
        for (int k = 0; k < 8; k++)
        {
            tmp = reg & 0x01;
            reg >>= 1;
            if (tmp)
                reg ^= 0xedb88320;
        }
    }

    return ~reg;
}

#pragma mark -
#pragma mark Memory Read/Write
#pragma mark -

UInt32 BCM5906MEthernet::readNICMem(UInt32 offset)
{
    return OSReadLittleInt32((void *) mNICBaseAddr, offset);
}

void BCM5906MEthernet::writeNICMem(UInt32 offset, UInt32 data)
{
    OSWriteLittleInt32((void *) mNICBaseAddr, offset, data);
}

UInt32 BCM5906MEthernet::readNICMemIndirect(UInt32 offset)
{
    UInt32 data;

    mPCIDevice->configWrite32(BGE_PCI_MEMWIN_BASEADDR, offset);
    data = mPCIDevice->configRead32(BGE_PCI_MEMWIN_DATA);
    mPCIDevice->configWrite32(BGE_PCI_MEMWIN_BASEADDR, 0);

    return data;
}

void BCM5906MEthernet::writeNICMemIndirect(UInt32 offset, UInt32 data)
{
    mPCIDevice->configWrite32(BGE_PCI_MEMWIN_BASEADDR, offset);
    mPCIDevice->configWrite32(BGE_PCI_MEMWIN_DATA, data);
    mPCIDevice->configWrite32(BGE_PCI_MEMWIN_BASEADDR, 0);
}

void BCM5906MEthernet::writeRegisterIndirect(UInt32 offset, UInt32 data)
{
    mPCIDevice->configWrite32(BGE_PCI_REG_BASEADDR, offset);
    mPCIDevice->configWrite32(BGE_PCI_REG_DATA, data);
}

UInt32 BCM5906MEthernet::readRegisterMailbox(UInt32 offset)
{
    offset += BGE_LPMBX_IRQ0_HI - BGE_MBX_IRQ0_HI;
    return readNICMem(offset);
}

void BCM5906MEthernet::writeRegisterMailbox(UInt32 offset, UInt32 data)
{
    offset += BGE_LPMBX_IRQ0_HI - BGE_MBX_IRQ0_HI;
    writeNICMem(offset, data);
}

#pragma mark -
#pragma mark Power
#pragma mark -

IOReturn BCM5906MEthernet::registerWithPolicyMaker(IOService *policyMaker)
{
    static IOPMPowerState powerStateArray[2] =
    {
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, kIOPMDeviceUsable, kIOPMPowerOn, kIOPMPowerOn, 0, 0, 0, 0, 0, 0, 0, 0}
    };

    return policyMaker->registerPowerDriver(this, powerStateArray, 2);
}

#pragma mark -
#pragma mark Medium
#pragma mark -

void BCM5906MEthernet::publishMedium()
{
    UInt32 miiStatus = miiReadReg(BGE_MII_STATUS);

    addMediumType(kIOMediumEthernetAuto, 0, BGE_MEDIUM_AUTO);

    if (miiStatus & BGE_CAPABLE_10_HD)
        addMediumType(kIOMediumEthernet10BaseT   | kIOMediumOptionHalfDuplex, MBPS_10,  BGE_MEDIUM_10HD);

    if (miiStatus & BGE_CAPABLE_10_FD)
        addMediumType(kIOMediumEthernet10BaseT   | kIOMediumOptionFullDuplex, MBPS_10,  BGE_MEDIUM_10FD);

    if (miiStatus & BGE_CAPABLE_100_TX_HD)
        addMediumType(kIOMediumEthernet100BaseTX | kIOMediumOptionHalfDuplex, MBPS_100, BGE_MEDIUM_100HD);

    if (miiStatus & BGE_CAPABLE_100_TX_FD)
        addMediumType(kIOMediumEthernet100BaseTX | kIOMediumOptionFullDuplex, MBPS_100, BGE_MEDIUM_100FD);

    if (miiStatus & BGE_CAPABLE_100_T4)
        addMediumType(kIOMediumEthernet100BaseT4, MBPS_100, BGE_MEDIUM_100T4);
}

void BCM5906MEthernet::addMediumType(UInt32 type, UInt32 speed, UInt32 index)
{
    IONetworkMedium *medium;
    bool result;

    medium = IONetworkMedium::medium(type, speed, 0, index);
    if (medium)
    {
        result = IONetworkMedium::addMedium(mMediumDict, medium);
        if (result)
            mMediumTable[index] = medium;
        else
        {
            mMediumTable[index] = 0;
            medium->release();
        }
    }
}

IONetworkMedium *BCM5906MEthernet::getMediumWithType(bcmMediumType type)
{
    if (type < BGE_MEDIUM_AUTO || type > BGE_MEDIUM_100T4)
        return 0;
    return mMediumTable[type];
}

IOReturn BCM5906MEthernet::selectMedium(const IONetworkMedium *medium)
{
    bool success;

    if (OSDynamicCast(IONetworkMedium, medium) == 0)
    {
        // Default is autonegotiation.
        medium = getMediumWithType(BGE_MEDIUM_AUTO);
        if (medium == 0)
        {
            DLOG("Error getting medium.");
            return kIOReturnError;
        }
    }

    // Program PHY to select the desired medium.
    success = phySetMedium((bcmMediumType) medium->getIndex());
    if (!success)
        DLOG("phySetMedium() failed.");

    // Update the current medium property.
    if (!setCurrentMedium(medium))
        DLOG("setCurrentMedium() error.");

    return (success ? kIOReturnSuccess : kIOReturnIOError);
}
