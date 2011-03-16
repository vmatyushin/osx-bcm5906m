#include "osx_bcm5906m.h"
#include "osx_bcm5906m_reg.h"

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

bcmMediumType BCM5906MEthernet::phyGetActiveMedium()
{
    bcmMediumType medium;

    UInt32 anar = miiReadReg(BGE_MII_ANAR);
    UInt32 anlpar = miiReadReg(BGE_MII_ANLPAR);
    UInt32 common = anar & anlpar;

    if (common & BGE_MII_ANAR_T4)
        medium = BGE_MEDIUM_100T4;
    else if (common & BGE_MII_ANAR_TX_FD)
        medium = BGE_MEDIUM_100FD;
    else if (common & BGE_MII_ANAR_TX_HD)
        medium = BGE_MEDIUM_100HD;
    else if (common & BGE_MII_ANAR_10_FD)
        medium = BGE_MEDIUM_10FD;
    else
        medium = BGE_MEDIUM_10HD;

    return medium;
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
