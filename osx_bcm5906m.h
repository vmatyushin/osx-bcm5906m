#ifndef _OSX_BCM5906_H
#define _OSX_BCM5906_H

#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/network/IOEthernetController.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IONetworkInterface.h>
#include <IOKit/network/IOGatedOutputQueue.h>
#include <IOKit/network/IOPacketQueue.h>
#include <IOKit/network/IOMbufMemoryCursor.h>
#include <IOKit/IOFilterInterruptEventSource.h>
#include <IOKit/IOTimerEventSource.h>
#include <IOKit/IOBufferMemoryDescriptor.h>
#include <IOKit/IOMemoryDescriptor.h>
#include <sys/kpi_mbuf.h>
#include <net/kpi_interface.h>
#include <sys/errno.h>

#include "osx_bcm5906m_reg.h"

#define BCM5906MEthernet com_abiogenesis_driver_BCM5906MEthernet

#define DLOG(args...) {IOLog("-- BCM5906MEthernet --  "); IOLog(args); IOLog("\n");}
#define RELEASE(x) do { if(x) { (x)->release(); (x) = 0; } } while(0)
#define MBPS_10 10000000
#define MBPS_100 100000000

class BCM5906MEthernet : public IOEthernetController
{
    OSDeclareDefaultStructors(BCM5906MEthernet);

public:
    virtual bool start(IOService *provider);
    virtual void stop (IOService *provider);
    virtual void free (void);

    virtual IOReturn enable (IONetworkInterface *netif);
    virtual IOReturn disable(IONetworkInterface *netif);

    virtual IOReturn getHardwareAddress(IOEthernetAddress *address);
    virtual IOReturn setHardwareAddress(const IOEthernetAddress *address);

    virtual IOReturn setPromiscuousMode(bool active);
    virtual IOReturn setMulticastMode(bool active);
    virtual IOReturn setMulticastList(IOEthernetAddress *addrList, UInt32 count);
    virtual IOReturn getChecksumSupport(UInt32 *checksumMask, UInt32 checksumFamily, bool isOutput);

    virtual IOOutputQueue *createOutputQueue (void);
    virtual bool createWorkLoop(void);
    virtual IOWorkLoop *getWorkLoop(void) const;

    virtual bool configureInterface(IONetworkInterface *interface);
    virtual IOReturn selectMedium(const IONetworkMedium *medium);

    virtual const OSString *newVendorString(void) const;
    virtual const OSString *newModelString(void) const;

    virtual UInt32 outputPacket(mbuf_t packet, void *param);
    virtual void getPacketBufferConstraints(IOPacketBufferConstraints *constraints) const;

    virtual IOReturn registerWithPolicyMaker(IOService *policyMaker);

private:
    bool initDriverObjects(IOService *provider);
    void initPCIConfigSpace(IOPCIDevice *pci);

    // Receive
    bool allocateRxMemory();
    void releaseRxMemory();
    void freeRxRingPackets();
    bool initRxRing();
    void updateRxDescriptor(UInt32 index);
    void serviceRxInterrupt();

    // Transmit
    bool allocateTxMemory();
    void releaseTxMemory();
    void freeTxRingPackets();
    void initTxRing();
    void serviceTxInterrupt();

    // Status block
    bool allocateStatusBlockMemory();
    void releaseStatusBlockMemory();

    // Memory read/write
    UInt32 readNICMem(UInt32 offset);
    void writeNICMem(UInt32 offset, UInt32 data);

    void writeRegisterIndirect(UInt32 offset, UInt32 data);

    UInt32 readRegisterMailbox(UInt32 offset);
    void writeRegisterMailbox(UInt32 offset, UInt32 data);

    UInt32 readNICMemIndirect(UInt32 offset);
    void writeNICMemIndirect(UInt32 offset, UInt32 data);

    bool readNVRAMByte(UInt32 offset, UInt8 *dest);
    bool readEthernetAddress();

    // Chip
    bool initBlock();
    void initChip();
    void resetChip();
    void stopChip();

    // PHY
    UInt32 miiReadReg(UInt32 reg);
    void miiWriteReg(UInt32 reg, UInt32 data);
    bool phyInit();
    bool phySetMedium(bcmMediumType mediumType);
    bcmMediumType phyGetActiveMedium();
    void phyGetLinkStatus(bool firstPoll);

    // Medium
    void publishMedium();
    void addMediumType(UInt32 type, UInt32 speed, UInt32 index);
    IONetworkMedium *getMediumWithType(bcmMediumType type);

    // Event source handlers
    void interruptHandler(OSObject *owner, IOInterruptEventSource *sender, int count);
    bool interruptFilter(OSObject *owner, IOFilterInterruptEventSource *sender);
    void timeoutHandler(OSObject *owner, IOTimerEventSource *sender);

    // Miscellaneous
    UInt32 computeEthernetCRC32(IOEthernetAddress *addr);
    void updateStatistics();

    IOEthernetAddress mEtherAddr;
    IOEthernetInterface *mNetworkInterface;
    IOPCIDevice *mPCIDevice;
    IOWorkLoop *mWorkLoop;
    IOMemoryMap *mMemoryMap;
    IOVirtualAddress mNICBaseAddr;
    OSDictionary *mMediumDict;
    static const int mMediumTableSize = 6;
    IONetworkMedium *mMediumTable[mMediumTableSize];
    IOFilterInterruptEventSource *mInterruptSrc;
    IOTimerEventSource *mTimerSrc;
    static const UInt32 timerInterval = 5000;
    IONetworkStats *mNetStats;
    IOEthernetStats *mEtherStats;
    IOMbufNaturalMemoryCursor *mMemoryCursor;

    // Transmit
    IOBufferMemoryDescriptor *mSendRingDesc;
    IOPhysicalAddress mSendRingPhysAddr;
    bcmSendBD *mSendRingAddr;
    mbuf_t *mTxPacketArray;
    IOOutputQueue *mTransmitQueue;
    UInt32 mTxLastCons;
    UInt32 mTxProd;
    UInt32 mTxDescBusy;

    // Receive
    mbuf_t *mRxPacketArray;

    // Producer ring
    IOBufferMemoryDescriptor *mReceiveProducerRingDesc;
    IOPhysicalAddress mReceiveProducerRingPhysAddr;
    bcmReceiveBD *mReceiveProducerRingAddr;
    UInt32 mRxProducerProd;

    // Return ring
    IOBufferMemoryDescriptor *mReceiveReturnRingDesc;
    IOPhysicalAddress mReceiveReturnRingPhysAddr;
    bcmReceiveBD *mReceiveReturnRingAddr;
    UInt32 mRxReturnCons;

    // Status block
    IOBufferMemoryDescriptor *mStatusBlockDesc;
    IOPhysicalAddress mStatusBlockPhysAddr;
    bcmStatusBlock *mStatusBlockAddr;

    UInt16 mPHYPrevStatus;
    UInt32 mPCICacheSize;

#define bcmSetBit(reg, value) writeNICMem((reg), (readNICMem((reg)) | ((value))))
#define bcmClrBit(reg, value) writeNICMem((reg), (readNICMem((reg)) & ~((value))))
};

#endif // _OSX_BCM5906_H
