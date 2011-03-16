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
    writeNICMem(BFE_PCI_MEMWIN_BASEADDR, 0);

    // Configure mbuf memory pool.
    writeNICMem(BFE_BMAN_MBUFPOOL_BASEADDR, BFE_BUFFPOOL_1);
    writeNICMem(BFE_BMAN_MBUFPOOL_LEN, 0x18000);

    // Configure DMA resource pool.
    writeNICMem(BFE_BMAN_DMA_DESCPOOL_BASEADDR, BFE_DMA_DESCRIPTORS);
    writeNICMem(BFE_BMAN_DMA_DESCPOOL_LEN, 0x2000);

    // Configure mbuf pool watermarks.
    writeNICMem(BFE_BMAN_MBUFPOOL_READDMA_LOWAT, 0x0);
    writeNICMem(BFE_BMAN_MBUFPOOL_MACRX_LOWAT, 0x04);
    writeNICMem(BFE_BMAN_MBUFPOOL_HIWAT, 0x10);

    // Configure DMA resource watermarks.
    writeNICMem(BFE_BMAN_DMA_DESCPOOL_LOWAT, 5);
    writeNICMem(BFE_BMAN_DMA_DESCPOOL_HIWAT, 10);

    // Enable buffer manager.
    writeNICMem(BFE_BMAN_MODE, BFE_BMANMODE_ENABLE | BFE_BMANMODE_LOMBUF_ATTN);

    // Poll for buffer manager start indication.
    for (i = 0; i < BFE_TIMEOUT; i++)
    {
        IODelay(10);
        if (readNICMem(BFE_BMAN_MODE) & BFE_BMANMODE_ENABLE)
            break;
    }

    if (i == BFE_TIMEOUT)
    {
        DLOG("Can't start NIC buffer manager.");
        return false;
    }

    // Enable flow-through queues.
    writeNICMem(BFE_FTQ_RESET, 0xFFFFFFFF);
    writeNICMem(BFE_FTQ_RESET, 0);

    // Wait until queue initialization is complete.
    for (i = 0; i < BFE_TIMEOUT; i++)
    {
        IODelay(10);
        if (readNICMem(BFE_FTQ_RESET) == 0)
            break;
    }

    if (i == BFE_TIMEOUT)
    {
        DLOG("Flow-through queue init failed.");
        return false;
    }

    // Initialize the RX producer ring control block.
    BFE_HOSTADDR(ringAddr, mReceiveProducerRingPhysAddr);
    writeNICMem(BFE_RX_STD_RCB_HADDR_HI, ringAddr.addrHi);
    writeNICMem(BFE_RX_STD_RCB_HADDR_LO, ringAddr.addrLo);
    writeNICMem(BFE_RX_STD_RCB_MAXLEN_FLAGS, BFE_MAX_FRAMELEN << 16);
    writeNICMem(BFE_RX_STD_RCB_NICADDR, BFE_STD_RX_RINGS);

    // Set the BD ring replentish thresholds.
    writeNICMem(BFE_RBDI_STD_REPL_THRESH, 25);

    // Initialize send ring producer index mailbox register.
    writeNICMem(BFE_MBX_TX_HOST_PROD0_LO, 0);

    /*
     * Disable all unused send rings by setting the 'ring disabled'
     * bit in the flags field of all the TX send ring control blocks.
     * These are located in NIC memory.
     */
    offset = BFE_MEMWIN_START + BFE_SEND_RING_RCB;
    for (i = 0; i < BFE_TX_RINGS_EXTSSRAM_MAX; i++)
    {
        bcmRCBWrite(offset, maxlen_flags, BFE_RCB_FLAG_RING_DISABLED);
        bcmRCBWrite(offset, nicAddr, 0);
        offset += sizeof(bcmRCB);
    }

    // Configure TX RCB 0 (we use only the first ring).
    offset = BFE_MEMWIN_START + BFE_SEND_RING_RCB;
    BFE_HOSTADDR(ringAddr, mSendRingPhysAddr);
    bcmRCBWrite(offset, ringHostAddr.addrHi, ringAddr.addrHi);
    bcmRCBWrite(offset, ringHostAddr.addrLo, ringAddr.addrLo);
    bcmRCBWrite(offset, nicAddr, BFE_NIC_TXRING_ADDR);
    bcmRCBWrite(offset, maxlen_flags, BFE_TX_RING_CNT << 16);

    // Disable all unused RX return rings.
    offset = BFE_MEMWIN_START + BFE_RX_RETURN_RING_RCB;
    for (i = 0; i < BFE_RX_RINGS_MAX; i++)
    {
        bcmRCBWrite(offset, ringHostAddr.addrHi, 0);
        bcmRCBWrite(offset, ringHostAddr.addrLo, 0);
        bcmRCBWrite(offset, maxlen_flags, BFE_RCB_FLAG_RING_DISABLED);
        bcmRCBWrite(offset, nicAddr, 0);
        writeRegisterMailbox(BFE_MBX_RX_CONS0_LO + (i * (sizeof(UInt64))), 0);
        offset += sizeof(bcmRCB);
    }

    // Initialize producer ring producer index mailbox register.
    writeRegisterMailbox(BFE_MBX_RX_STD_PROD_LO, 0);

    /*
     * Set up RX return ring 0
     * Note that the NIC address for RX return rings is 0x0.
     * The return rings live entirely within the host, so the
     * nicaddr field in the RCB isn't used.
     */
    offset = BFE_MEMWIN_START + BFE_RX_RETURN_RING_RCB;
    BFE_HOSTADDR(ringAddr, mReceiveReturnRingPhysAddr);
    bcmRCBWrite(offset, ringHostAddr.addrHi, ringAddr.addrHi);
    bcmRCBWrite(offset, ringHostAddr.addrLo, ringAddr.addrLo);
    bcmRCBWrite(offset, maxlen_flags, BFE_RX_RING_CNT << 16);
    bcmRCBWrite(offset, nicAddr, 0);

    // Load MAC address.
    writeNICMem(BFE_MAC_ADDR1_LO, mEtherAddr.bytes[0] << 8  | (mEtherAddr.bytes[1]));
    writeNICMem(BFE_MAC_ADDR1_HI, mEtherAddr.bytes[2] << 24 | (mEtherAddr.bytes[3] << 16)|
                (mEtherAddr.bytes[4] << 8) | (mEtherAddr.bytes[5]));

    // Set random backoff seed for TX.
    writeNICMem(BFE_TX_RANDOM_BACKOFF,
                mEtherAddr.bytes[0] + mEtherAddr.bytes[1] + mEtherAddr.bytes[2] +
                mEtherAddr.bytes[3] + mEtherAddr.bytes[4] + mEtherAddr.bytes[5] +
                BFE_TX_BACKOFF_SEED_MASK);

    // Set inter-packet gap for transmit.
    writeNICMem(BFE_TX_LENGTHS, 0x2620);

    // Specify which ring to use for packets that don't match any RX rules.
    writeNICMem(BFE_RX_RULES_CFG, 0x8);

    /*
     * Configure number of RX lists. One interrupt distribution
     * list, sixteen active lists, one bad frames class.
     */
    writeNICMem(BFE_RXLP_CFG, 0x181);

    // Inialize RX list placement stats mask.
    writeNICMem(BFE_RXLP_STATS_ENABLE_MASK, 0xFFFFFF);
    writeNICMem(BFE_RXLP_STATS_CTL, 0x1);

    // Disable host coalescing until we get it set up.
    writeNICMem(BFE_HCC_MODE, 0x0);

    // Poll to make sure it's shut down.
    for (i = 0; i < BFE_TIMEOUT; i++)
    {
        IODelay(10);
        if (!(readNICMem(BFE_HCC_MODE) & BFE_HCCMODE_ENABLE))
            break;
    }

    if (i == BFE_TIMEOUT)
    {
        DLOG("Host coalescing engine failed to shut down.");
        return false;
    }

    // Set up host coalescing defaults.
    writeNICMem(BFE_HCC_RX_COAL_TICKS, 150);
    writeNICMem(BFE_HCC_TX_COAL_TICKS, 150);
    writeNICMem(BFE_HCC_RX_MAX_COAL_BDS, 10);
    writeNICMem(BFE_HCC_TX_MAX_COAL_BDS, 10);
    writeNICMem(BFE_HCC_RX_MAX_COAL_BDS_INT, 1);
    writeNICMem(BFE_HCC_TX_MAX_COAL_BDS_INT, 1);

    // Set up address of the statistics block.
    writeNICMem(BFE_HCC_STATUSBLK_BASEADDR, BFE_STATUS_BLOCK);

    // Set up address of the status block.
    writeNICMem(BFE_HCC_STATUSBLK_ADDR_HI, BFE_ADDR_HI(mStatusBlockPhysAddr));
    writeNICMem(BFE_HCC_STATUSBLK_ADDR_LO, BFE_ADDR_LO(mStatusBlockPhysAddr));

    // Set up status block size.
    val = BFE_STATBLKSZ_32BYTE;

    // Turn on host coalescing state machine.
    writeNICMem(BFE_HCC_MODE, val | BFE_HCCMODE_ENABLE);

    // Turn on RX BD completion state machine and enable attentions.
    writeNICMem(BFE_RBDC_MODE, BFE_RBDCMODE_ENABLE | BFE_RBDCMODE_ATTN);

    // Turn on RX list placement state machine.
    writeNICMem(BFE_RXLP_MODE, BFE_RXLPMODE_ENABLE);

    // Turn on RX list selector state machine.
    writeNICMem(BFE_RXLS_MODE, BFE_RXLSMODE_ENABLE);

    val = BFE_MACMODE_TXDMA_ENB | BFE_MACMODE_RXDMA_ENB |
    BFE_MACMODE_RX_STATS_CLEAR | BFE_MACMODE_TX_STATS_CLEAR |
    BFE_MACMODE_RX_STATS_ENB | BFE_MACMODE_TX_STATS_ENB |
    BFE_MACMODE_FRMHDR_DMA_ENB;
    val |= BFE_PORTMODE_MII;

    // Turn on DMA, clear stats.
    writeNICMem(BFE_MAC_MODE, val);

    // Set misc. local control, enable interrupts on attentions.
    writeNICMem(BFE_MISC_LOCAL_CTL, BFE_MLC_INTR_ONATTN);

    // Turn on DMA completion state machine.
    writeNICMem(BFE_DMAC_MODE, BFE_DMACMODE_ENABLE);

    // Turn on write DMA state machine.
    val = BFE_WDMAMODE_ENABLE | BFE_WDMAMODE_ALL_ATTNS | BFE_WDMAMODE_STATUS_TAG_FIX;
    writeNICMem(BFE_WDMA_MODE, val);
    IODelay(40);

    // Turn on read DMA state machine.
    val = BFE_RDMAMODE_ENABLE | BFE_RDMAMODE_ALL_ATTNS;
    val |= BFE_RDMAMODE_FIFO_LONG_BURST;
    writeNICMem(BFE_RDMA_MODE, val);
    IODelay(40);

    // Turn on RX data completion state machine.
    writeNICMem(BFE_RDC_MODE, BFE_RDCMODE_ENABLE | BFE_RDCMODE_ATTN);

    // Turn on RX BD initiator state machine.
    writeNICMem(BFE_RBDI_MODE, BFE_RBDIMODE_ENABLE | BFE_RBDIMODE_ATTN);

    // Turn on RX data and RX BD initiator state machine.
    writeNICMem(BFE_RDBDI_MODE, BFE_RDBDIMODE_ENABLE | BFE_RDBDIMODE_BADRINGSZ_ATTN);

    // Turn on Mbuf cluster free state machine.
    writeNICMem(BFE_MBCF_MODE, BFE_MBCFMODE_ENABLE);

    // Turn on send BD completion state machine.
    writeNICMem(BFE_SBDC_MODE, BFE_SBDCMODE_ENABLE);

    // Turn on send data completion state machine.
    writeNICMem(BFE_SDC_MODE, BFE_SDCMODE_ENABLE);

    // Turn on send data initiator state machine.
    writeNICMem(BFE_SDI_MODE, BFE_SDIMODE_ENABLE);

    // Turn on send BD initiator state machine.
    writeNICMem(BFE_SBDI_MODE, BFE_SBDIMODE_ENABLE);

    // Turn on send BD selector state machine.
    writeNICMem(BFE_SRS_MODE, BFE_SRSMODE_ENABLE);

    writeNICMem(BFE_SDI_STATS_ENABLE_MASK, 0xFFFFFF);
    writeNICMem(BFE_SDI_STATS_CTL, BFE_SDISTATSCTL_ENABLE | BFE_SDISTATSCTL_FASTER);

    // ack/clear link change events.
    writeNICMem(BFE_MAC_STS, BFE_MACSTAT_SYNC_CHANGED |
                BFE_MACSTAT_CFG_CHANGED | BFE_MACSTAT_MI_COMPLETE |
                BFE_MACSTAT_LINK_CHANGED);
    writeNICMem(BFE_MI_STS, 0);

    // Enable PHY auto polling (for MII/GMII only).
    bfeSetBit(BFE_MI_MODE, BFE_MIMODE_AUTOPOLL | (10 << 16));

    // Clear any pending link state attention.
    writeNICMem(BFE_MAC_STS, BFE_MACSTAT_SYNC_CHANGED |
                BFE_MACSTAT_CFG_CHANGED | BFE_MACSTAT_MI_COMPLETE |
                BFE_MACSTAT_LINK_CHANGED);

    // Enable link state change attentions.
    bfeSetBit(BFE_MAC_EVT_ENB, BFE_EVTENB_LINK_CHANGED);

    // Specify MTU.
    writeNICMem(BFE_RX_MTU, BFE_MAX_FRAMELEN);

    // Turn on transmitter.
    bfeSetBit(BFE_TX_MODE, BFE_TXMODE_ENABLE);

    // Turn on receiver.
    bfeSetBit(BFE_RX_MODE, BFE_RXMODE_ENABLE);

    // Tell firmware we're alive.
    bfeSetBit(BFE_MODE_CTL, BFE_MODECTL_STACKUP);

    // Enable host interrupts.
    bfeSetBit(BFE_PCI_MISC_CTL, BFE_PCIMISCCTL_CLEAR_INTA);
    bfeClrBit(BFE_PCI_MISC_CTL, BFE_PCIMISCCTL_MASK_PCI_INTR);
    writeRegisterMailbox(BFE_MBX_IRQ0_LO, 0);

    return true;
}

void BCM5906MEthernet::initChip()
{
    UInt32 dmaControlRegister;

    // Set endianness before we access any non-PCI registers.
    mPCIDevice->configWrite32(BFE_PCI_MISC_CTL, BFE_INIT);

    // Clear the MAC statistics block in the NIC's internal memory.
    for (int i = BFE_STATS_BLOCK; i < BFE_STATS_BLOCK_END + 1; i += sizeof(UInt32))
    {
        mPCIDevice->configWrite32(BFE_PCI_MEMWIN_BASEADDR, (0xFFFF0000 & i));
        writeNICMem(BFE_MEMWIN_START + (i & 0xFFFF), 0);
    }

    // Clear the status block in the NIC's internal memory.
    for (int i = BFE_STATUS_BLOCK; i < BFE_STATUS_BLOCK_END + 1; i += sizeof(UInt32))
    {
        mPCIDevice->configWrite32(BFE_PCI_MEMWIN_BASEADDR, (0xFFFF0000 & i));
        writeNICMem(BFE_MEMWIN_START + (i & 0xFFFF), 0);
    }

    // Set up the PCI DMA control register.
    dmaControlRegister = BFE_PCIDMARWCTL_RD_CMD_SHIFT(6) | BFE_PCIDMARWCTL_WR_CMD_SHIFT(7);
    // Read watermark not used, 128 bytes for write.
    dmaControlRegister |= BFE_PCIDMARWCTL_WR_WAT_SHIFT(3);
    mPCIDevice->configWrite32(BFE_PCI_DMA_RW_CTL, dmaControlRegister);

    // Set up general mode register.
    writeNICMem(BFE_MODE_CTL, BFE_DMA_SWAP_OPTIONS |
                BFE_MODECTL_MAC_ATTN_INTR |
                BFE_MODECTL_HOST_SEND_BDS |
                BFE_MODECTL_TX_NO_PHDR_CSUM);

    bfeSetBit(BFE_MODE_CTL, BFE_MODECTL_STACKUP);

    /*
     * Disable memory write invalidate.  Apparently it is not supported
     * properly by these devices.  Also ensure that INTx isn't disabled,
     * as these chips need it even when using MSI.
     */
    mPCIDevice->setConfigBits(kIOPCIConfigCommand, kIOPCICommandInterruptDisable | kIOPCICommandMemWrInvalidate,
                              ~(kIOPCICommandInterruptDisable | kIOPCICommandMemWrInvalidate));
    IODelay(40);

    // Put PHY into ready state.
    bfeClrBit(BFE_MISC_CFG, BFE_MISCCFG_EPHY_IDDQ);
    readNICMem(BFE_MISC_CFG);
    IODelay(40);
}

void BCM5906MEthernet::resetChip()
{
    UInt32 command, pcistate, reset, val, i;

    // Save some important PCI state.
    mPCICacheSize = mPCIDevice->configRead32(BFE_PCI_CACHESZ);
    command = mPCIDevice->configRead32(BFE_PCI_CMD);
    pcistate = mPCIDevice->configRead32(BFE_PCI_PCISTATE);

    mPCIDevice->configWrite32(BFE_PCI_MISC_CTL, BFE_PCIMISCCTL_INDIRECT_ACCESS |
                              BFE_PCIMISCCTL_MASK_PCI_INTR |
                              BFE_HIF_SWAP_OPTIONS |
                              BFE_PCIMISCCTL_PCISTATE_RW);

    // Enable Memory Arbiter.
    writeNICMem(BFE_MARB_MODE, BFE_MARBMODE_ENABLE);

    // Initialize the Miscellaneous Host Control register.
    mPCIDevice->configWrite32(BFE_PCI_MISC_CTL, BFE_PCIMISCCTL_INDIRECT_ACCESS |
                              BFE_PCIMISCCTL_MASK_PCI_INTR |
                              BFE_HIF_SWAP_OPTIONS |
                              BFE_PCIMISCCTL_PCISTATE_RW);

    /*
     * Write the magic number to SRAM at offset 0xB50.
     * When firmware finishes its initialization it will
     * write ~BFE_MAGIC_NUMBER to the same location.
     */
    writeNICMemIndirect(BFE_SOFTWARE_GENCOMM, BFE_MAGIC_NUMBER);

    // Prevent PCIE link training during global reset.
    writeNICMem(BFE_MISC_CFG, 1 << 29);

    // Issue global reset.
    reset = BFE_MISCCFG_RESET_CORE_CLOCKS;
    reset |= 1 << 29;
    writeRegisterIndirect(BFE_MISC_CFG, reset);

    bfeSetBit(BFE_VCPU_STATUS, BFE_VCPU_STATUS_DRV_RESET);
    bfeClrBit(BFE_VCPU_EXT_CTRL, BFE_VCPU_EXT_CTRL_HALT_CPU);
    IODelay(1000);

    // Disable host interrupts.
    bfeSetBit(BFE_PCI_MISC_CTL, BFE_PCIMISCCTL_MASK_PCI_INTR);
    writeRegisterMailbox(BFE_MBX_IRQ0_LO, 1);

    // Reset some of the PCI state that got zapped by reset.
    mPCIDevice->configWrite32(BFE_PCI_MISC_CTL, BFE_PCIMISCCTL_INDIRECT_ACCESS |
                              BFE_PCIMISCCTL_MASK_PCI_INTR |
                              BFE_HIF_SWAP_OPTIONS |
                              BFE_PCIMISCCTL_PCISTATE_RW);
    mPCIDevice->configWrite32(BFE_PCI_CACHESZ, mPCICacheSize);
    mPCIDevice->configWrite32(BFE_PCI_CMD, command);

    // Enable Memory Arbiter again.
    writeNICMem(BFE_MARB_MODE, BFE_MARBMODE_ENABLE);

    for (i = 0; i < BFE_TIMEOUT; i++)
    {
        val = readNICMem(BFE_VCPU_STATUS);
        if (val & BFE_VCPU_STATUS_INIT_DONE)
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
    for (i = 0; i < BFE_TIMEOUT; i++)
    {
        if (mPCIDevice->configRead32(BFE_PCI_PCISTATE) == pcistate)
            break;
        IODelay(10);
    }

    val = readNICMem(0x7C00);
    writeNICMem(0x7C00, val | (1 << 25));

    // Fix up byte swapping.
    writeNICMem(BFE_MODE_CTL, BFE_DMA_SWAP_OPTIONS | BFE_MODECTL_BYTESWAP_DATA);

    // Port mode = 01 (MII).
    writeNICMem(BFE_MAC_MODE, 0x4);

    IODelay(1000);
}

void BCM5906MEthernet::stopChip()
{
    // Disable host interrupts.
    bfeSetBit(BFE_PCI_MISC_CTL, BFE_PCIMISCCTL_MASK_PCI_INTR);
    writeRegisterMailbox(BFE_MBX_IRQ0_LO, 1);

    // Disable all of the receiver blocks.
    bfeClrBit(BFE_RX_MODE, BFE_RXMODE_ENABLE);
    bfeClrBit(BFE_RBDI_MODE, BFE_RBDIMODE_ENABLE);
    bfeClrBit(BFE_RXLP_MODE, BFE_RXLPMODE_ENABLE);
    bfeClrBit(BFE_RDBDI_MODE, BFE_RBDIMODE_ENABLE);
    bfeClrBit(BFE_RDC_MODE, BFE_RDCMODE_ENABLE);
    bfeClrBit(BFE_RBDC_MODE, BFE_RBDCMODE_ENABLE);

    // Disable all of the transmit blocks.
    bfeClrBit(BFE_SRS_MODE, BFE_SRSMODE_ENABLE);
    bfeClrBit(BFE_SBDI_MODE, BFE_SBDIMODE_ENABLE);
    bfeClrBit(BFE_SDI_MODE, BFE_SDIMODE_ENABLE);
    bfeClrBit(BFE_RDMA_MODE, BFE_RDMAMODE_ENABLE);
    bfeClrBit(BFE_SDC_MODE, BFE_SDCMODE_ENABLE);
    bfeClrBit(BFE_SBDC_MODE, BFE_SBDCMODE_ENABLE);

    // Shut down all of the memory managers and related state machines.
    bfeClrBit(BFE_HCC_MODE, BFE_HCCMODE_ENABLE);
    bfeClrBit(BFE_WDMA_MODE, BFE_WDMAMODE_ENABLE);

    writeNICMem(BFE_FTQ_RESET, 0xFFFFFFFF);
    writeNICMem(BFE_FTQ_RESET, 0);

    bfeClrBit(BFE_MODE_CTL, BFE_MODECTL_STACKUP);
}
