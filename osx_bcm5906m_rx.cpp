#include "osx_bcm5906m.h"
#include "osx_bcm5906m_reg.h"

bool BCM5906MEthernet::allocateRxMemory()
{
    // Allocate Receive Producer ring.
    mReceiveProducerRingDesc = IOBufferMemoryDescriptor::withOptions(kIOMemoryPhysicallyContiguous,
                                                                     BGE_RX_RING_SZ, mPCICacheSize);
    if (mReceiveProducerRingDesc == 0)
    {
        DLOG("No memory for receive rroducer ring.");
        return false;
    }

    if (mReceiveProducerRingDesc->prepare() != kIOReturnSuccess)
    {
        DLOG("Receive producer ring prepare() failed.");
        mReceiveProducerRingDesc->release();
        mReceiveProducerRingDesc = 0;
        return false;
    }

    mReceiveProducerRingPhysAddr = mReceiveProducerRingDesc->getPhysicalSegment(0, 0);
    if (mReceiveProducerRingPhysAddr == 0)
    {
        DLOG("Receive producer ring getPhysicalSegment() failed.");
        return false;
    }

    mReceiveProducerRingAddr = (bcmReceiveBD *) mReceiveProducerRingDesc->getBytesNoCopy();
    memset(mReceiveProducerRingAddr, 0, BGE_RX_RING_SZ);

    // Allocate Receive Return ring.
    mReceiveReturnRingDesc = IOBufferMemoryDescriptor::withOptions(kIOMemoryPhysicallyContiguous,
                                                                   BGE_RX_RING_SZ, mPCICacheSize);
    if (mReceiveReturnRingDesc == 0)
    {
        DLOG("No memory for receive return ring.");
        return false;
    }

    if (mReceiveReturnRingDesc->prepare() != kIOReturnSuccess)
    {
        DLOG("Receive return ring prepare() failed.");
        mReceiveReturnRingDesc->release();
        mReceiveReturnRingDesc = 0;
        return false;
    }

    mReceiveReturnRingPhysAddr = mReceiveReturnRingDesc->getPhysicalSegment(0, 0);
    if (mReceiveReturnRingPhysAddr == 0)
    {
        DLOG("Receive return ring getPhysicalSegment() failed.");
        return false;
    }

    mReceiveReturnRingAddr = (bcmReceiveBD *) mReceiveReturnRingDesc->getBytesNoCopy();
    memset(mReceiveReturnRingAddr, 0, BGE_RX_RING_SZ);

    mRxPacketArray = IONew(mbuf_t, BGE_RX_RING_CNT);
    if (mRxPacketArray == 0)
    {
        DLOG("Can't get memory for RX packets.");
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
    UInt16 rxIndex = 0;
    UInt32 checksumValidMask = 0;
    mbuf_t packet;
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

        // Frame has vlan tag.
        if (receiveBD->flags & BGE_RXBDFLAG_VLAN_TAG)
            setVlanTag(packet, receiveBD->vlanTag);

        if (receiveBD->flags & BGE_RXBDFLAG_IP_CSUM)
            checksumValidMask |= kChecksumIP;

        if (receiveBD->flags & BGE_RXBDFLAG_TCP_UDP_CSUM)
            checksumValidMask |= (kChecksumTCP | kChecksumUDP);

        setChecksumResult(packet, kChecksumFamilyTCPIP,
                          (kChecksumIP | kChecksumTCP | kChecksumUDP), checksumValidMask);

        mNetworkInterface->inputPacket(packet, receiveBD->length - ETH_CRC_LEN,
                                       IONetworkInterface::kInputOptionQueuePacket);
        mNetStats->inputPackets += 1;
    }

    mRxReturnCons = returnRingCons;
    writeRegisterMailbox(BGE_MBX_RX_CONS0_LO, mRxReturnCons);
    if (packetsReceived > 0)
        writeRegisterMailbox(BGE_MBX_RX_STD_PROD_LO, mRxProducerProd);

    mNetworkInterface->flushInputQueue();
}
