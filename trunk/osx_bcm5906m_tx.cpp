// Copyright (c) 2011, Vyacheslav Matyushin.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
// * Neither the name of the <organization> nor the
// names of its contributors may be used to endorse or promote products
// derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "osx_bcm5906m.h"
#include "osx_bcm5906m_reg.h"

bool BCM5906MEthernet::allocateTxMemory()
{
    mSendRingDesc = IOBufferMemoryDescriptor::withOptions(kIOMemoryPhysicallyContiguous,
                                                          BFE_TX_RING_SZ, mPCICacheSize);
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
    memset(mSendRingAddr, 0, BFE_TX_RING_SZ);

    mTxPacketArray = IONew(mbuf_t, BFE_TX_RING_CNT);
    if (mTxPacketArray == 0)
    {
        DLOG("Can't get memory for TX packets.");
        return false;
    }
    memset(mTxPacketArray, 0, BFE_TX_RING_CNT * sizeof(mbuf_t));

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
        IODelete(mTxPacketArray, mbuf_t, BFE_TX_RING_CNT);
        mTxPacketArray = 0;
    }
}

void BCM5906MEthernet::freeTxRingPackets()
{
    if (mTxPacketArray)
    {
        for (int i = 0; i < BFE_TX_RING_CNT; i++)
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
        memset(mSendRingAddr, 0, BFE_TX_RING_SZ);
    if (mTxPacketArray)
        memset(mTxPacketArray, 0, BFE_TX_RING_CNT * sizeof(mbuf_t));

    mTxLastCons = 0;
    mTxDescBusy = 0;
    mTxProd = 0;
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
        BFE_INC(index, BFE_TX_RING_CNT);

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
    if (BFE_TX_RING_CNT - mTxDescBusy < bcmMaxSegmentNum)
        return kIOReturnOutputStall;

    getChecksumDemand(packet, kChecksumFamilyTCPIP, &demandMask);

    // Check if we must compute an IP checksum.
    if (demandMask & kChecksumIP)
        flags |= BFE_TXBDFLAG_IP_CSUM;

    // Check if we must compute a TCP or UDP checksum.
    if ((demandMask & kChecksumTCP) || (demandMask & kChecksumUDP))
        flags |= BFE_TXBDFLAG_TCP_UDP_CSUM;

    // Check if packet has a VLAN tag.
    if (getVlanTagDemand(packet, (UInt32 *) &vlanTag))
        flags |= BFE_TXBDFLAG_VLAN_TAG;

    // Extract physical address and length pairs from the packet.
    fragCount = mMemoryCursor->getPhysicalSegmentsWithCoalesce(packet, vectors, bcmMaxSegmentNum);
    if (fragCount == 0)
        goto drop_packet;

    for (UInt32 i = 0; i < fragCount; i++)
    {
        BFE_HOSTADDR(bdAddr, vectors[i].location);
        OSWriteLittleInt32(&mSendRingAddr[currFrag].bufHostAddr.addrHi, 0, bdAddr.addrHi);
        OSWriteLittleInt32(&mSendRingAddr[currFrag].bufHostAddr.addrLo, 0, bdAddr.addrLo);
        OSWriteLittleInt16(&mSendRingAddr[currFrag].flags, 0, flags);
        OSWriteLittleInt16(&mSendRingAddr[currFrag].vlanTag, 0, vlanTag);
        OSWriteLittleInt16(&mSendRingAddr[currFrag].launchTime, 0, 0);
        OSWriteLittleInt16(&mSendRingAddr[currFrag].length, 0, vectors[i].length);

        prevCurrFrag = currFrag;
        BFE_INC(currFrag, BFE_TX_RING_CNT);
        mTxDescBusy++;
    }

    // Mark the last segment as end of packet.
    OSWriteLittleInt16(&mSendRingAddr[prevCurrFrag].flags, 0, flags | BFE_TXBDFLAG_END);

    // Attach mbuf packet to ring until transmission is complete.
    mTxProd = currFrag;
    mTxPacketArray[currFrag] = packet;

    // Update hardware pointer.
    writeRegisterMailbox(BFE_MBX_TX_HOST_PROD0_LO, mTxProd);

    mNetStats->outputPackets += 1;
    return kIOReturnOutputSuccess;

drop_packet:
    freePacket(packet);
    mNetStats->outputErrors += 1;
    return kIOReturnOutputDropped;
}
