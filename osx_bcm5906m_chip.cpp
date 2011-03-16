#include "osx_bcm5906m.h"
#include "osx_bcm5906m_reg.h"

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
        DLOG("Can't start NIC buffer manager.");
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
        DLOG("Flow-through queue init failed.");
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
    writeNICMem(BGE_MAC_ADDR1_LO, mEtherAddr.bytes[0] << 8  | (mEtherAddr.bytes[1]));
    writeNICMem(BGE_MAC_ADDR1_HI, mEtherAddr.bytes[2] << 24 | (mEtherAddr.bytes[3] << 16)|
                (mEtherAddr.bytes[4] << 8) | (mEtherAddr.bytes[5]));

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

    // Set up address of the statistics block.
    writeNICMem(BGE_HCC_STATUSBLK_BASEADDR, BGE_STATUS_BLOCK);

    // Set up address of the status block.
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
    writeNICMem(BGE_SDC_MODE, BGE_SDCMODE_ENABLE);

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

    // Turn on transmitter.
    bcmSetBit(BGE_TX_MODE, BGE_TXMODE_ENABLE);

    // Turn on receiver.
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
                BGE_MODECTL_HOST_SEND_BDS |
                BGE_MODECTL_TX_NO_PHDR_CSUM);

    bcmSetBit(BGE_MODE_CTL, BGE_MODECTL_STACKUP);

    /*
     * Disable memory write invalidate.  Apparently it is not supported
     * properly by these devices.  Also ensure that INTx isn't disabled,
     * as these chips need it even when using MSI.
     */
    mPCIDevice->setConfigBits(kIOPCIConfigCommand, kIOPCICommandInterruptDisable | kIOPCICommandMemWrInvalidate,
                              ~(kIOPCICommandInterruptDisable | kIOPCICommandMemWrInvalidate));
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
