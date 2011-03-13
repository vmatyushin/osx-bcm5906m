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
            DLOG("super::start failed");
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
            DLOG("initDriverObjects failed");
            break;
        }

        if (allocateTxMemory() == false)
        {
            DLOG("Can't allocate TX memory!");
            break;
        }

        if (allocateRxMemory() == false)
        {
            DLOG("Can't allocate RX memory!");
            break;
        }

        if (allocateStatusBlockMemory() == false)
        {
            DLOG("Can't allocate Status Block memory!");
            break;
        }

        initPCIConfigSpace(mPCIDevice);

        mMemoryMap = mPCIDevice->mapDeviceMemoryWithRegister(kIOPCIConfigBaseAddress0, kIOMapInhibitCache);
        if (mMemoryMap == NULL)
        {
            DLOG("mMemoryMap failed");
            break;
        }
        mNICBaseAddr = mMemoryMap->getVirtualAddress();

        resetChip();
        initChip();

        if (readEthernetAddress() == false)
        {
            DLOG("Failed to read MAC address");
            break;
        }

        if (phyInit() == false)
        {
            DLOG("Failed to init PHY");
            break;
        }

        publishMedium();
        if (publishMediumDictionary(mMediumDict) == false)
		{
			DLOG("Failed to publish media support");
			break;
		}

        // FIXME: vlan
        // Tell the OS that we will do IPv4, TCP and UDP checksums and support VLAN.
   /*     errno_t retval = ifnet_set_offload(mNetworkInterface->getIfnet(),
                                           IFNET_CSUM_IP | IFNET_CSUM_TCP | IFNET_CSUM_UDP |
                                           IFNET_VLAN_TAGGING | IFNET_VLAN_MTU);
        DLOG("0x%x\n", retval);
*/
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

    if (mPCIDevice)
        mPCIDevice->close(this);

    for (int i = 0; i < mMediumTableSize; i++)
        if (mMediumTable[i] != 0)
            mMediumTable[i]->release();

    RELEASE(mMediumDict);
    RELEASE(mInterruptSrc);
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
		DLOG ("initDriverObjects - no transmit queue was created");
		return false;
	}

    // Get our work loop
    IOWorkLoop *workLoop = (IOWorkLoop *) getWorkLoop();
    if (!workLoop)
    {
        DLOG("initDriverObjects - no work loop was created");
        return false;
    }

    // Create a mbuf cursor.
    mMemoryCursor = IOMbufNaturalMemoryCursor::withSpecification(BGE_MAX_FRAMELEN, 1);
    if (!mMemoryCursor)
    {
        DLOG("Mbuf cursor allocation error");
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
        DLOG("initDriverObjects - IOInterruptEventSource error");
        return false;
    }
	mInterruptSrc->enable();

    mMediumDict = OSDictionary::withCapacity(6);
    if(!mMediumDict)
    {
        DLOG("Failed to create medium dictionary");
        return false;
    }

    return true;
}

bool BCM5906MEthernet::configureInterface(IONetworkInterface *netif)
{
    IONetworkData * data;

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

#pragma mark -
#pragma mark Vendor/Model
#pragma mark -

const OSString * BCM5906MEthernet::newVendorString(void) const
{
    return OSString::withCString("Broadcom Corporation");
}

const OSString * BCM5906MEthernet::newModelString(void) const
{
    return OSString::withCString("NetLink BCM5906M Fast Ethernet PCI Express");
}

#pragma mark -
#pragma mark Receive
#pragma mark -

bool BCM5906MEthernet::allocateRxMemory()
{
    // Allocate Receive Producer ring.
    mReceiveProducerRingDesc = IOBufferMemoryDescriptor::withOptions(kIOMemoryPhysicallyContiguous,
                                                                     BGE_RX_RING_SZ, mPCICacheSize);
    if (mReceiveProducerRingDesc == 0)
    {
        DLOG("No memory for Receive Producer Ring");
        return false;
    }

    if (mReceiveProducerRingDesc->prepare() != kIOReturnSuccess)
    {
        DLOG("Receive Producer Ring prepare() failed");
        mReceiveProducerRingDesc->release();
        mReceiveProducerRingDesc = 0;
        return false;
    }

    mReceiveProducerRingPhysAddr = mReceiveProducerRingDesc->getPhysicalSegment(0, 0);
    if (mReceiveProducerRingPhysAddr == 0)
    {
        DLOG("Receive Producer Ring getPhysicalSegment failed");
        return false;
    }

    mReceiveProducerRingAddr = (bcmReceiveBD *) mReceiveProducerRingDesc->getBytesNoCopy();
    memset(mReceiveProducerRingAddr, 0, BGE_RX_RING_SZ);

    // Allocate Receive Return ring.
    mReceiveReturnRingDesc = IOBufferMemoryDescriptor::withOptions(kIOMemoryPhysicallyContiguous,
                                                                   BGE_RX_RING_SZ, mPCICacheSize);
    if (mReceiveReturnRingDesc == 0)
    {
        DLOG("No memory for Receive Return Ring");
        return false;
    }

    if (mReceiveReturnRingDesc->prepare() != kIOReturnSuccess)
    {
        DLOG("Receive Return Ring prepare() failed");
        mReceiveReturnRingDesc->release();
        mReceiveReturnRingDesc = 0;
        return false;
    }

    mReceiveReturnRingPhysAddr = mReceiveReturnRingDesc->getPhysicalSegment(0, 0);
    if (mReceiveReturnRingPhysAddr == 0)
    {
        DLOG("Receive Return Ring getPhysicalSegment failed");
        return false;
    }

    mReceiveReturnRingAddr = (bcmReceiveBD *) mReceiveReturnRingDesc->getBytesNoCopy();
    memset(mReceiveReturnRingAddr, 0, BGE_RX_RING_SZ);

    mRxPacketArray = IONew(mbuf_t, BGE_RX_RING_CNT);
    if (mRxPacketArray == 0)
    {
        DLOG("Can't get memory for RX packets");
        return false;
    }
    memset(mRxPacketArray, 0, BGE_RX_RING_CNT * sizeof(mbuf_t));

    return true;
}

void BCM5906MEthernet::releaseRxMemory()
{
    if (mReceiveProducerRingDesc)
    {
        mReceiveProducerRingDesc->complete();
        mReceiveProducerRingDesc->release();
        mReceiveProducerRingDesc = 0;
        mReceiveProducerRingAddr = 0;
    }

    if (mReceiveReturnRingDesc)
    {
        mReceiveReturnRingDesc->complete();
        mReceiveReturnRingDesc->release();
        mReceiveReturnRingDesc = 0;
        mReceiveReturnRingAddr = 0;
    }

    if (mRxPacketArray)
    {
        freeRxRingPackets();
        IODelete(mRxPacketArray, mbuf_t, BGE_RX_RING_CNT);
        mRxPacketArray = 0;
    }
}

void BCM5906MEthernet::freeRxRingPackets()
{
    if (mRxPacketArray)
    {
        for (int i = 0; i < BGE_RX_RING_CNT; i++)
        {
            if (mRxPacketArray[i])
            {
                freePacket(mRxPacketArray[i]);
                mRxPacketArray[i] = 0;
            }
        }
    }
}

bool BCM5906MEthernet::initRxRing()
{
    freeRxRingPackets();

    memset(mReceiveProducerRingAddr, 0, BGE_RX_RING_SZ);
    memset(mReceiveReturnRingAddr, 0, BGE_RX_RING_SZ);
    memset(mRxPacketArray, 0, BGE_RX_RING_CNT * sizeof(mbuf_t));

    mRxProducerProd = 0;
    for (int i = 0; i < BGE_RX_RING_CNT; i++)
    {
        if (mRxPacketArray[i] == 0)
            mRxPacketArray[i] = allocatePacket(BGE_MAX_FRAMELEN);
        if (mRxPacketArray[i] == 0)
        {
            DLOG("Failed to allocate RX packet.");
            return false;
        }
        updateRxDescriptor(i);
        BGE_INC(mRxProducerProd, BGE_STD_RX_RING_CNT);
    }

    mRxProducerProd = 0;
    mRxReturnCons = 0;
    writeRegisterMailbox(BGE_MBX_RX_STD_PROD_LO, BGE_STD_RX_RING_CNT - 1);

    return true;
}

void BCM5906MEthernet::updateRxDescriptor(UInt32 index)
{
    UInt32 bcmMaxSegmentNum = 2;
    IOPhysicalSegment vectors[2];
    bcmHostAddr bdAddr;
    UInt32 fragCount;

    fragCount = mMemoryCursor->getPhysicalSegmentsWithCoalesce(mRxPacketArray[index], vectors, bcmMaxSegmentNum);

    BGE_HOSTADDR(bdAddr, vectors[0].location);
    OSWriteLittleInt32(&mReceiveProducerRingAddr[mRxProducerProd].bufHostAddr.addrHi, 0, bdAddr.addrHi);
    OSWriteLittleInt32(&mReceiveProducerRingAddr[mRxProducerProd].bufHostAddr.addrLo, 0, bdAddr.addrLo);
    OSWriteLittleInt16(&mReceiveProducerRingAddr[mRxProducerProd].length, 0, BGE_MAX_FRAMELEN);
    OSWriteLittleInt16(&mReceiveProducerRingAddr[mRxProducerProd].index, 0, index);
    OSWriteLittleInt16(&mReceiveProducerRingAddr[mRxProducerProd].flags, 0, BGE_RXBDFLAG_END);
    OSWriteLittleInt16(&mReceiveProducerRingAddr[mRxProducerProd].type, 0, 0);
    OSWriteLittleInt16(&mReceiveProducerRingAddr[mRxProducerProd].tcp_udp_cksum, 0, 0);
    OSWriteLittleInt16(&mReceiveProducerRingAddr[mRxProducerProd].ip_cksum, 0, 0);
    OSWriteLittleInt16(&mReceiveProducerRingAddr[mRxProducerProd].vlanTag, 0, 0);
    OSWriteLittleInt16(&mReceiveProducerRingAddr[mRxProducerProd].errorFlags, 0, 0);
    OSWriteLittleInt32(&mReceiveProducerRingAddr[mRxProducerProd].rssHash, 0, 0);
    OSWriteLittleInt32(&mReceiveProducerRingAddr[mRxProducerProd].opaque, 0, 0);
}

void BCM5906MEthernet::serviceRxInterrupt()
{
    UInt32 returnRingProd = mStatusBlockAddr->returnRingProd;
    UInt32 returnRingCons = mRxReturnCons;
    bcmReceiveBD *receiveBD;
    UInt32 packetsReceived = 0;
    UInt16 vlanTag = 0;
    UInt16 rxIndex = 0;
    mbuf_t packet;
    bool hasVLan;
    bool replaced;
    bool inputFail;

    // No new packets arrived.
    if (returnRingCons == returnRingProd)
        return;

    while (returnRingCons != returnRingProd)
    {
        receiveBD = &(mReceiveReturnRingAddr[returnRingCons]);
        BGE_INC(returnRingCons, BGE_RX_RING_CNT);
        packetsReceived++;

        rxIndex = receiveBD->index;
        inputFail = false;
        hasVLan = false;

        // Frame has vlan tag.
        if (receiveBD->flags & BGE_RXBDFLAG_VLAN_TAG)
        {
            hasVLan = true;
            vlanTag = receiveBD->vlanTag;
        }

        // Frame has error.
        if (receiveBD->flags & BGE_RXBDFLAG_ERROR)
            inputFail = true;

        if (!inputFail)
        {
            packet = replaceOrCopyPacket(&mRxPacketArray[rxIndex], BGE_MAX_FRAMELEN, &replaced);
            if (packet == 0)
                inputFail = true;
        }

        updateRxDescriptor(rxIndex);
        BGE_INC(mRxProducerProd, BGE_STD_RX_RING_CNT);

        if (inputFail)
        {
            mNetStats->inputErrors += 1;
            continue;
        }

        if (hasVLan)
            setVlanTag(packet, vlanTag);

        // FIXME: correct length
        mNetworkInterface->inputPacket(packet, receiveBD->length, IONetworkInterface::kInputOptionQueuePacket);
        mNetStats->inputPackets += 1;
    }

    mRxReturnCons = returnRingCons;
    writeRegisterMailbox(BGE_MBX_RX_CONS0_LO, mRxReturnCons);
    if (packetsReceived > 0)
        writeRegisterMailbox(BGE_MBX_RX_STD_PROD_LO, mRxProducerProd);

    mNetworkInterface->flushInputQueue();
}

#pragma mark -
#pragma mark Transmit
#pragma mark -

bool BCM5906MEthernet::allocateTxMemory()
{
    mSendRingDesc = IOBufferMemoryDescriptor::withOptions(kIOMemoryPhysicallyContiguous,
                                                          BGE_TX_RING_SZ, mPCICacheSize);
    if (mSendRingDesc == 0)
    {
        DLOG("No memory for Send Ring");
        return false;
    }

    if (mSendRingDesc->prepare() != kIOReturnSuccess)
    {
        DLOG("Send Rings's memory prepare() failed");
        mSendRingDesc->release();
        mSendRingDesc = 0;
        return false;
    }

    mSendRingPhysAddr = mSendRingDesc->getPhysicalSegment(0, 0);
    if (mSendRingPhysAddr == 0)
    {
        DLOG("Send Ring getPhysicalSegment failed");
        return false;
    }

    mSendRingAddr = (bcmSendBD *) mSendRingDesc->getBytesNoCopy();
    memset(mSendRingAddr, 0, BGE_TX_RING_SZ);

    mTxPacketArray = IONew(mbuf_t, BGE_TX_RING_CNT);
    if (mTxPacketArray == 0)
    {
        DLOG("Can't get memory for TX packets");
        return false;
    }
    memset(mTxPacketArray, 0, BGE_TX_RING_CNT * sizeof(mbuf_t));

    return true;
}

void BCM5906MEthernet::releaseTxMemory()
{
    if (mSendRingDesc)
    {
        mSendRingDesc->complete();
        mSendRingDesc->release();
        mSendRingDesc = 0;
        mSendRingAddr = 0;
    }

    if (mTxPacketArray)
    {
        freeTxRingPackets();
        IODelete(mTxPacketArray, mbuf_t, BGE_TX_RING_CNT);
        mTxPacketArray = 0;
    }
}

void BCM5906MEthernet::freeTxRingPackets()
{
    if (mTxPacketArray)
    {
        for (int i = 0; i < BGE_TX_RING_CNT; i++)
        {
            if (mTxPacketArray[i])
            {
                freePacket(mTxPacketArray[i]);
                mTxPacketArray[i] = 0;
            }
        }
    }
}

void BCM5906MEthernet::initTxRing()
{
    freeTxRingPackets();

    if (mSendRingAddr)
        memset(mSendRingAddr, 0, BGE_TX_RING_SZ);
    if (mTxPacketArray)
        memset(mTxPacketArray, 0, BGE_TX_RING_CNT * sizeof(mbuf_t));

    mTxLastCons = 0;
    mTxDescBusy = 0;
}

void BCM5906MEthernet::serviceTxInterrupt()
{
    UInt32 currTXCons = mStatusBlockAddr->sendRingCons;
    UInt32 index = mTxLastCons;

    mTransmitQueue->service();

    if (mTxLastCons == currTXCons)
        return;

    while (index != currTXCons)
    {
        if (mTxPacketArray[index] != 0)
        {
            freePacket(mTxPacketArray[index]);
            mTxPacketArray[index] = 0;
        }
        BGE_INC(index, BGE_TX_RING_CNT);

        if (mTxDescBusy > 0)
            mTxDescBusy--;
    }

    mTxLastCons = currTXCons;
}

UInt32 BCM5906MEthernet::outputPacket(mbuf_t packet, void *param)
{
    UInt32 bcmMaxSegmentNum = 10;
    IOPhysicalSegment vectors[bcmMaxSegmentNum];

    UInt32 currFrag = mTxProd;
    UInt32 prevCurrFrag;
    UInt32 fragCount;

    bcmHostAddr bdAddr;
    UInt16 vlanTag = 0;
    UInt16 flags = 0;

    //UInt32 demandMask;
    //bool mustDoIPChecksum = false;

    // Stall output if ring is almost full.
    if (BGE_TX_RING_CNT - mTxDescBusy < bcmMaxSegmentNum)
        return kIOReturnOutputStall;

    // Check if packet has VLAN tag.
    if (mbuf_get_vlan_tag(packet, &vlanTag) != ENXIO)
        flags = BGE_TXBDFLAG_VLAN_TAG;

    // TODO: offloading
    /*
    getChecksumDemand(packet, kChecksumFamilyTCPIP, &demandMask);
    if ((demandMask & kChecksumTCP) || (demandMask & kChecksumUDP))
        flags |= BGE_TXBDFLAG_TCP_UDP_CSUM;
    if (demandMask & kChecksumIP)
        mustDoIPChecksum = true;
    */

    // Extract physical address and length pairs from the packet.
    fragCount = mMemoryCursor->getPhysicalSegmentsWithCoalesce(packet, vectors, bcmMaxSegmentNum);
    if (fragCount == 0)
        goto drop_packet;

    for (int i = 0; i < fragCount; i++)
    {
        BGE_HOSTADDR(bdAddr, vectors[i].location);
        OSWriteLittleInt32(&mSendRingAddr[currFrag].bufHostAddr.addrHi, 0, bdAddr.addrHi);
        OSWriteLittleInt32(&mSendRingAddr[currFrag].bufHostAddr.addrLo, 0, bdAddr.addrLo);
        // FIXME: FIX FIX FIX
        /*
        if (mustDoIPChecksum && (i == 0))
            OSWriteLittleInt16(&mSendRingAddr[currFrag].flags, 0, flags | BGE_TXBDFLAG_IP_CSUM);
        else*/
        OSWriteLittleInt16(&mSendRingAddr[currFrag].flags, 0, flags);
        OSWriteLittleInt16(&mSendRingAddr[currFrag].vlanTag, 0, vlanTag);
        OSWriteLittleInt16(&mSendRingAddr[currFrag].launchTime, 0, 0);
        OSWriteLittleInt16(&mSendRingAddr[currFrag].length, 0, vectors[i].length);

        prevCurrFrag = currFrag;
        BGE_INC(currFrag, BGE_TX_RING_CNT);
        mTxDescBusy++;
    }

    // Mark the last segment as end of packet.
    OSWriteLittleInt16(&mSendRingAddr[prevCurrFrag].flags, 0, flags | BGE_TXBDFLAG_END);

    // Attach mbuf packet to ring until transmission is complete.
    mTxProd = currFrag;
    mTxPacketArray[currFrag] = packet;

    // Update hardware pointer.
    writeRegisterMailbox(BGE_MBX_TX_HOST_PROD0_LO, mTxProd);

    mNetStats->outputPackets += 1;
    return kIOReturnOutputSuccess;

drop_packet:
    freePacket(packet);
    mNetStats->outputErrors += 1;
    return kIOReturnOutputDropped;
}

void BCM5906MEthernet::getPacketBufferConstraints(IOPacketBufferConstraints *constraints) const
{
    constraints->alignStart = kIOPacketBufferAlign1;
    constraints->alignLength = kIOPacketBufferAlign1;
    return;
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
        DLOG("No memory for Status Block");
        return false;
    }

    if (mStatusBlockDesc->prepare() != kIOReturnSuccess)
    {
        DLOG("Status Block memory prepare() failed");
        mStatusBlockDesc->release();
        mStatusBlockDesc = 0;
        return false;
    }

    mStatusBlockPhysAddr = mStatusBlockDesc->getPhysicalSegment(0, 0);
    if (mStatusBlockPhysAddr == 0)
    {
        DLOG("Status Block getPhysicalSegment failed");
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
#pragma mark Interrupt
#pragma mark -

void BCM5906MEthernet::interruptHandler(OSObject *owner, IOInterruptEventSource *sender, int count)
{
    // Read Interrupt Mailbox 0 in order to flush any posted writes in the PCI chipset.
    readRegisterMailbox(BGE_MBX_IRQ0_LO);

    // Disable interrupts.
    writeRegisterMailbox(BGE_MBX_IRQ0_LO, 1);

    /*DLOG("sw 0x%x", (unsigned int) mStatusBlockAddr->statusWord);
    if (mStatusBlockAddr->statusWord & BGE_STATUSWORD_ERROR)
    {
        DLOG("BGE_FLOW_ATTN: 0x%x", readNICMem(BGE_FLOW_ATTN));
        DLOG("BGE_MAC_STS: 0x%x", readNICMem(BGE_MAC_STS));
        DLOG("BGE_MSI_STATUS: 0x%x", readNICMem(BGE_MSI_STATUS));
        DLOG("BGE_RDMA_STATUS: 0x%x", readNICMem(BGE_RDMA_STATUS));
        DLOG("BGE_WDMA_STATUS: 0x%x", readNICMem(BGE_WDMA_STATUS));
        DLOG("BGE_RXCPU_STATUS: 0x%x", readNICMem(BGE_RXCPU_STATUS));
        DLOG("BGE_RX_MODE: 0x%x", readNICMem(BGE_RX_MODE));
    }

    DLOG("ran out %d", readNICMem(BGE_RXLP_LOCSTAT_OUT_OF_BDS));
    DLOG("frames send %d", readNICMem(BGE_LOCSTATS_COS0));
    DLOG("good dropped %d", readNICMem(BGE_RXLP_LOCSTAT_FILTDROP));
    DLOG("rx placed 1 %d", readNICMem(BGE_RXLP_LOCSTAT_COS0));
    DLOG("rx placed 2 %d", readNICMem(BGE_RXLP_LOCSTAT_COS1));
    DLOG("rx placed 2 %d", readNICMem(BGE_RXLP_LOCSTAT_COS2));
    DLOG("rx placed 2 %d", readNICMem(BGE_RXLP_LOCSTAT_COS3));
    DLOG("discarded %d", readNICMem(BGE_RXLP_LOCSTAT_IFIN_DROPS));
    DLOG("errored %d", readNICMem(BGE_RXLP_LOCSTAT_IFIN_ERRORS));

    DLOG("ifHCInOctets %d", readNICMem(0x0880));
    DLOG("dot3StatsFCSErrors %d", readNICMem(0x0898));
    DLOG("dot3StatsAlignmentErrors %d", readNICMem(0x089C));
    DLOG("xonPauseFramesReceived %d", readNICMem(0x08A0));
    DLOG("xoffPauseFramesReceived %d", readNICMem(0x08A4));
    DLOG("dot3StatsFramesTooLongs %d", readNICMem(0x08B0));
    DLOG("etherStatsUndersizePkts %d", readNICMem(0x08B8));
     */

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

#pragma mark -
#pragma mark Promiscuous/Multicast/Checksum
#pragma mark -

// FIXME: Implement
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
    return kIOReturnSuccess;
}

/*
IOReturn BCM5906MEthernet::getChecksumSupport(UInt32 *checksumMask, UInt32 checksumFamily, bool isOutput)
{
    if (checksumFamily != kChecksumFamilyTCPIP)
		return kIOReturnUnsupported;

    *checksumMask = kChecksumIP | kChecksumTCP | kChecksumUDP;

    return kIOReturnSuccess;
}
*/

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
#pragma mark Chip
#pragma mark -

bool BCM5906MEthernet::initBlock()
{
    UInt32 val, offset;
    int i;
    bcmHostAddr ringAddr;

    /*
	 * Initialize the memory window pointer register so that
	 * we can access the first 32K of internal NIC RAM. This will
	 * allow us to set up the TX send ring RCBs and the RX return
	 * ring RCBs, plus other things which live in NIC memory.
	 */
	writeNICMem(BGE_PCI_MEMWIN_BASEADDR, 0);

    // Configure mbuf memory pool.
    writeNICMem(BGE_BMAN_MBUFPOOL_BASEADDR, BGE_BUFFPOOL_1);
    writeNICMem(BGE_BMAN_MBUFPOOL_LEN, 0x18000);

    // Configure DMA resource pool.
    writeNICMem(BGE_BMAN_DMA_DESCPOOL_BASEADDR, BGE_DMA_DESCRIPTORS);
    writeNICMem(BGE_BMAN_DMA_DESCPOOL_LEN, 0x2000);

	// Configure mbuf pool watermarks.
    writeNICMem(BGE_BMAN_MBUFPOOL_READDMA_LOWAT, 0x0);
    writeNICMem(BGE_BMAN_MBUFPOOL_MACRX_LOWAT, 0x04);
    writeNICMem(BGE_BMAN_MBUFPOOL_HIWAT, 0x10);

	// Configure DMA resource watermarks.
	writeNICMem(BGE_BMAN_DMA_DESCPOOL_LOWAT, 5);
	writeNICMem(BGE_BMAN_DMA_DESCPOOL_HIWAT, 10);

	// Enable buffer manager.
    writeNICMem(BGE_BMAN_MODE, BGE_BMANMODE_ENABLE | BGE_BMANMODE_LOMBUF_ATTN);

    // Poll for buffer manager start indication.
    for (i = 0; i < BGE_TIMEOUT; i++)
    {
        IODelay(10);
        if (readNICMem(BGE_BMAN_MODE) & BGE_BMANMODE_ENABLE)
            break;
    }

    if (i == BGE_TIMEOUT)
    {
        DLOG("Can't start NIC buffer manager");
        return false;
    }

	// Enable flow-through queues.
	writeNICMem(BGE_FTQ_RESET, 0xFFFFFFFF);
	writeNICMem(BGE_FTQ_RESET, 0);

	// Wait until queue initialization is complete.
	for (i = 0; i < BGE_TIMEOUT; i++)
    {
		IODelay(10);
		if (readNICMem(BGE_FTQ_RESET) == 0)
			break;
	}

	if (i == BGE_TIMEOUT)
    {
		DLOG("Flow-through queue init failed");
		return false;
	}

	// Initialize the RX producer ring control block.
	BGE_HOSTADDR(ringAddr, mReceiveProducerRingPhysAddr);
	writeNICMem(BGE_RX_STD_RCB_HADDR_HI, ringAddr.addrHi);
	writeNICMem(BGE_RX_STD_RCB_HADDR_LO, ringAddr.addrLo);
    writeNICMem(BGE_RX_STD_RCB_MAXLEN_FLAGS, BGE_MAX_FRAMELEN << 16);
	writeNICMem(BGE_RX_STD_RCB_NICADDR, BGE_STD_RX_RINGS);

	// Set the BD ring replentish thresholds.
	writeNICMem(BGE_RBDI_STD_REPL_THRESH, 25);

    // Initialize send ring producer index mailbox register.
    writeNICMem(BGE_MBX_TX_HOST_PROD0_LO, 0);

	/*
	 * Disable all unused send rings by setting the 'ring disabled'
	 * bit in the flags field of all the TX send ring control blocks.
	 * These are located in NIC memory.
	 */
	offset = BGE_MEMWIN_START + BGE_SEND_RING_RCB;
	for (i = 0; i < BGE_TX_RINGS_EXTSSRAM_MAX; i++)
    {
		bcmRCBWrite(offset, maxlen_flags, BGE_RCB_FLAG_RING_DISABLED);
		bcmRCBWrite(offset, nicAddr, 0);
		offset += sizeof(bcmRCB);
	}

	// Configure TX RCB 0 (we use only the first ring).
	offset = BGE_MEMWIN_START + BGE_SEND_RING_RCB;
	BGE_HOSTADDR(ringAddr, mSendRingPhysAddr);
	bcmRCBWrite(offset, ringHostAddr.addrHi, ringAddr.addrHi);
	bcmRCBWrite(offset, ringHostAddr.addrLo, ringAddr.addrLo);
	bcmRCBWrite(offset, nicAddr, BGE_NIC_TXRING_ADDR);
	bcmRCBWrite(offset, maxlen_flags, BGE_TX_RING_CNT << 16);

	// Disable all unused RX return rings.
	offset = BGE_MEMWIN_START + BGE_RX_RETURN_RING_RCB;
	for (i = 0; i < BGE_RX_RINGS_MAX; i++)
    {
		bcmRCBWrite(offset, ringHostAddr.addrHi, 0);
		bcmRCBWrite(offset, ringHostAddr.addrLo, 0);
		bcmRCBWrite(offset, maxlen_flags, BGE_RCB_FLAG_RING_DISABLED);
		bcmRCBWrite(offset, nicAddr, 0);
		writeRegisterMailbox(BGE_MBX_RX_CONS0_LO + (i * (sizeof(UInt64))), 0);
		offset += sizeof(bcmRCB);
	}

	// Initialize producer ring producer index mailbox register.
    writeRegisterMailbox(BGE_MBX_RX_STD_PROD_LO, 0);

	/*
	 * Set up RX return ring 0
	 * Note that the NIC address for RX return rings is 0x0.
	 * The return rings live entirely within the host, so the
	 * nicaddr field in the RCB isn't used.
	 */
    offset = BGE_MEMWIN_START + BGE_RX_RETURN_RING_RCB;
    BGE_HOSTADDR(ringAddr, mReceiveReturnRingPhysAddr);
	bcmRCBWrite(offset, ringHostAddr.addrHi, ringAddr.addrHi);
    bcmRCBWrite(offset, ringHostAddr.addrLo, ringAddr.addrLo);
    bcmRCBWrite(offset, maxlen_flags, BGE_RX_RING_CNT << 16);
    bcmRCBWrite(offset, nicAddr, 0);

    // Load MAC address.
    writeNICMem(BGE_MAC_ADDR1_LO, mEtherAddr.bytes[4] | (mEtherAddr.bytes[5] << 8));
    writeNICMem(BGE_MAC_ADDR1_HI, mEtherAddr.bytes[0] | (mEtherAddr.bytes[1] << 8)|
                                 (mEtherAddr.bytes[2] << 16) | (mEtherAddr.bytes[3] << 24));

	// Set random backoff seed for TX.
	writeNICMem(BGE_TX_RANDOM_BACKOFF,
                mEtherAddr.bytes[0] + mEtherAddr.bytes[1] + mEtherAddr.bytes[2] +
                mEtherAddr.bytes[3] + mEtherAddr.bytes[4] + mEtherAddr.bytes[5] +
                BGE_TX_BACKOFF_SEED_MASK);

	// Set inter-packet gap for transmit.
	writeNICMem(BGE_TX_LENGTHS, 0x2620);

    // Specify which ring to use for packets that don't match any RX rules.
	writeNICMem(BGE_RX_RULES_CFG, 0x8);

	/*
	 * Configure number of RX lists. One interrupt distribution
	 * list, sixteen active lists, one bad frames class.
	 */
	writeNICMem(BGE_RXLP_CFG, 0x181);

	// Inialize RX list placement stats mask.
	writeNICMem(BGE_RXLP_STATS_ENABLE_MASK, 0xFFFFFF);
	writeNICMem(BGE_RXLP_STATS_CTL, 0x1);

	// Disable host coalescing until we get it set up.
	writeNICMem(BGE_HCC_MODE, 0x0);

	// Poll to make sure it's shut down.
	for (i = 0; i < BGE_TIMEOUT; i++)
    {
		IODelay(10);
		if (!(readNICMem(BGE_HCC_MODE) & BGE_HCCMODE_ENABLE))
			break;
	}

	if (i == BGE_TIMEOUT)
    {
		DLOG("Host coalescing engine failed to shut down.");
		return false;
	}

	// Set up host coalescing defaults.
	writeNICMem(BGE_HCC_RX_COAL_TICKS, 150);
	writeNICMem(BGE_HCC_TX_COAL_TICKS, 150);
	writeNICMem(BGE_HCC_RX_MAX_COAL_BDS, 10);
	writeNICMem(BGE_HCC_TX_MAX_COAL_BDS, 10);
	writeNICMem(BGE_HCC_RX_MAX_COAL_BDS_INT, 1);
	writeNICMem(BGE_HCC_TX_MAX_COAL_BDS_INT, 1);

    // Set up address of statistics block.
    // FIXME: stats
    //writeNICMem(BGE_HCC_STATS_ADDR_HI, BGE_ADDR_HI(sc->bge_ldata.bge_stats_paddr));
    //writeNICMem(BGE_HCC_STATS_ADDR_LO, BGE_ADDR_LO(sc->bge_ldata.bge_stats_paddr));
    writeNICMem(BGE_HCC_STATUSBLK_BASEADDR, BGE_STATUS_BLOCK);

    // Set up address of status block.
	writeNICMem(BGE_HCC_STATUSBLK_ADDR_HI, BGE_ADDR_HI(mStatusBlockPhysAddr));
	writeNICMem(BGE_HCC_STATUSBLK_ADDR_LO, BGE_ADDR_LO(mStatusBlockPhysAddr));

	// Set up status block size.
    val = BGE_STATBLKSZ_32BYTE;

	// Turn on host coalescing state machine.
	writeNICMem(BGE_HCC_MODE, val | BGE_HCCMODE_ENABLE);

	// Turn on RX BD completion state machine and enable attentions.
	writeNICMem(BGE_RBDC_MODE, BGE_RBDCMODE_ENABLE | BGE_RBDCMODE_ATTN);

	// Turn on RX list placement state machine.
	writeNICMem(BGE_RXLP_MODE, BGE_RXLPMODE_ENABLE);

	// Turn on RX list selector state machine.
    writeNICMem(BGE_RXLS_MODE, BGE_RXLSMODE_ENABLE);

	val = BGE_MACMODE_TXDMA_ENB | BGE_MACMODE_RXDMA_ENB |
          BGE_MACMODE_RX_STATS_CLEAR | BGE_MACMODE_TX_STATS_CLEAR |
          BGE_MACMODE_RX_STATS_ENB | BGE_MACMODE_TX_STATS_ENB |
          BGE_MACMODE_FRMHDR_DMA_ENB;
    val |= BGE_PORTMODE_MII;

	// Turn on DMA, clear stats.
	writeNICMem(BGE_MAC_MODE, val);

	// Set misc. local control, enable interrupts on attentions.
	writeNICMem(BGE_MISC_LOCAL_CTL, BGE_MLC_INTR_ONATTN);

	// Turn on DMA completion state machine.
    writeNICMem(BGE_DMAC_MODE, BGE_DMACMODE_ENABLE);

	// Turn on write DMA state machine.
    val = BGE_WDMAMODE_ENABLE | BGE_WDMAMODE_ALL_ATTNS | BGE_WDMAMODE_STATUS_TAG_FIX;
	writeNICMem(BGE_WDMA_MODE, val);
	IODelay(40);

	// Turn on read DMA state machine.
	val = BGE_RDMAMODE_ENABLE | BGE_RDMAMODE_ALL_ATTNS;
    val |= BGE_RDMAMODE_FIFO_LONG_BURST;
	writeNICMem(BGE_RDMA_MODE, val);
	IODelay(40);

	// Turn on RX data completion state machine.
	writeNICMem(BGE_RDC_MODE, BGE_RDCMODE_ENABLE | BGE_RDCMODE_ATTN);

	// Turn on RX BD initiator state machine.
	writeNICMem(BGE_RBDI_MODE, BGE_RBDIMODE_ENABLE | BGE_RBDIMODE_ATTN);

	// Turn on RX data and RX BD initiator state machine.
	writeNICMem(BGE_RDBDI_MODE, BGE_RDBDIMODE_ENABLE | BGE_RDBDIMODE_BADRINGSZ_ATTN);

	// Turn on Mbuf cluster free state machine.
    writeNICMem(BGE_MBCF_MODE, BGE_MBCFMODE_ENABLE);

	// Turn on send BD completion state machine.
	writeNICMem(BGE_SBDC_MODE, BGE_SBDCMODE_ENABLE);

	// Turn on send data completion state machine.
	val = BGE_SDCMODE_ENABLE;
	writeNICMem(BGE_SDC_MODE, val);

	// Turn on send data initiator state machine.
    writeNICMem(BGE_SDI_MODE, BGE_SDIMODE_ENABLE);

	// Turn on send BD initiator state machine.
	writeNICMem(BGE_SBDI_MODE, BGE_SBDIMODE_ENABLE);

	// Turn on send BD selector state machine.
	writeNICMem(BGE_SRS_MODE, BGE_SRSMODE_ENABLE);

	writeNICMem(BGE_SDI_STATS_ENABLE_MASK, 0xFFFFFF);
	writeNICMem(BGE_SDI_STATS_CTL, BGE_SDISTATSCTL_ENABLE | BGE_SDISTATSCTL_FASTER);

	// ack/clear link change events.
	writeNICMem(BGE_MAC_STS, BGE_MACSTAT_SYNC_CHANGED |
                BGE_MACSTAT_CFG_CHANGED | BGE_MACSTAT_MI_COMPLETE |
                BGE_MACSTAT_LINK_CHANGED);
	writeNICMem(BGE_MI_STS, 0);

	// Enable PHY auto polling (for MII/GMII only).
    bcmSetBit(BGE_MI_MODE, BGE_MIMODE_AUTOPOLL | (10 << 16));

	// Clear any pending link state attention.
	writeNICMem(BGE_MAC_STS, BGE_MACSTAT_SYNC_CHANGED |
                BGE_MACSTAT_CFG_CHANGED | BGE_MACSTAT_MI_COMPLETE |
                BGE_MACSTAT_LINK_CHANGED);

	// Enable link state change attentions.
	bcmSetBit(BGE_MAC_EVT_ENB, BGE_EVTENB_LINK_CHANGED);

    // Specify MTU.
    writeNICMem(BGE_RX_MTU, BGE_MAX_FRAMELEN);
    // FIXME: correct mtu?
	//writeNICMem(BGE_RX_MTU, mNetworkInterface->getMaxTransferUnit() +
    //                        ETH_HDR_LEN + ETH_CRC_LEN + ETH_VLAN_TAG_LEN);

    // Turn on transmitter.
	bcmSetBit(BGE_TX_MODE, BGE_TXMODE_ENABLE);

	// Turn on receiver.
    // FIXME: unwanted promisc
    bcmSetBit(BGE_RX_MODE, BGE_RXMODE_RX_PROMISC);
	bcmSetBit(BGE_RX_MODE, BGE_RXMODE_ENABLE);

	// Tell firmware we're alive.
	bcmSetBit(BGE_MODE_CTL, BGE_MODECTL_STACKUP);

	// Enable host interrupts.
    bcmSetBit(BGE_PCI_MISC_CTL, BGE_PCIMISCCTL_CLEAR_INTA);
	bcmClrBit(BGE_PCI_MISC_CTL, BGE_PCIMISCCTL_MASK_PCI_INTR);
    writeRegisterMailbox(BGE_MBX_IRQ0_LO, 0);

    return true;
}

void BCM5906MEthernet::initChip()
{
    UInt32 dmaControlRegister;

    // Set endianness before we access any non-PCI registers.
    mPCIDevice->configWrite32(BGE_PCI_MISC_CTL, BGE_INIT);

    // Clear the MAC statistics block in the NIC's internal memory.
    for (int i = BGE_STATS_BLOCK; i < BGE_STATS_BLOCK_END + 1; i += sizeof(UInt32))
    {
        mPCIDevice->configWrite32(BGE_PCI_MEMWIN_BASEADDR, (0xFFFF0000 & i));
        writeNICMem(BGE_MEMWIN_START + (i & 0xFFFF), 0);
    }

    // Clear the status block in the NIC's internal memory.
    for (int i = BGE_STATUS_BLOCK; i < BGE_STATUS_BLOCK_END + 1; i += sizeof(UInt32))
    {
        mPCIDevice->configWrite32(BGE_PCI_MEMWIN_BASEADDR, (0xFFFF0000 & i));
        writeNICMem(BGE_MEMWIN_START + (i & 0xFFFF), 0);
    }

    // Set up the PCI DMA control register.
    dmaControlRegister = BGE_PCIDMARWCTL_RD_CMD_SHIFT(6) | BGE_PCIDMARWCTL_WR_CMD_SHIFT(7);
    // Read watermark not used, 128 bytes for write.
    dmaControlRegister |= BGE_PCIDMARWCTL_WR_WAT_SHIFT(3);
    mPCIDevice->configWrite32(BGE_PCI_DMA_RW_CTL, dmaControlRegister);

    // Set up general mode register.
    writeNICMem(BGE_MODE_CTL, BGE_DMA_SWAP_OPTIONS |
                              BGE_MODECTL_MAC_ATTN_INTR |
                              BGE_MODECTL_HOST_SEND_BDS);

    bcmSetBit(BGE_MODE_CTL, BGE_MODECTL_STACKUP);

    /*
	 * Disable memory write invalidate.  Apparently it is not supported
	 * properly by these devices.  Also ensure that INTx isn't disabled,
	 * as these chips need it even when using MSI.
	 */
    mPCIDevice->setConfigBits(kIOPCIConfigCommand, kIOPCICommandInterruptDisable | kIOPCICommandMemWrInvalidate,
                              ~(kIOPCICommandInterruptDisable | kIOPCICommandMemWrInvalidate));

	// The Linux tg3 driver does this at the start of brgphy_reset.
    IODelay(40);
    // Put PHY into ready state.
    bcmClrBit(BGE_MISC_CFG, BGE_MISCCFG_EPHY_IDDQ);
    readNICMem(BGE_MISC_CFG);
    IODelay(40);
}

void BCM5906MEthernet::resetChip()
{
    UInt32 command, pcistate, reset, val, i;

    // Save some important PCI state.
    mPCICacheSize = mPCIDevice->configRead32(BGE_PCI_CACHESZ);
    command = mPCIDevice->configRead32(BGE_PCI_CMD);
    pcistate = mPCIDevice->configRead32(BGE_PCI_PCISTATE);

    mPCIDevice->configWrite32(BGE_PCI_MISC_CTL, BGE_PCIMISCCTL_INDIRECT_ACCESS |
                                                BGE_PCIMISCCTL_MASK_PCI_INTR |
                                                BGE_HIF_SWAP_OPTIONS |
                                                BGE_PCIMISCCTL_PCISTATE_RW);

    // Enable Memory Arbiter.
    writeNICMem(BGE_MARB_MODE, BGE_MARBMODE_ENABLE);

    // Initialize the Miscellaneous Host Control register.
    mPCIDevice->configWrite32(BGE_PCI_MISC_CTL, BGE_PCIMISCCTL_INDIRECT_ACCESS |
                              BGE_PCIMISCCTL_MASK_PCI_INTR |
                              BGE_HIF_SWAP_OPTIONS |
                              BGE_PCIMISCCTL_PCISTATE_RW);

    /*
	 * Write the magic number to SRAM at offset 0xB50.
	 * When firmware finishes its initialization it will
	 * write ~BGE_MAGIC_NUMBER to the same location.
	 */
    writeNICMemIndirect(BGE_SOFTWARE_GENCOMM, BGE_MAGIC_NUMBER);

    // Prevent PCIE link training during global reset.
    writeNICMem(BGE_MISC_CFG, 1 << 29);

    // Issue global reset.
    reset = BGE_MISCCFG_RESET_CORE_CLOCKS;
    reset |= 1 << 29;
    writeRegisterIndirect(BGE_MISC_CFG, reset);

    bcmSetBit(BGE_VCPU_STATUS, BGE_VCPU_STATUS_DRV_RESET);
    bcmClrBit(BGE_VCPU_EXT_CTRL, BGE_VCPU_EXT_CTRL_HALT_CPU);
    IODelay(1000);

    // Disable host interrupts.
    bcmSetBit(BGE_PCI_MISC_CTL, BGE_PCIMISCCTL_MASK_PCI_INTR);
    writeRegisterMailbox(BGE_MBX_IRQ0_LO, 1);

    // Reset some of the PCI state that got zapped by reset.
    mPCIDevice->configWrite32(BGE_PCI_MISC_CTL, BGE_PCIMISCCTL_INDIRECT_ACCESS |
                                                BGE_PCIMISCCTL_MASK_PCI_INTR |
                                                BGE_HIF_SWAP_OPTIONS |
                                                BGE_PCIMISCCTL_PCISTATE_RW);
    mPCIDevice->configWrite32(BGE_PCI_CACHESZ, mPCICacheSize);
    mPCIDevice->configWrite32(BGE_PCI_CMD, command);

    // Enable Memory Arbiter again.
    writeNICMem(BGE_MARB_MODE, BGE_MARBMODE_ENABLE);

    for (i = 0; i < BGE_TIMEOUT; i++)
    {
        val = readNICMem(BGE_VCPU_STATUS);
        if (val & BGE_VCPU_STATUS_INIT_DONE)
            break;
        IODelay(100);
    }

    /*
     * Wait for the value of the PCISTATE register to
     * return to its original pre-reset state. This is a
     * fairly good indicator of reset completion. If we don't
     * wait for the reset to fully complete, trying to read
     * from the device's non-PCI registers may yield garbage
     * results.
     */
    for (i = 0; i < BGE_TIMEOUT; i++)
    {
        if (mPCIDevice->configRead32(BGE_PCI_PCISTATE) == pcistate)
            break;
        IODelay(10);
    }

    val = readNICMem(0x7C00);
    writeNICMem(0x7C00, val | (1 << 25));

    // Fix up byte swapping.
    writeNICMem(BGE_MODE_CTL, BGE_DMA_SWAP_OPTIONS | BGE_MODECTL_BYTESWAP_DATA);

    // Port mode = 01 (MII).
    writeNICMem(BGE_MAC_MODE, 0x4);

    IODelay(1000);
}

void BCM5906MEthernet::stopChip()
{
    // Disable host interrupts.
    bcmSetBit(BGE_PCI_MISC_CTL, BGE_PCIMISCCTL_MASK_PCI_INTR);
    writeRegisterMailbox(BGE_MBX_IRQ0_LO, 1);

    // Disable all of the receiver blocks.
    bcmClrBit(BGE_RX_MODE, BGE_RXMODE_ENABLE);
    bcmClrBit(BGE_RBDI_MODE, BGE_RBDIMODE_ENABLE);
    bcmClrBit(BGE_RXLP_MODE, BGE_RXLPMODE_ENABLE);
    bcmClrBit(BGE_RDBDI_MODE, BGE_RBDIMODE_ENABLE);
    bcmClrBit(BGE_RDC_MODE, BGE_RDCMODE_ENABLE);
    bcmClrBit(BGE_RBDC_MODE, BGE_RBDCMODE_ENABLE);

    // Disable all of the transmit blocks.
    bcmClrBit(BGE_SRS_MODE, BGE_SRSMODE_ENABLE);
    bcmClrBit(BGE_SBDI_MODE, BGE_SBDIMODE_ENABLE);
    bcmClrBit(BGE_SDI_MODE, BGE_SDIMODE_ENABLE);
    bcmClrBit(BGE_RDMA_MODE, BGE_RDMAMODE_ENABLE);
    bcmClrBit(BGE_SDC_MODE, BGE_SDCMODE_ENABLE);
    bcmClrBit(BGE_SBDC_MODE, BGE_SBDCMODE_ENABLE);

    // Shut down all of the memory managers and related state machines.
    bcmClrBit(BGE_HCC_MODE, BGE_HCCMODE_ENABLE);
    bcmClrBit(BGE_WDMA_MODE, BGE_WDMAMODE_ENABLE);

    writeNICMem(BGE_FTQ_RESET, 0xFFFFFFFF);
    writeNICMem(BGE_FTQ_RESET, 0);

    bcmClrBit(BGE_MODE_CTL, BGE_MODECTL_STACKUP);
}

#pragma mark -
#pragma mark Medium
#pragma mark -

void BCM5906MEthernet::publishMedium()
{
    UInt32 miiStatus = miiReadReg(BGE_MII_STATUS);

    addMediumType(kIOMediumEthernetAuto, 0, BGE_MEDIUM_AUTO);

    if (miiStatus & BGE_CAPABLE_10_HD)
        addMediumType(kIOMediumEthernet10BaseT | kIOMediumOptionHalfDuplex, MBPS_10, BGE_MEDIUM_10HD);

    if (miiStatus & BGE_CAPABLE_10_FD)
        addMediumType(kIOMediumEthernet10BaseT | kIOMediumOptionFullDuplex, MBPS_10, BGE_MEDIUM_10FD);

    if (miiStatus & BGE_CAPABLE_100_TX_HD)
        addMediumType(kIOMediumEthernet100BaseTX | kIOMediumOptionHalfDuplex, MBPS_100, BGE_MEDIUM_100HD);

    if (miiStatus & BGE_CAPABLE_100_TX_FD)
        addMediumType(kIOMediumEthernet100BaseTX | kIOMediumOptionFullDuplex, MBPS_100, BGE_MEDIUM_100FD);

    if (miiStatus & BGE_CAPABLE_100_T4)
        addMediumType(kIOMediumEthernet100BaseT4, MBPS_100, BGE_MEDIUM_100T4);
}

void BCM5906MEthernet::addMediumType(UInt32 type, UInt32 speed, UInt32 index)
{
    IONetworkMedium	* medium;
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

IOReturn BCM5906MEthernet::selectMedium(const IONetworkMedium *medium)
{
    bool  success;

	if (OSDynamicCast(IONetworkMedium, medium) == 0)
    {
        // Defaults to Auto.
        medium = phyGetMediumWithType(BGE_MEDIUM_AUTO);
        if (medium == 0)
		{
			DLOG("Error getting medium");
			return kIOReturnError;
		}
    }

	// Program PHY to select the desired medium.
	success = phySetMedium((bcmMediumType) medium->getIndex());
    if (!success)
        DLOG("phySetMedium failed");

	// Update the current medium property.
	if (!setCurrentMedium(medium))
		DLOG("setCurrentMedium error");

	return (success ? kIOReturnSuccess : kIOReturnIOError);
}

#pragma mark -
#pragma mark PHY
#pragma mark -

UInt32 BCM5906MEthernet::miiReadReg(UInt32 reg)
{
    UInt32 data, autopoll, i;

    // Temporarily disable autopolling.
    autopoll = readNICMem(BGE_MI_MODE);
    if (autopoll & BGE_MIMODE_AUTOPOLL)
    {
        bcmClrBit(BGE_MI_MODE, BGE_MIMODE_AUTOPOLL);
        IODelay(40);
    }

    // Setup MII Communication register.
    writeNICMem(BGE_MI_COMM, BGE_MICMD_READ | BGE_MICOMM_BUSY | BGE_MIPHY(1) | BGE_MIREG(reg));

    for (i = 0; i < BGE_TIMEOUT; i++)
    {
        IODelay(10);
        data = readNICMem(BGE_MI_COMM);
        if (!(data & BGE_MICOMM_BUSY))
            break;
    }

    if (i == BGE_TIMEOUT)
        data = 0;
    else
    {
        IODelay(5);
        data = readNICMem(BGE_MI_COMM);
    }

    if (autopoll & BGE_MIMODE_AUTOPOLL)
    {
        bcmSetBit(BGE_MI_MODE, BGE_MIMODE_AUTOPOLL);
        IODelay(40);
    }

    if (data & BGE_MICOMM_READFAIL)
        return (0);

    return (data & 0xFFFF);
}

void BCM5906MEthernet::miiWriteReg(UInt32 reg, UInt32 data)
{
    UInt32 autopoll, i;

    // Temporarily disable autopolling.
    autopoll = readNICMem(BGE_MI_MODE);
    if (autopoll & BGE_MIMODE_AUTOPOLL)
    {
        bcmClrBit(BGE_MI_MODE, BGE_MIMODE_AUTOPOLL);
        IODelay(40);
    }

    // Setup MII Communication register.
    writeNICMem(BGE_MI_COMM, BGE_MICMD_WRITE | BGE_MICOMM_BUSY | BGE_MIPHY(1) | BGE_MIREG(reg) | data);

    for (i = 0; i < BGE_TIMEOUT; i++)
    {
        IODelay(10);
        if (!(readNICMem(BGE_MI_COMM) & BGE_MICOMM_BUSY))
        {
            IODelay(5);
            readNICMem(BGE_MI_COMM);
            break;
        }
    }

    if (autopoll & BGE_MIMODE_AUTOPOLL)
    {
        bcmSetBit(BGE_MI_MODE, BGE_MIMODE_AUTOPOLL);
        IODelay(40);
    }
}

bool BCM5906MEthernet::phyInit()
{
    UInt32 phyControl, i;

    // Reset PHY.
    phyControl = BGE_PHY_RESET;
    miiWriteReg(BGE_MII_CTL, phyControl);

    for (i = 0; i < BGE_TIMEOUT; i++)
    {
        phyControl = miiReadReg(BGE_MII_CTL);
        if ((phyControl & BGE_PHY_RESET) == 0)
            break;
        IODelay(10);
    }
    if (i == BGE_TIMEOUT)
    {
        DLOG("PHY reset failed.");
        return false;
    }

    // Init PHY.
    // Disable link events.
    writeNICMem(BGE_MAC_EVT_ENB, 0);

    // Clear link attentions.
    bcmClrBit(BGE_MAC_STS, BGE_MACSTAT_LINK_CHANGED);

    // Disable autopolling mode.
    writeNICMem(BGE_MI_MODE, 0xC0020);
    IODelay(40);

    // Acknowledge outstanding interrupts (must read twice).
    miiReadReg(BGE_MII_INTERRUPT);
    miiReadReg(BGE_MII_INTERRUPT);

    // Enable autopolling mode.
    bcmSetBit(BGE_MI_MODE, BGE_MIMODE_AUTOPOLL);

    // Enable link attentions.
    writeNICMem(BGE_MAC_EVT_ENB, BGE_EVTENB_LINK_CHANGED);
    bcmSetBit(BGE_MODE_CTL, BGE_MODECTL_MAC_ATTN_INTR);

    return true;
}

IONetworkMedium *BCM5906MEthernet::phyGetMediumWithType(bcmMediumType type)
{
    if (type < BGE_MEDIUM_AUTO || type > BGE_MEDIUM_100T4)
        return 0;
    return mMediumTable[type];
}

bool BCM5906MEthernet::phySetMedium(bcmMediumType mediumType)
{
    // Reset PHY.
    phyInit();

    if (mediumType == BGE_MEDIUM_AUTO)
        return true;

    UInt32 miiControl = miiReadReg(BGE_MII_CTL);
    miiControl &= BGE_MII_CTL_AUTONEG_DISABLE;

	switch (mediumType)
	{
        case BGE_MEDIUM_10HD:
            miiControl &= BGE_MII_CTL_FORCED_10;
            miiControl &= BGE_MII_CTL_DUPLEX_HALF;
            break;

        case BGE_MEDIUM_10FD:
            miiControl &= BGE_MII_CTL_FORCED_10;
            miiControl |= BGE_MII_CTL_DUPLEX_FULL;
            break;

        case BGE_MEDIUM_100HD:
        case BGE_MEDIUM_100T4:
            miiControl |= BGE_MII_CTL_FORCED_100;
            miiControl &= BGE_MII_CTL_DUPLEX_HALF;
            break;

        case BGE_MEDIUM_100FD:
            miiControl |= BGE_MII_CTL_FORCED_100;
            miiControl |= BGE_MII_CTL_DUPLEX_FULL;
            break;

		default:
			return false;
			break;
	}

    miiWriteReg(BGE_MII_CTL, miiControl);
    return true;
}

void BCM5906MEthernet::phyGetLinkStatus(bool firstPoll)
{
    UInt32 miiStatus = miiReadReg(BGE_MII_STATUS);
    UInt32 statusChange, i;

    // Detect a change in the two link related bits.
    statusChange = (mPHYPrevStatus ^ miiStatus) & (BGE_MII_STS_LINK | BGE_MII_STS_AUTONEG_COMP);

    if (statusChange || firstPoll)
    {
        if (firstPoll)
        {
            // For the initial link status poll, wait a bit, then
            // re-read the status register to clear any latched bits.
            i = 5000;

            while (i > 0)
            {
                miiStatus = miiReadReg(BGE_MII_STATUS);
                if (!miiStatus)
                    break;

                if (miiStatus & BGE_MII_STS_AUTONEG_COMP)
                    break;

                IOSleep(20);
                i -= 20;
            }

            miiStatus = miiReadReg(BGE_MII_STATUS);
            miiStatus = miiReadReg(BGE_MII_STATUS);
        }

        // Determine link status.
        if (miiStatus & BGE_MII_STS_LINK)
        {
            // Link is up.
            IONetworkMedium *activeMedium = phyGetMediumWithType(phyGetActiveMedium());
            setLinkStatus(kIONetworkLinkValid | kIONetworkLinkActive, activeMedium);
            DLOG("Link is up");
        }
        else
        {
            // Link is down.
            setLinkStatus(kIONetworkLinkValid, 0);
            DLOG("Link is down");
        }

        // Save status.
        mPHYPrevStatus = miiStatus;
    }
}

bcmMediumType BCM5906MEthernet::phyGetActiveMedium()
{
    bcmMediumType medium;

    UInt32 anar = miiReadReg(BGE_MII_ANAR);
	UInt32 anlpar = miiReadReg(BGE_MII_ANLPAR);
	UInt32 common = anar & anlpar;

    if (common & BGE_MII_ANAR_T4)
	{
		DLOG("100 T4 Active");
		medium = BGE_MEDIUM_100T4;
	}
    else if (common & BGE_MII_ANAR_TX_FD)
	{
		DLOG("100 TX FD Active");
		medium = BGE_MEDIUM_100FD;
	}
    else if (common & BGE_MII_ANAR_TX_HD)
	{
		DLOG("100 TX HD Active");
		medium = BGE_MEDIUM_100HD;
	}
    else if (common & BGE_MII_ANAR_10_FD)
	{
		DLOG("10 TX FD Active");
		medium = BGE_MEDIUM_10FD;
	}
    else
	{
		DLOG("10 TX HD Active");
		medium = BGE_MEDIUM_10HD;
	}

    return medium;
}
