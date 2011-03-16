#include "osx_bcm5906m.h"
#include "osx_bcm5906m_reg.h"

bool BCM5906MEthernet::allocateTxMemory()
{
    mSendRingDesc = IOBufferMemoryDescriptor::withOptions(kIOMemoryPhysicallyContiguous,
                                                          BGE_TX_RING_SZ, mPCICacheSize);
    if (mSendRingDesc == 0)
    {
        DLOG("No memory for send ring.");
        return false;
    }

    if (mSendRingDesc->prepare() != kIOReturnSuccess)
    {
        DLOG("Send ring memory prepare() failed.");
        mSendRingDesc->release();
        mSendRingDesc = 0;
        return false;
    }

    mSendRingPhysAddr = mSendRingDesc->getPhysicalSegment(0, 0);
    if (mSendRingPhysAddr == 0)
    {
        DLOG("Send ring getPhysicalSegment() failed.");
        return false;
    }

    mSendRingAddr = (bcmSendBD *) mSendRingDesc->getBytesNoCopy();
    memset(mSendRingAddr, 0, BGE_TX_RING_SZ);

    mTxPacketArray = IONew(mbuf_t, BGE_TX_RING_CNT);
    if (mTxPacketArray == 0)
    {
        DLOG("Can't get memory for TX packets.");
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
    UInt32 demandMask = 0;

    // Stall output if ring is almost full.
    if (BGE_TX_RING_CNT - mTxDescBusy < bcmMaxSegmentNum)
        return kIOReturnOutputStall;

    getChecksumDemand(packet, kChecksumFamilyTCPIP, &demandMask);

    // Check if we must compute an IP checksum.
    if (demandMask & kChecksumIP)
        flags |= BGE_TXBDFLAG_IP_CSUM;

    // Check if we must compute a TCP or UDP checksum.
    if ((demandMask & kChecksumTCP) || (demandMask & kChecksumUDP))
        flags |= BGE_TXBDFLAG_TCP_UDP_CSUM;

    // Check if packet has a VLAN tag.
    if (getVlanTagDemand(packet, (UInt32 *) &vlanTag))
        flags |= BGE_TXBDFLAG_VLAN_TAG;

    // Extract physical address and length pairs from the packet.
    fragCount = mMemoryCursor->getPhysicalSegmentsWithCoalesce(packet, vectors, bcmMaxSegmentNum);
    if (fragCount == 0)
        goto drop_packet;

    for (int i = 0; i < fragCount; i++)
    {
        BGE_HOSTADDR(bdAddr, vectors[i].location);
        OSWriteLittleInt32(&mSendRingAddr[currFrag].bufHostAddr.addrHi, 0, bdAddr.addrHi);
        OSWriteLittleInt32(&mSendRingAddr[currFrag].bufHostAddr.addrLo, 0, bdAddr.addrLo);
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
