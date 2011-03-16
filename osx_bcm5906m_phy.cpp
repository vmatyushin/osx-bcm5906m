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

UInt32 BCM5906MEthernet::miiReadReg(UInt32 reg)
{
    UInt32 data, autopoll, i;

    // Temporarily disable autopolling.
    autopoll = readNICMem(BFE_MI_MODE);
    if (autopoll & BFE_MIMODE_AUTOPOLL)
    {
        bfeClrBit(BFE_MI_MODE, BFE_MIMODE_AUTOPOLL);
        IODelay(40);
    }

    // Setup MII Communication register.
    writeNICMem(BFE_MI_COMM, BFE_MICMD_READ | BFE_MICOMM_BUSY | BFE_MIPHY(1) | BFE_MIREG(reg));

    for (i = 0; i < BFE_TIMEOUT; i++)
    {
        IODelay(10);
        data = readNICMem(BFE_MI_COMM);
        if (!(data & BFE_MICOMM_BUSY))
            break;
    }

    if (i == BFE_TIMEOUT)
        data = 0;
    else
    {
        IODelay(5);
        data = readNICMem(BFE_MI_COMM);
    }

    if (autopoll & BFE_MIMODE_AUTOPOLL)
    {
        bfeSetBit(BFE_MI_MODE, BFE_MIMODE_AUTOPOLL);
        IODelay(40);
    }

    if (data & BFE_MICOMM_READFAIL)
        return (0);

    return (data & 0xFFFF);
}

void BCM5906MEthernet::miiWriteReg(UInt32 reg, UInt32 data)
{
    UInt32 autopoll, i;

    // Temporarily disable autopolling.
    autopoll = readNICMem(BFE_MI_MODE);
    if (autopoll & BFE_MIMODE_AUTOPOLL)
    {
        bfeClrBit(BFE_MI_MODE, BFE_MIMODE_AUTOPOLL);
        IODelay(40);
    }

    // Setup MII Communication register.
    writeNICMem(BFE_MI_COMM, BFE_MICMD_WRITE | BFE_MICOMM_BUSY | BFE_MIPHY(1) | BFE_MIREG(reg) | data);

    for (i = 0; i < BFE_TIMEOUT; i++)
    {
        IODelay(10);
        if (!(readNICMem(BFE_MI_COMM) & BFE_MICOMM_BUSY))
        {
            IODelay(5);
            readNICMem(BFE_MI_COMM);
            break;
        }
    }

    if (autopoll & BFE_MIMODE_AUTOPOLL)
    {
        bfeSetBit(BFE_MI_MODE, BFE_MIMODE_AUTOPOLL);
        IODelay(40);
    }
}

bool BCM5906MEthernet::phyInit()
{
    UInt32 phyControl, i;

    // Reset PHY.
    phyControl = BFE_PHY_RESET;
    miiWriteReg(BFE_MII_CTL, phyControl);

    for (i = 0; i < BFE_TIMEOUT; i++)
    {
        phyControl = miiReadReg(BFE_MII_CTL);
        if ((phyControl & BFE_PHY_RESET) == 0)
            break;
        IODelay(10);
    }
    if (i == BFE_TIMEOUT)
    {
        DLOG("PHY reset failed.");
        return false;
    }

    // Init PHY.
    // Disable link events.
    writeNICMem(BFE_MAC_EVT_ENB, 0);

    // Clear link attentions.
    bfeClrBit(BFE_MAC_STS, BFE_MACSTAT_LINK_CHANGED);

    // Disable autopolling mode.
    writeNICMem(BFE_MI_MODE, 0xC0020);
    IODelay(40);

    // Acknowledge outstanding interrupts (must read twice).
    miiReadReg(BFE_MII_INTERRUPT);
    miiReadReg(BFE_MII_INTERRUPT);

    // Enable autopolling mode.
    bfeSetBit(BFE_MI_MODE, BFE_MIMODE_AUTOPOLL);

    // Enable link attentions.
    writeNICMem(BFE_MAC_EVT_ENB, BFE_EVTENB_LINK_CHANGED);
    bfeSetBit(BFE_MODE_CTL, BFE_MODECTL_MAC_ATTN_INTR);

    return true;
}

bool BCM5906MEthernet::phySetMedium(bcmMediumType mediumType)
{
    // Reset PHY.
    phyInit();

    if (mediumType == BFE_MEDIUM_AUTO)
        return true;

    UInt32 miiControl = miiReadReg(BFE_MII_CTL);
    miiControl &= BFE_MII_CTL_AUTONEG_DISABLE;

    switch (mediumType)
    {
        case BFE_MEDIUM_10HD:
            miiControl &= BFE_MII_CTL_FORCED_10;
            miiControl &= BFE_MII_CTL_DUPLEX_HALF;
            break;

        case BFE_MEDIUM_10FD:
            miiControl &= BFE_MII_CTL_FORCED_10;
            miiControl |= BFE_MII_CTL_DUPLEX_FULL;
            break;

        case BFE_MEDIUM_100HD:
        case BFE_MEDIUM_100T4:
            miiControl |= BFE_MII_CTL_FORCED_100;
            miiControl &= BFE_MII_CTL_DUPLEX_HALF;
            break;

        case BFE_MEDIUM_100FD:
            miiControl |= BFE_MII_CTL_FORCED_100;
            miiControl |= BFE_MII_CTL_DUPLEX_FULL;
            break;

        default:
            return false;
            break;
    }

    miiWriteReg(BFE_MII_CTL, miiControl);
    return true;
}

bcmMediumType BCM5906MEthernet::phyGetActiveMedium()
{
    bcmMediumType medium;

    UInt32 anar = miiReadReg(BFE_MII_ANAR);
    UInt32 anlpar = miiReadReg(BFE_MII_ANLPAR);
    UInt32 common = anar & anlpar;

    if (common & BFE_MII_ANAR_T4)
        medium = BFE_MEDIUM_100T4;
    else if (common & BFE_MII_ANAR_TX_FD)
        medium = BFE_MEDIUM_100FD;
    else if (common & BFE_MII_ANAR_TX_HD)
        medium = BFE_MEDIUM_100HD;
    else if (common & BFE_MII_ANAR_10_FD)
        medium = BFE_MEDIUM_10FD;
    else
        medium = BFE_MEDIUM_10HD;

    return medium;
}

void BCM5906MEthernet::phyGetLinkStatus(bool firstPoll)
{
    UInt32 miiStatus = miiReadReg(BFE_MII_STATUS);
    UInt32 statusChange, i;

    // Detect a change in the two link related bits.
    statusChange = (mPHYPrevStatus ^ miiStatus) & (BFE_MII_STS_LINK | BFE_MII_STS_AUTONEG_COMP);

    if (statusChange || firstPoll)
    {
        if (firstPoll)
        {
            // For the initial link status poll, wait a bit, then
            // re-read the status register to clear any latched bits.
            i = 5000;

            while (i > 0)
            {
                miiStatus = miiReadReg(BFE_MII_STATUS);
                if (!miiStatus)
                    break;

                if (miiStatus & BFE_MII_STS_AUTONEG_COMP)
                    break;

                IOSleep(20);
                i -= 20;
            }

            miiStatus = miiReadReg(BFE_MII_STATUS);
            miiStatus = miiReadReg(BFE_MII_STATUS);
        }

        // Determine link status.
        if (miiStatus & BFE_MII_STS_LINK)
        {
            // Link is up.
            IONetworkMedium *activeMedium = getMediumWithType(phyGetActiveMedium());
            setLinkStatus(kIONetworkLinkValid | kIONetworkLinkActive, activeMedium);
        }
        else
        {
            // Link is down.
            setLinkStatus(kIONetworkLinkValid, 0);
        }

        // Save status.
        mPHYPrevStatus = miiStatus;
    }
}
