#ifndef _OSX_BCM5906_REG_H
#define _OSX_BCM5906_REG_H

#define BGE_PAGE_ZERO                     0x00000000
#define BGE_PAGE_ZERO_END                 0x000000FF
#define BGE_SEND_RING_RCB                 0x00000100
#define BGE_SEND_RING_RCB_END             0x000001FF
#define BGE_RX_RETURN_RING_RCB            0x00000200
#define BGE_RX_RETURN_RING_RCB_END        0x000002FF
#define BGE_STATS_BLOCK                   0x00000300
#define BGE_STATS_BLOCK_END               0x00000AFF
#define BGE_STATUS_BLOCK                  0x00000B00
#define BGE_STATUS_BLOCK_END              0x00000B4F
#define BGE_SOFTWARE_GENCOMM              0x00000B50
#define BGE_SOFTWARE_GENCOMM_SIG          0x00000B54
#define BGE_SOFTWARE_GENCOMM_NICCFG       0x00000B58
#define BGE_SOFTWARE_GENCOMM_FW           0x00000B78
#define BGE_SOFTWARE_GENNCOMM_FW_LEN      0x00000B7C
#define BGE_SOFTWARE_GENNCOMM_FW_DATA     0x00000B80
#define BGE_SOFTWARE_GENCOMM_END          0x00000FFF
#define BGE_UNMAPPED                      0x00001000
#define BGE_UNMAPPED_END                  0x00001FFF
#define BGE_DMA_DESCRIPTORS               0x00002000
#define BGE_DMA_DESCRIPTORS_END           0x00003FFF
#define BGE_SEND_RING_1_TO_4              0x00004000
#define BGE_SEND_RING_1_TO_4_END          0x00005FFF

/* Mappings for internal memory configuration */
#define BGE_STD_RX_RINGS               0x00006000
#define BGE_STD_RX_RINGS_END           0x00006FFF
#define BGE_JUMBO_RX_RINGS             0x00007000
#define BGE_JUMBO_RX_RINGS_END         0x00007FFF
#define BGE_BUFFPOOL_1                 0x00008000
#define BGE_BUFFPOOL_1_END             0x0000FFFF
#define BGE_BUFFPOOL_2                 0x00010000
#define BGE_BUFFPOOL_2_END             0x00017FFF
#define BGE_BUFFPOOL_3                 0x00018000
#define BGE_BUFFPOOL_3_END             0x0001FFFF

/* Mappings for external SSRAM configurations */
#define BGE_SEND_RING_5_TO_6           0x00006000
#define BGE_SEND_RING_5_TO_6_END       0x00006FFF
#define BGE_SEND_RING_7_TO_8           0x00007000
#define BGE_SEND_RING_7_TO_8_END       0x00007FFF
#define BGE_SEND_RING_9_TO_16          0x00008000
#define BGE_SEND_RING_9_TO_16_END      0x0000BFFF
#define BGE_EXT_STD_RX_RINGS           0x0000C000
#define BGE_EXT_STD_RX_RINGS_END       0x0000CFFF
#define BGE_EXT_JUMBO_RX_RINGS         0x0000D000
#define BGE_EXT_JUMBO_RX_RINGS_END     0x0000DFFF
#define BGE_MINI_RX_RINGS              0x0000E000
#define BGE_MINI_RX_RINGS_END          0x0000FFFF
#define BGE_AVAIL_REGION1              0x00010000
#define BGE_AVAIL_REGION1_END          0x00017FFF
#define BGE_AVAIL_REGION2              0x00018000
#define BGE_AVAIL_REGION2_END          0x0001FFFF
#define BGE_EXT_SSRAM                  0x00020000
#define BGE_EXT_SSRAM_END              0x000FFFFF


/*
 * BCM570x register offsets. These are memory mapped registers
 * which can be accessed with the CSR_READ_4()/CSR_WRITE_4() macros.
 * Each register must be accessed using 32 bit operations.
 *
 * All registers are accessed through a 32K shared memory block.
 * The first group of registers are actually copies of the PCI
 * configuration space registers.
 */

/*
 * PCI registers defined in the PCI 2.2 spec.
 */
#define BGE_PCI_VID                 0x00
#define BGE_PCI_DID                 0x02
#define BGE_PCI_CMD                 0x04
#define BGE_PCI_STS                 0x06
#define BGE_PCI_REV                 0x08
#define BGE_PCI_CLASS               0x09
#define BGE_PCI_CACHESZ             0x0C
#define BGE_PCI_LATTIMER            0x0D
#define BGE_PCI_HDRTYPE             0x0E
#define BGE_PCI_BIST                0x0F
#define BGE_PCI_BAR0                0x10
#define BGE_PCI_BAR1                0x14
#define BGE_PCI_SUBSYS              0x2C
#define BGE_PCI_SUBVID              0x2E
#define BGE_PCI_ROMBASE             0x30
#define BGE_PCI_CAPPTR              0x34
#define BGE_PCI_INTLINE             0x3C
#define BGE_PCI_INTPIN              0x3D
#define BGE_PCI_MINGNT              0x3E
#define BGE_PCI_MAXLAT              0x3F
#define BGE_PCI_PCIXCAP             0x40
#define BGE_PCI_NEXTPTR_PM          0x41
#define BGE_PCI_PCIX_CMD            0x42
#define BGE_PCI_PCIX_STS            0x44
#define BGE_PCI_PWRMGMT_CAPID       0x48
#define BGE_PCI_NEXTPTR_VPD         0x49
#define BGE_PCI_PWRMGMT_CAPS        0x4A
#define BGE_PCI_PWRMGMT_CMD         0x4C
#define BGE_PCI_PWRMGMT_STS         0x4D
#define BGE_PCI_PWRMGMT_DATA        0x4F
#define BGE_PCI_VPD_CAPID           0x50
#define BGE_PCI_NEXTPTR_MSI         0x51
#define BGE_PCI_VPD_ADDR            0x52
#define BGE_PCI_VPD_DATA            0x54
#define BGE_PCI_MSI_CAPID           0x58
#define BGE_PCI_NEXTPTR_NONE        0x59
#define BGE_PCI_MSI_CTL             0x5A
#define BGE_PCI_MSI_ADDR_HI         0x5C
#define BGE_PCI_MSI_ADDR_LO         0x60
#define BGE_PCI_MSI_DATA            0x64

/*
 * PCI registers specific to the BCM570x family.
 */
#define BGE_PCI_MISC_CTL                   0x68
#define BGE_PCI_DMA_RW_CTL                 0x6C
#define BGE_PCI_PCISTATE                   0x70
#define BGE_PCI_CLKCTL                     0x74
#define BGE_PCI_REG_BASEADDR               0x78
#define BGE_PCI_MEMWIN_BASEADDR            0x7C
#define BGE_PCI_REG_DATA                   0x80
#define BGE_PCI_MEMWIN_DATA                0x84
#define BGE_PCI_MODECTL                    0x88
#define BGE_PCI_MISC_CFG                   0x8C
#define BGE_PCI_MISC_LOCALCTL              0x90
#define BGE_PCI_UNDI_RX_STD_PRODIDX_HI     0x98
#define BGE_PCI_UNDI_RX_STD_PRODIDX_LO     0x9C
#define BGE_PCI_UNDI_RX_RTN_CONSIDX_HI     0xA0
#define BGE_PCI_UNDI_RX_RTN_CONSIDX_LO     0xA4
#define BGE_PCI_UNDI_TX_BD_PRODIDX_HI      0xA8
#define BGE_PCI_UNDI_TX_BD_PRODIDX_LO      0xAC
#define BGE_PCI_ISR_MBX_HI                 0xB0
#define BGE_PCI_ISR_MBX_LO                 0xB4
#define BGE_PCI_PRODID_ASICREV             0xBC

/* PCI Misc. Host control register */
#define BGE_PCIMISCCTL_CLEAR_INTA          0x00000001
#define BGE_PCIMISCCTL_MASK_PCI_INTR       0x00000002
#define BGE_PCIMISCCTL_ENDIAN_BYTESWAP     0x00000004
#define BGE_PCIMISCCTL_ENDIAN_WORDSWAP     0x00000008
#define BGE_PCIMISCCTL_PCISTATE_RW         0x00000010
#define BGE_PCIMISCCTL_CLOCKCTL_RW         0x00000020
#define BGE_PCIMISCCTL_REG_WORDSWAP        0x00000040
#define BGE_PCIMISCCTL_INDIRECT_ACCESS     0x00000080
#define BGE_PCIMISCCTL_ASICREV             0xFFFF0000
#define BGE_PCIMISCCTL_ASICREV_SHIFT               16

#define BGE_HIF_SWAP_OPTIONS    (BGE_PCIMISCCTL_ENDIAN_WORDSWAP)

#define BGE_DMA_SWAP_OPTIONS BGE_MODECTL_WORDSWAP_NONFRAME | \
                             BGE_MODECTL_BYTESWAP_DATA | \
                             BGE_MODECTL_WORDSWAP_DATA

#define BGE_INIT             (BGE_HIF_SWAP_OPTIONS | \
                              BGE_PCIMISCCTL_CLEAR_INTA | \
                              BGE_PCIMISCCTL_MASK_PCI_INTR | \
                              BGE_PCIMISCCTL_INDIRECT_ACCESS)

/* PCI DMA Read/Write Control register */
#define BGE_PCIDMARWCTL_MINDMA                   0x000000FF
#define BGE_PCIDMARWCTL_RDADRR_BNDRY             0x00000700
#define BGE_PCIDMARWCTL_WRADDR_BNDRY             0x00003800
#define BGE_PCIDMARWCTL_ONEDMA_ATONCE            0x0000C000
#define BGE_PCIDMARWCTL_ONEDMA_ATONCE_GLOBAL     0x00004000
#define BGE_PCIDMARWCTL_ONEDMA_ATONCE_LOCAL      0x00008000
#define BGE_PCIDMARWCTL_RD_WAT                   0x00070000
#define BGE_PCIDMARWCTL_WR_WAT                   0x00380000
#define BGE_PCIDMARWCTL_USE_MRM                  0x00400000
#define BGE_PCIDMARWCTL_ASRT_ALL_BE              0x00800000
#define BGE_PCIDMARWCTL_DFLT_PCI_RD_CMD          0x0F000000
#define BGE_PCIDMARWCTL_DFLT_PCI_WR_CMD          0xF0000000

#define BGE_PCIDMARWCTL_RD_WAT_SHIFT(x)    ((x) << 16)
#define BGE_PCIDMARWCTL_WR_WAT_SHIFT(x)    ((x) << 19)
#define BGE_PCIDMARWCTL_RD_CMD_SHIFT(x)    ((x) << 24)
#define BGE_PCIDMARWCTL_WR_CMD_SHIFT(x)    ((x) << 28)

/*
 * High priority mailbox registers
 * Each mailbox is 64-bits wide, though we only use the
 * lower 32 bits. To write a 64-bit value, write the upper 32 bits
 * first. The NIC will load the mailbox after the lower 32 bit word
 * has been updated.
 */
#define BGE_MBX_IRQ0_HI                 0x0200
#define BGE_MBX_IRQ0_LO                 0x0204
#define BGE_MBX_IRQ1_HI                 0x0208
#define BGE_MBX_IRQ1_LO                 0x020C
#define BGE_MBX_IRQ2_HI                 0x0210
#define BGE_MBX_IRQ2_LO                 0x0214
#define BGE_MBX_IRQ3_HI                 0x0218
#define BGE_MBX_IRQ3_LO                 0x021C
#define BGE_MBX_GEN0_HI                 0x0220
#define BGE_MBX_GEN0_LO                 0x0224
#define BGE_MBX_GEN1_HI                 0x0228
#define BGE_MBX_GEN1_LO                 0x022C
#define BGE_MBX_GEN2_HI                 0x0230
#define BGE_MBX_GEN2_LO                 0x0234
#define BGE_MBX_GEN3_HI                 0x0228
#define BGE_MBX_GEN3_LO                 0x022C
#define BGE_MBX_GEN4_HI                 0x0240
#define BGE_MBX_GEN4_LO                 0x0244
#define BGE_MBX_GEN5_HI                 0x0248
#define BGE_MBX_GEN5_LO                 0x024C
#define BGE_MBX_GEN6_HI                 0x0250
#define BGE_MBX_GEN6_LO                 0x0254
#define BGE_MBX_GEN7_HI                 0x0258
#define BGE_MBX_GEN7_LO                 0x025C
#define BGE_MBX_RELOAD_STATS_HI         0x0260
#define BGE_MBX_RELOAD_STATS_LO         0x0264
#define BGE_MBX_RX_STD_PROD_HI          0x0268
#define BGE_MBX_RX_STD_PROD_LO          0x026C
#define BGE_MBX_RX_JUMBO_PROD_HI        0x0270
#define BGE_MBX_RX_JUMBO_PROD_LO        0x0274
#define BGE_MBX_RX_MINI_PROD_HI         0x0278
#define BGE_MBX_RX_MINI_PROD_LO         0x027C
#define BGE_MBX_RX_CONS0_HI             0x0280
#define BGE_MBX_RX_CONS0_LO             0x0284
#define BGE_MBX_RX_CONS1_HI             0x0288
#define BGE_MBX_RX_CONS1_LO             0x028C
#define BGE_MBX_RX_CONS2_HI             0x0290
#define BGE_MBX_RX_CONS2_LO             0x0294
#define BGE_MBX_RX_CONS3_HI             0x0298
#define BGE_MBX_RX_CONS3_LO             0x029C
#define BGE_MBX_RX_CONS4_HI             0x02A0
#define BGE_MBX_RX_CONS4_LO             0x02A4
#define BGE_MBX_RX_CONS5_HI             0x02A8
#define BGE_MBX_RX_CONS5_LO             0x02AC
#define BGE_MBX_RX_CONS6_HI             0x02B0
#define BGE_MBX_RX_CONS6_LO             0x02B4
#define BGE_MBX_RX_CONS7_HI             0x02B8
#define BGE_MBX_RX_CONS7_LO             0x02BC
#define BGE_MBX_RX_CONS8_HI             0x02C0
#define BGE_MBX_RX_CONS8_LO             0x02C4
#define BGE_MBX_RX_CONS9_HI             0x02C8
#define BGE_MBX_RX_CONS9_LO             0x02CC
#define BGE_MBX_RX_CONS10_HI            0x02D0
#define BGE_MBX_RX_CONS10_LO            0x02D4
#define BGE_MBX_RX_CONS11_HI            0x02D8
#define BGE_MBX_RX_CONS11_LO            0x02DC
#define BGE_MBX_RX_CONS12_HI            0x02E0
#define BGE_MBX_RX_CONS12_LO            0x02E4
#define BGE_MBX_RX_CONS13_HI            0x02E8
#define BGE_MBX_RX_CONS13_LO            0x02EC
#define BGE_MBX_RX_CONS14_HI            0x02F0
#define BGE_MBX_RX_CONS14_LO            0x02F4
#define BGE_MBX_RX_CONS15_HI            0x02F8
#define BGE_MBX_RX_CONS15_LO            0x02FC
#define BGE_MBX_TX_HOST_PROD0_HI        0x0300
#define BGE_MBX_TX_HOST_PROD0_LO        0x0304
#define BGE_MBX_TX_HOST_PROD1_HI        0x0308
#define BGE_MBX_TX_HOST_PROD1_LO        0x030C
#define BGE_MBX_TX_HOST_PROD2_HI        0x0310
#define BGE_MBX_TX_HOST_PROD2_LO        0x0314
#define BGE_MBX_TX_HOST_PROD3_HI        0x0318
#define BGE_MBX_TX_HOST_PROD3_LO        0x031C
#define BGE_MBX_TX_HOST_PROD4_HI        0x0320
#define BGE_MBX_TX_HOST_PROD4_LO        0x0324
#define BGE_MBX_TX_HOST_PROD5_HI        0x0328
#define BGE_MBX_TX_HOST_PROD5_LO        0x032C
#define BGE_MBX_TX_HOST_PROD6_HI        0x0330
#define BGE_MBX_TX_HOST_PROD6_LO        0x0334
#define BGE_MBX_TX_HOST_PROD7_HI        0x0338
#define BGE_MBX_TX_HOST_PROD7_LO        0x033C
#define BGE_MBX_TX_HOST_PROD8_HI        0x0340
#define BGE_MBX_TX_HOST_PROD8_LO        0x0344
#define BGE_MBX_TX_HOST_PROD9_HI        0x0348
#define BGE_MBX_TX_HOST_PROD9_LO        0x034C
#define BGE_MBX_TX_HOST_PROD10_HI       0x0350
#define BGE_MBX_TX_HOST_PROD10_LO       0x0354
#define BGE_MBX_TX_HOST_PROD11_HI       0x0358
#define BGE_MBX_TX_HOST_PROD11_LO       0x035C
#define BGE_MBX_TX_HOST_PROD12_HI       0x0360
#define BGE_MBX_TX_HOST_PROD12_LO       0x0364
#define BGE_MBX_TX_HOST_PROD13_HI       0x0368
#define BGE_MBX_TX_HOST_PROD13_LO       0x036C
#define BGE_MBX_TX_HOST_PROD14_HI       0x0370
#define BGE_MBX_TX_HOST_PROD14_LO       0x0374
#define BGE_MBX_TX_HOST_PROD15_HI       0x0378
#define BGE_MBX_TX_HOST_PROD15_LO       0x037C
#define BGE_MBX_TX_NIC_PROD0_HI         0x0380
#define BGE_MBX_TX_NIC_PROD0_LO         0x0384
#define BGE_MBX_TX_NIC_PROD1_HI         0x0388
#define BGE_MBX_TX_NIC_PROD1_LO         0x038C
#define BGE_MBX_TX_NIC_PROD2_HI         0x0390
#define BGE_MBX_TX_NIC_PROD2_LO         0x0394
#define BGE_MBX_TX_NIC_PROD3_HI         0x0398
#define BGE_MBX_TX_NIC_PROD3_LO         0x039C
#define BGE_MBX_TX_NIC_PROD4_HI         0x03A0
#define BGE_MBX_TX_NIC_PROD4_LO         0x03A4
#define BGE_MBX_TX_NIC_PROD5_HI         0x03A8
#define BGE_MBX_TX_NIC_PROD5_LO         0x03AC
#define BGE_MBX_TX_NIC_PROD6_HI         0x03B0
#define BGE_MBX_TX_NIC_PROD6_LO         0x03B4
#define BGE_MBX_TX_NIC_PROD7_HI         0x03B8
#define BGE_MBX_TX_NIC_PROD7_LO         0x03BC
#define BGE_MBX_TX_NIC_PROD8_HI         0x03C0
#define BGE_MBX_TX_NIC_PROD8_LO         0x03C4
#define BGE_MBX_TX_NIC_PROD9_HI         0x03C8
#define BGE_MBX_TX_NIC_PROD9_LO         0x03CC
#define BGE_MBX_TX_NIC_PROD10_HI        0x03D0
#define BGE_MBX_TX_NIC_PROD10_LO        0x03D4
#define BGE_MBX_TX_NIC_PROD11_HI        0x03D8
#define BGE_MBX_TX_NIC_PROD11_LO        0x03DC
#define BGE_MBX_TX_NIC_PROD12_HI        0x03E0
#define BGE_MBX_TX_NIC_PROD12_LO        0x03E4
#define BGE_MBX_TX_NIC_PROD13_HI        0x03E8
#define BGE_MBX_TX_NIC_PROD13_LO        0x03EC
#define BGE_MBX_TX_NIC_PROD14_HI        0x03F0
#define BGE_MBX_TX_NIC_PROD14_LO        0x03F4
#define BGE_MBX_TX_NIC_PROD15_HI        0x03F8
#define BGE_MBX_TX_NIC_PROD15_LO        0x03FC

#define BGE_TX_RINGS_MAX        4
#define BGE_TX_RINGS_EXTSSRAM_MAX    16
#define BGE_RX_RINGS_MAX        16

/* Ethernet MAC control registers */
#define BGE_MAC_MODE                  0x0400
#define BGE_MAC_STS                   0x0404
#define BGE_MAC_EVT_ENB               0x0408
#define BGE_MAC_LED_CTL               0x040C
#define BGE_MAC_ADDR1_LO              0x0410
#define BGE_MAC_ADDR1_HI              0x0414
#define BGE_MAC_ADDR2_LO              0x0418
#define BGE_MAC_ADDR2_HI              0x041C
#define BGE_MAC_ADDR3_LO              0x0420
#define BGE_MAC_ADDR3_HI              0x0424
#define BGE_MAC_ADDR4_LO              0x0428
#define BGE_MAC_ADDR4_HI              0x042C
#define BGE_WOL_PATPTR                0x0430
#define BGE_WOL_PATCFG                0x0434
#define BGE_TX_RANDOM_BACKOFF         0x0438
#define BGE_RX_MTU                    0x043C
#define BGE_GBIT_PCS_TEST             0x0440
#define BGE_TX_TBI_AUTONEG            0x0444
#define BGE_RX_TBI_AUTONEG            0x0448
#define BGE_MI_COMM                   0x044C
#define BGE_MI_STS                    0x0450
#define BGE_MI_MODE                   0x0454
#define BGE_AUTOPOLL_STS              0x0458
#define BGE_TX_MODE                   0x045C
#define BGE_TX_STS                    0x0460
#define BGE_TX_LENGTHS                0x0464
#define BGE_RX_MODE                   0x0468
#define BGE_RX_STS                    0x046C
#define BGE_MAR0                      0x0470
#define BGE_MAR1                      0x0474
#define BGE_MAR2                      0x0478
#define BGE_MAR3                      0x047C
#define BGE_RX_BD_RULES_CTL0          0x0480
#define BGE_RX_BD_RULES_MASKVAL0      0x0484
#define BGE_RX_BD_RULES_CTL1          0x0488
#define BGE_RX_BD_RULES_MASKVAL1      0x048C
#define BGE_RX_BD_RULES_CTL2          0x0490
#define BGE_RX_BD_RULES_MASKVAL2      0x0494
#define BGE_RX_BD_RULES_CTL3          0x0498
#define BGE_RX_BD_RULES_MASKVAL3      0x049C
#define BGE_RX_BD_RULES_CTL4          0x04A0
#define BGE_RX_BD_RULES_MASKVAL4      0x04A4
#define BGE_RX_BD_RULES_CTL5          0x04A8
#define BGE_RX_BD_RULES_MASKVAL5      0x04AC
#define BGE_RX_BD_RULES_CTL6          0x04B0
#define BGE_RX_BD_RULES_MASKVAL6      0x04B4
#define BGE_RX_BD_RULES_CTL7          0x04B8
#define BGE_RX_BD_RULES_MASKVAL7      0x04BC
#define BGE_RX_BD_RULES_CTL8          0x04C0
#define BGE_RX_BD_RULES_MASKVAL8      0x04C4
#define BGE_RX_BD_RULES_CTL9          0x04C8
#define BGE_RX_BD_RULES_MASKVAL9      0x04CC
#define BGE_RX_BD_RULES_CTL10         0x04D0
#define BGE_RX_BD_RULES_MASKVAL10     0x04D4
#define BGE_RX_BD_RULES_CTL11         0x04D8
#define BGE_RX_BD_RULES_MASKVAL11     0x04DC
#define BGE_RX_BD_RULES_CTL12         0x04E0
#define BGE_RX_BD_RULES_MASKVAL12     0x04E4
#define BGE_RX_BD_RULES_CTL13         0x04E8
#define BGE_RX_BD_RULES_MASKVAL13     0x04EC
#define BGE_RX_BD_RULES_CTL14         0x04F0
#define BGE_RX_BD_RULES_MASKVAL14     0x04F4
#define BGE_RX_BD_RULES_CTL15         0x04F8
#define BGE_RX_BD_RULES_MASKVAL15     0x04FC
#define BGE_RX_RULES_CFG              0x0500
#define BGE_SERDES_CFG                0x0590
#define BGE_SERDES_STS                0x0594
#define BGE_SGDIG_CFG                 0x05B0
#define BGE_SGDIG_STS                 0x05B4
#define BGE_MAC_STATS                 0x0800

/* Ethernet MAC Mode register */
#define BGE_MACMODE_RESET               0x00000001
#define BGE_MACMODE_HALF_DUPLEX         0x00000002
#define BGE_MACMODE_PORTMODE            0x0000000C
#define BGE_MACMODE_LOOPBACK            0x00000010
#define BGE_MACMODE_RX_TAGGEDPKT        0x00000080
#define BGE_MACMODE_TX_BURST_ENB        0x00000100
#define BGE_MACMODE_MAX_DEFER           0x00000200
#define BGE_MACMODE_LINK_POLARITY       0x00000400
#define BGE_MACMODE_RX_STATS_ENB        0x00000800
#define BGE_MACMODE_RX_STATS_CLEAR      0x00001000
#define BGE_MACMODE_RX_STATS_FLUSH      0x00002000
#define BGE_MACMODE_TX_STATS_ENB        0x00004000
#define BGE_MACMODE_TX_STATS_CLEAR      0x00008000
#define BGE_MACMODE_TX_STATS_FLUSH      0x00010000
#define BGE_MACMODE_TBI_SEND_CFGS       0x00020000
#define BGE_MACMODE_MAGIC_PKT_ENB       0x00040000
#define BGE_MACMODE_ACPI_PWRON_ENB      0x00080000
#define BGE_MACMODE_MIP_ENB             0x00100000
#define BGE_MACMODE_TXDMA_ENB           0x00200000
#define BGE_MACMODE_RXDMA_ENB           0x00400000
#define BGE_MACMODE_FRMHDR_DMA_ENB      0x00800000

#define BGE_PORTMODE_NONE         0x00000000
#define BGE_PORTMODE_MII          0x00000004
#define BGE_PORTMODE_GMII         0x00000008
#define BGE_PORTMODE_TBI          0x0000000C

/* MAC Status register */
#define BGE_MACSTAT_TBI_PCS_SYNCHED       0x00000001
#define BGE_MACSTAT_TBI_SIGNAL_DETECT     0x00000002
#define BGE_MACSTAT_RX_CFG                0x00000004
#define BGE_MACSTAT_CFG_CHANGED           0x00000008
#define BGE_MACSTAT_SYNC_CHANGED          0x00000010
#define BGE_MACSTAT_PORT_DECODE_ERROR     0x00000400
#define BGE_MACSTAT_LINK_CHANGED          0x00001000
#define BGE_MACSTAT_MI_COMPLETE           0x00400000
#define BGE_MACSTAT_MI_INTERRUPT          0x00800000
#define BGE_MACSTAT_AUTOPOLL_ERROR        0x01000000
#define BGE_MACSTAT_ODI_ERROR             0x02000000
#define BGE_MACSTAT_RXSTAT_OFLOW          0x04000000
#define BGE_MACSTAT_TXSTAT_OFLOW          0x08000000

/* MAC Event Enable Register */
#define BGE_EVTENB_PORT_DECODE_ERROR     0x00000400
#define BGE_EVTENB_LINK_CHANGED          0x00001000
#define BGE_EVTENB_MI_COMPLETE           0x00400000
#define BGE_EVTENB_MI_INTERRUPT          0x00800000
#define BGE_EVTENB_AUTOPOLL_ERROR        0x01000000
#define BGE_EVTENB_ODI_ERROR             0x02000000
#define BGE_EVTENB_RXSTAT_OFLOW          0x04000000
#define BGE_EVTENB_TXSTAT_OFLOW          0x08000000

/* LED Control Register */
#define BGE_LEDCTL_LINKLED_OVERRIDE         0x00000001
#define BGE_LEDCTL_1000MBPS_LED             0x00000002
#define BGE_LEDCTL_100MBPS_LED              0x00000004
#define BGE_LEDCTL_10MBPS_LED               0x00000008
#define BGE_LEDCTL_TRAFLED_OVERRIDE         0x00000010
#define BGE_LEDCTL_TRAFLED_BLINK            0x00000020
#define BGE_LEDCTL_TREFLED_BLINK_2          0x00000040
#define BGE_LEDCTL_1000MBPS_STS             0x00000080
#define BGE_LEDCTL_100MBPS_STS              0x00000100
#define BGE_LEDCTL_10MBPS_STS               0x00000200
#define BGE_LEDCTL_TRADLED_STS              0x00000400
#define BGE_LEDCTL_BLINKPERIOD              0x7FF80000
#define BGE_LEDCTL_BLINKPERIOD_OVERRIDE     0x80000000

/* TX backoff seed register */
#define BGE_TX_BACKOFF_SEED_MASK    0x3FF

/* Autopoll status register */
#define BGE_AUTOPOLLSTS_ERROR        0x00000001

/* Transmit MAC mode register */
#define BGE_TXMODE_RESET                 0x00000001
#define BGE_TXMODE_ENABLE                0x00000002
#define BGE_TXMODE_FLOWCTL_ENABLE        0x00000010
#define BGE_TXMODE_BIGBACKOFF_ENABLE     0x00000020
#define BGE_TXMODE_LONGPAUSE_ENABLE      0x00000040

/* Transmit MAC status register */
#define BGE_TXSTAT_RX_XOFFED         0x00000001
#define BGE_TXSTAT_SENT_XOFF         0x00000002
#define BGE_TXSTAT_SENT_XON          0x00000004
#define BGE_TXSTAT_LINK_UP           0x00000008
#define BGE_TXSTAT_ODI_UFLOW         0x00000010
#define BGE_TXSTAT_ODI_OFLOW         0x00000020

/* Transmit MAC lengths register */
#define BGE_TXLEN_SLOTTIME         0x000000FF
#define BGE_TXLEN_IPG              0x00000F00
#define BGE_TXLEN_CRS              0x00003000

/* Receive MAC mode register */
#define BGE_RXMODE_RESET                 0x00000001
#define BGE_RXMODE_ENABLE                0x00000002
#define BGE_RXMODE_FLOWCTL_ENABLE        0x00000004
#define BGE_RXMODE_RX_GIANTS             0x00000020
#define BGE_RXMODE_RX_RUNTS              0x00000040
#define BGE_RXMODE_8022_LENCHECK         0x00000080
#define BGE_RXMODE_RX_PROMISC            0x00000100
#define BGE_RXMODE_RX_NO_CRC_CHECK       0x00000200
#define BGE_RXMODE_RX_KEEP_VLAN_DIAG     0x00000400

/* Receive MAC status register */
#define BGE_RXSTAT_REMOTE_XOFFED     0x00000001
#define BGE_RXSTAT_RCVD_XOFF         0x00000002
#define BGE_RXSTAT_RCVD_XON          0x00000004

/* Receive Rules Control register */
#define BGE_RXRULECTL_OFFSET             0x000000FF
#define BGE_RXRULECTL_CLASS              0x00001F00
#define BGE_RXRULECTL_HDRTYPE            0x0000E000
#define BGE_RXRULECTL_COMPARE_OP         0x00030000
#define BGE_RXRULECTL_MAP                0x01000000
#define BGE_RXRULECTL_DISCARD            0x02000000
#define BGE_RXRULECTL_MASK               0x04000000
#define BGE_RXRULECTL_ACTIVATE_PROC3     0x08000000
#define BGE_RXRULECTL_ACTIVATE_PROC2     0x10000000
#define BGE_RXRULECTL_ACTIVATE_PROC1     0x20000000
#define BGE_RXRULECTL_ANDWITHNEXT        0x40000000

/* Receive Rules Mask register */
#define BGE_RXRULEMASK_VALUE        0x0000FFFF
#define BGE_RXRULEMASK_MASKVAL      0xFFFF0000

/* SERDES configuration register */
#define BGE_SERDESCFG_RXR              0x00000007 /* phase interpolator */
#define BGE_SERDESCFG_RXG              0x00000018 /* rx gain setting */
#define BGE_SERDESCFG_RXEDGESEL        0x00000040 /* rising/falling egde */
#define BGE_SERDESCFG_TX_BIAS          0x00000380 /* TXDAC bias setting */
#define BGE_SERDESCFG_IBMAX            0x00000400 /* bias current +25% */
#define BGE_SERDESCFG_IBMIN            0x00000800 /* bias current -25% */
#define BGE_SERDESCFG_TXMODE           0x00001000
#define BGE_SERDESCFG_TXEDGESEL        0x00002000 /* rising/falling edge */
#define BGE_SERDESCFG_MODE             0x00004000 /* TXCP/TXCN disabled */
#define BGE_SERDESCFG_PLLTEST          0x00008000 /* PLL test mode */
#define BGE_SERDESCFG_CDET             0x00010000 /* comma detect enable */
#define BGE_SERDESCFG_TBILOOP          0x00020000 /* local loopback */
#define BGE_SERDESCFG_REMLOOP          0x00040000 /* remote loopback */
#define BGE_SERDESCFG_INVPHASE         0x00080000 /* Reverse 125Mhz clock */
#define BGE_SERDESCFG_12REGCTL         0x00300000 /* 1.2v regulator ctl */
#define BGE_SERDESCFG_REGCTL           0x00C00000 /* regulator ctl (2.5v) */

/* SERDES status register */
#define BGE_SERDESSTS_RXSTAT        0x0000000F /* receive status bits */
#define BGE_SERDESSTS_CDET          0x00000010 /* comma code detected */

/* SGDIG config (not documented) */
#define BGE_SGDIGCFG_PAUSE_CAP          0x00000800
#define BGE_SGDIGCFG_ASYM_PAUSE         0x00001000
#define BGE_SGDIGCFG_SEND               0x40000000
#define BGE_SGDIGCFG_AUTO               0x80000000

/* SGDIG status (not documented) */
#define BGE_SGDIGSTS_PAUSE_CAP          0x00080000
#define BGE_SGDIGSTS_ASYM_PAUSE         0x00100000
#define BGE_SGDIGSTS_DONE               0x00000002


/* MI communication register */
#define BGE_MICOMM_DATA             0x0000FFFF
#define BGE_MICOMM_REG              0x001F0000
#define BGE_MICOMM_PHY              0x03E00000
#define BGE_MICOMM_CMD              0x0C000000
#define BGE_MICOMM_READFAIL         0x10000000
#define BGE_MICOMM_BUSY             0x20000000

#define BGE_MIREG(x)    ((x & 0x1F) << 16)
#define BGE_MIPHY(x)    ((x & 0x1F) << 21)
#define BGE_MICMD_WRITE            0x04000000
#define BGE_MICMD_READ             0x08000000

/* MI status register */
#define BGE_MISTS_LINK               0x00000001
#define BGE_MISTS_10MBPS             0x00000002

#define BGE_MIMODE_SHORTPREAMBLE     0x00000002
#define BGE_MIMODE_AUTOPOLL          0x00000010
#define BGE_MIMODE_CLKCNT            0x001F0000


/*
 * Send data initiator control registers.
 */
#define BGE_SDI_MODE                        0x0C00
#define BGE_SDI_STATUS                      0x0C04
#define BGE_SDI_STATS_CTL                   0x0C08
#define BGE_SDI_STATS_ENABLE_MASK           0x0C0C
#define BGE_SDI_STATS_INCREMENT_MASK        0x0C10
#define BGE_LOCSTATS_COS0                   0x0C80
#define BGE_LOCSTATS_COS1                   0x0C84
#define BGE_LOCSTATS_COS2                   0x0C88
#define BGE_LOCSTATS_COS3                   0x0C8C
#define BGE_LOCSTATS_COS4                   0x0C90
#define BGE_LOCSTATS_COS5                   0x0C84
#define BGE_LOCSTATS_COS6                   0x0C98
#define BGE_LOCSTATS_COS7                   0x0C9C
#define BGE_LOCSTATS_COS8                   0x0CA0
#define BGE_LOCSTATS_COS9                   0x0CA4
#define BGE_LOCSTATS_COS10                  0x0CA8
#define BGE_LOCSTATS_COS11                  0x0CAC
#define BGE_LOCSTATS_COS12                  0x0CB0
#define BGE_LOCSTATS_COS13                  0x0CB4
#define BGE_LOCSTATS_COS14                  0x0CB8
#define BGE_LOCSTATS_COS15                  0x0CBC
#define BGE_LOCSTATS_DMA_RQ_FULL            0x0CC0
#define BGE_LOCSTATS_DMA_HIPRIO_RQ_FULL     0x0CC4
#define BGE_LOCSTATS_SDC_QUEUE_FULL         0x0CC8
#define BGE_LOCSTATS_NIC_SENDPROD_SET       0x0CCC
#define BGE_LOCSTATS_STATS_UPDATED          0x0CD0
#define BGE_LOCSTATS_IRQS                   0x0CD4
#define BGE_LOCSTATS_AVOIDED_IRQS           0x0CD8
#define BGE_LOCSTATS_TX_THRESH_HIT          0x0CDC

/* Send Data Initiator mode register */
#define BGE_SDIMODE_RESET                0x00000001
#define BGE_SDIMODE_ENABLE               0x00000002
#define BGE_SDIMODE_STATS_OFLOW_ATTN     0x00000004

/* Send Data Initiator stats register */
#define BGE_SDISTAT_STATS_OFLOW_ATTN    0x00000004

/* Send Data Initiator stats control register */
#define BGE_SDISTATSCTL_ENABLE         0x00000001
#define BGE_SDISTATSCTL_FASTER         0x00000002
#define BGE_SDISTATSCTL_CLEAR          0x00000004
#define BGE_SDISTATSCTL_FORCEFLUSH     0x00000008
#define BGE_SDISTATSCTL_FORCEZERO      0x00000010

/*
 * Send Data Completion Control registers
 */
#define BGE_SDC_MODE            0x1000
#define BGE_SDC_STATUS          0x1004

/* Send Data completion mode register */
#define BGE_SDCMODE_RESET          0x00000001
#define BGE_SDCMODE_ENABLE         0x00000002
#define BGE_SDCMODE_ATTN           0x00000004
#define BGE_SDCMODE_CDELAY         0x00000010

/* Send Data completion status register */
#define BGE_SDCSTAT_ATTN        0x00000004

/*
 * Send BD Ring Selector Control registers
 */
#define BGE_SRS_MODE                   0x1400
#define BGE_SRS_STATUS                 0x1404
#define BGE_SRS_HWDIAG                 0x1408
#define BGE_SRS_LOC_NIC_CONS0          0x1440
#define BGE_SRS_LOC_NIC_CONS1          0x1444
#define BGE_SRS_LOC_NIC_CONS2          0x1448
#define BGE_SRS_LOC_NIC_CONS3          0x144C
#define BGE_SRS_LOC_NIC_CONS4          0x1450
#define BGE_SRS_LOC_NIC_CONS5          0x1454
#define BGE_SRS_LOC_NIC_CONS6          0x1458
#define BGE_SRS_LOC_NIC_CONS7          0x145C
#define BGE_SRS_LOC_NIC_CONS8          0x1460
#define BGE_SRS_LOC_NIC_CONS9          0x1464
#define BGE_SRS_LOC_NIC_CONS10         0x1468
#define BGE_SRS_LOC_NIC_CONS11         0x146C
#define BGE_SRS_LOC_NIC_CONS12         0x1470
#define BGE_SRS_LOC_NIC_CONS13         0x1474
#define BGE_SRS_LOC_NIC_CONS14         0x1478
#define BGE_SRS_LOC_NIC_CONS15         0x147C

/* Send BD Ring Selector Mode register */
#define BGE_SRSMODE_RESET          0x00000001
#define BGE_SRSMODE_ENABLE         0x00000002
#define BGE_SRSMODE_ATTN           0x00000004

/* Send BD Ring Selector Status register */
#define BGE_SRSSTAT_ERROR        0x00000004

/* Send BD Ring Selector HW Diagnostics register */
#define BGE_SRSHWDIAG_STATE              0x0000000F
#define BGE_SRSHWDIAG_CURRINGNUM         0x000000F0
#define BGE_SRSHWDIAG_STAGEDRINGNUM      0x00000F00
#define BGE_SRSHWDIAG_RINGNUM_IN_MBX     0x0000F000

/*
 * Send BD Initiator Selector Control registers
 */
#define BGE_SBDI_MODE                   0x1800
#define BGE_SBDI_STATUS                 0x1804
#define BGE_SBDI_LOC_NIC_PROD0          0x1808
#define BGE_SBDI_LOC_NIC_PROD1          0x180C
#define BGE_SBDI_LOC_NIC_PROD2          0x1810
#define BGE_SBDI_LOC_NIC_PROD3          0x1814
#define BGE_SBDI_LOC_NIC_PROD4          0x1818
#define BGE_SBDI_LOC_NIC_PROD5          0x181C
#define BGE_SBDI_LOC_NIC_PROD6          0x1820
#define BGE_SBDI_LOC_NIC_PROD7          0x1824
#define BGE_SBDI_LOC_NIC_PROD8          0x1828
#define BGE_SBDI_LOC_NIC_PROD9          0x182C
#define BGE_SBDI_LOC_NIC_PROD10         0x1830
#define BGE_SBDI_LOC_NIC_PROD11         0x1834
#define BGE_SBDI_LOC_NIC_PROD12         0x1838
#define BGE_SBDI_LOC_NIC_PROD13         0x183C
#define BGE_SBDI_LOC_NIC_PROD14         0x1840
#define BGE_SBDI_LOC_NIC_PROD15         0x1844

/* Send BD Initiator Mode register */
#define BGE_SBDIMODE_RESET          0x00000001
#define BGE_SBDIMODE_ENABLE         0x00000002
#define BGE_SBDIMODE_ATTN           0x00000004

/* Send BD Initiator Status register */
#define BGE_SBDISTAT_ERROR        0x00000004

/*
 * Send BD Completion Control registers
 */
#define BGE_SBDC_MODE               0x1C00
#define BGE_SBDC_STATUS             0x1C04

/* Send BD Completion Control Mode register */
#define BGE_SBDCMODE_RESET          0x00000001
#define BGE_SBDCMODE_ENABLE         0x00000002
#define BGE_SBDCMODE_ATTN           0x00000004

/* Send BD Completion Control Status register */
#define BGE_SBDCSTAT_ATTN        0x00000004

/*
 * Receive List Placement Control registers
 */
#define BGE_RXLP_MODE                       0x2000
#define BGE_RXLP_STATUS                     0x2004
#define BGE_RXLP_SEL_LIST_LOCK              0x2008
#define BGE_RXLP_SEL_NON_EMPTY_BITS         0x200C
#define BGE_RXLP_CFG                        0x2010
#define BGE_RXLP_STATS_CTL                  0x2014
#define BGE_RXLP_STATS_ENABLE_MASK          0x2018
#define BGE_RXLP_STATS_INCREMENT_MASK       0x201C
#define BGE_RXLP_HEAD0                      0x2100
#define BGE_RXLP_TAIL0                      0x2104
#define BGE_RXLP_COUNT0                     0x2108
#define BGE_RXLP_HEAD1                      0x2110
#define BGE_RXLP_TAIL1                      0x2114
#define BGE_RXLP_COUNT1                     0x2118
#define BGE_RXLP_HEAD2                      0x2120
#define BGE_RXLP_TAIL2                      0x2124
#define BGE_RXLP_COUNT2                     0x2128
#define BGE_RXLP_HEAD3                      0x2130
#define BGE_RXLP_TAIL3                      0x2134
#define BGE_RXLP_COUNT3                     0x2138
#define BGE_RXLP_HEAD4                      0x2140
#define BGE_RXLP_TAIL4                      0x2144
#define BGE_RXLP_COUNT4                     0x2148
#define BGE_RXLP_HEAD5                      0x2150
#define BGE_RXLP_TAIL5                      0x2154
#define BGE_RXLP_COUNT5                     0x2158
#define BGE_RXLP_HEAD6                      0x2160
#define BGE_RXLP_TAIL6                      0x2164
#define BGE_RXLP_COUNT6                     0x2168
#define BGE_RXLP_HEAD7                      0x2170
#define BGE_RXLP_TAIL7                      0x2174
#define BGE_RXLP_COUNT7                     0x2178
#define BGE_RXLP_HEAD8                      0x2180
#define BGE_RXLP_TAIL8                      0x2184
#define BGE_RXLP_COUNT8                     0x2188
#define BGE_RXLP_HEAD9                      0x2190
#define BGE_RXLP_TAIL9                      0x2194
#define BGE_RXLP_COUNT9                     0x2198
#define BGE_RXLP_HEAD10                     0x21A0
#define BGE_RXLP_TAIL10                     0x21A4
#define BGE_RXLP_COUNT10                    0x21A8
#define BGE_RXLP_HEAD11                     0x21B0
#define BGE_RXLP_TAIL11                     0x21B4
#define BGE_RXLP_COUNT11                    0x21B8
#define BGE_RXLP_HEAD12                     0x21C0
#define BGE_RXLP_TAIL12                     0x21C4
#define BGE_RXLP_COUNT12                    0x21C8
#define BGE_RXLP_HEAD13                     0x21D0
#define BGE_RXLP_TAIL13                     0x21D4
#define BGE_RXLP_COUNT13                    0x21D8
#define BGE_RXLP_HEAD14                     0x21E0
#define BGE_RXLP_TAIL14                     0x21E4
#define BGE_RXLP_COUNT14                    0x21E8
#define BGE_RXLP_HEAD15                     0x21F0
#define BGE_RXLP_TAIL15                     0x21F4
#define BGE_RXLP_COUNT15                    0x21F8
#define BGE_RXLP_LOCSTAT_COS0               0x2200
#define BGE_RXLP_LOCSTAT_COS1               0x2204
#define BGE_RXLP_LOCSTAT_COS2               0x2208
#define BGE_RXLP_LOCSTAT_COS3               0x220C
#define BGE_RXLP_LOCSTAT_COS4               0x2210
#define BGE_RXLP_LOCSTAT_COS5               0x2214
#define BGE_RXLP_LOCSTAT_COS6               0x2218
#define BGE_RXLP_LOCSTAT_COS7               0x221C
#define BGE_RXLP_LOCSTAT_COS8               0x2220
#define BGE_RXLP_LOCSTAT_COS9               0x2224
#define BGE_RXLP_LOCSTAT_COS10              0x2228
#define BGE_RXLP_LOCSTAT_COS11              0x222C
#define BGE_RXLP_LOCSTAT_COS12              0x2230
#define BGE_RXLP_LOCSTAT_COS13              0x2234
#define BGE_RXLP_LOCSTAT_COS14              0x2238
#define BGE_RXLP_LOCSTAT_COS15              0x223C
#define BGE_RXLP_LOCSTAT_FILTDROP           0x2240
#define BGE_RXLP_LOCSTAT_DMA_WRQ_FULL       0x2244
#define BGE_RXLP_LOCSTAT_DMA_HPWRQ_FULL     0x2248
#define BGE_RXLP_LOCSTAT_OUT_OF_BDS         0x224C
#define BGE_RXLP_LOCSTAT_IFIN_DROPS         0x2250
#define BGE_RXLP_LOCSTAT_IFIN_ERRORS        0x2254
#define BGE_RXLP_LOCSTAT_RXTHRESH_HIT       0x2258


/* Receive List Placement mode register */
#define BGE_RXLPMODE_RESET                0x00000001
#define BGE_RXLPMODE_ENABLE               0x00000002
#define BGE_RXLPMODE_CLASS0_ATTN          0x00000004
#define BGE_RXLPMODE_MAPOUTRANGE_ATTN     0x00000008
#define BGE_RXLPMODE_STATSOFLOW_ATTN      0x00000010

/* Receive List Placement Status register */
#define BGE_RXLPSTAT_CLASS0_ATTN          0x00000004
#define BGE_RXLPSTAT_MAPOUTRANGE_ATTN     0x00000008
#define BGE_RXLPSTAT_STATSOFLOW_ATTN      0x00000010

/*
 * Receive Data and Receive BD Initiator Control Registers
 */
#define BGE_RDBDI_MODE                    0x2400
#define BGE_RDBDI_STATUS                  0x2404
#define BGE_RX_JUMBO_RCB_HADDR_HI         0x2440
#define BGE_RX_JUMBO_RCB_HADDR_LO         0x2444
#define BGE_RX_JUMBO_RCB_MAXLEN_FLAGS     0x2448
#define BGE_RX_JUMBO_RCB_NICADDR          0x244C
#define BGE_RX_STD_RCB_HADDR_HI           0x2450
#define BGE_RX_STD_RCB_HADDR_LO           0x2454
#define BGE_RX_STD_RCB_MAXLEN_FLAGS       0x2458
#define BGE_RX_STD_RCB_NICADDR            0x245C
#define BGE_RX_MINI_RCB_HADDR_HI          0x2460
#define BGE_RX_MINI_RCB_HADDR_LO          0x2464
#define BGE_RX_MINI_RCB_MAXLEN_FLAGS      0x2468
#define BGE_RX_MINI_RCB_NICADDR           0x246C
#define BGE_RDBDI_JUMBO_RX_CONS           0x2470
#define BGE_RDBDI_STD_RX_CONS             0x2474
#define BGE_RDBDI_MINI_RX_CONS            0x2478
#define BGE_RDBDI_RETURN_PROD0            0x2480
#define BGE_RDBDI_RETURN_PROD1            0x2484
#define BGE_RDBDI_RETURN_PROD2            0x2488
#define BGE_RDBDI_RETURN_PROD3            0x248C
#define BGE_RDBDI_RETURN_PROD4            0x2490
#define BGE_RDBDI_RETURN_PROD5            0x2494
#define BGE_RDBDI_RETURN_PROD6            0x2498
#define BGE_RDBDI_RETURN_PROD7            0x249C
#define BGE_RDBDI_RETURN_PROD8            0x24A0
#define BGE_RDBDI_RETURN_PROD9            0x24A4
#define BGE_RDBDI_RETURN_PROD10           0x24A8
#define BGE_RDBDI_RETURN_PROD11           0x24AC
#define BGE_RDBDI_RETURN_PROD12           0x24B0
#define BGE_RDBDI_RETURN_PROD13           0x24B4
#define BGE_RDBDI_RETURN_PROD14           0x24B8
#define BGE_RDBDI_RETURN_PROD15           0x24BC
#define BGE_RDBDI_HWDIAG                  0x24C0


/* Receive Data and Receive BD Initiator Mode register */
#define BGE_RDBDIMODE_RESET              0x00000001
#define BGE_RDBDIMODE_ENABLE             0x00000002
#define BGE_RDBDIMODE_JUMBO_ATTN         0x00000004
#define BGE_RDBDIMODE_GIANT_ATTN         0x00000008
#define BGE_RDBDIMODE_BADRINGSZ_ATTN     0x00000010

/* Receive Data and Receive BD Initiator Status register */
#define BGE_RDBDISTAT_JUMBO_ATTN         0x00000004
#define BGE_RDBDISTAT_GIANT_ATTN         0x00000008
#define BGE_RDBDISTAT_BADRINGSZ_ATTN     0x00000010


/*
 * Receive Data Completion Control registers
 */
#define BGE_RDC_MODE            0x2800

/* Receive Data Completion Mode register */
#define BGE_RDCMODE_RESET          0x00000001
#define BGE_RDCMODE_ENABLE         0x00000002
#define BGE_RDCMODE_ATTN           0x00000004

/*
 * Receive BD Initiator Control registers
 */
#define BGE_RBDI_MODE                  0x2C00
#define BGE_RBDI_STATUS                0x2C04
#define BGE_RBDI_NIC_JUMBO_BD_PROD     0x2C08
#define BGE_RBDI_NIC_STD_BD_PROD       0x2C0C
#define BGE_RBDI_NIC_MINI_BD_PROD      0x2C10
#define BGE_RBDI_MINI_REPL_THRESH      0x2C14
#define BGE_RBDI_STD_REPL_THRESH       0x2C18
#define BGE_RBDI_JUMBO_REPL_THRESH     0x2C1C

/* Receive BD Initiator Mode register */
#define BGE_RBDIMODE_RESET          0x00000001
#define BGE_RBDIMODE_ENABLE         0x00000002
#define BGE_RBDIMODE_ATTN           0x00000004

/* Receive BD Initiator Status register */
#define BGE_RBDISTAT_ATTN        0x00000004

/*
 * Receive BD Completion Control registers
 */
#define BGE_RBDC_MODE                  0x3000
#define BGE_RBDC_STATUS                0x3004
#define BGE_RBDC_JUMBO_BD_PROD         0x3008
#define BGE_RBDC_STD_BD_PROD           0x300C
#define BGE_RBDC_MINI_BD_PROD          0x3010

/* Receive BD completion mode register */
#define BGE_RBDCMODE_RESET         0x00000001
#define BGE_RBDCMODE_ENABLE        0x00000002
#define BGE_RBDCMODE_ATTN          0x00000004

/* Receive BD completion status register */
#define BGE_RBDCSTAT_ERROR        0x00000004

/*
 * Receive List Selector Control registers
 */
#define BGE_RXLS_MODE              0x3400
#define BGE_RXLS_STATUS            0x3404

/* Receive List Selector Mode register */
#define BGE_RXLSMODE_RESET        0x00000001
#define BGE_RXLSMODE_ENABLE       0x00000002
#define BGE_RXLSMODE_ATTN         0x00000004

/* Receive List Selector Status register */
#define BGE_RXLSSTAT_ERROR        0x00000004

/*
 * Mbuf Cluster Free registers (has nothing to do with BSD mbufs)
 */
#define BGE_MBCF_MODE              0x3800
#define BGE_MBCF_STATUS            0x3804

/* Mbuf Cluster Free mode register */
#define BGE_MBCFMODE_RESET        0x00000001
#define BGE_MBCFMODE_ENABLE       0x00000002
#define BGE_MBCFMODE_ATTN         0x00000004

/* Mbuf Cluster Free status register */
#define BGE_MBCFSTAT_ERROR        0x00000004

/*
 * Host Coalescing Control registers
 */
#define BGE_HCC_MODE                    0x3C00
#define BGE_HCC_STATUS                  0x3C04
#define BGE_HCC_RX_COAL_TICKS           0x3C08
#define BGE_HCC_TX_COAL_TICKS           0x3C0C
#define BGE_HCC_RX_MAX_COAL_BDS         0x3C10
#define BGE_HCC_TX_MAX_COAL_BDS         0x3C14
#define BGE_HCC_RX_COAL_TICKS_INT       0x3C18
#define BGE_HCC_TX_COAL_TICKS_INT       0x3C1C
#define BGE_HCC_RX_MAX_COAL_BDS_INT     0x3C20
#define BGE_HCC_TX_MAX_COAL_BDS_INT     0x3C24
#define BGE_HCC_STATS_TICKS             0x3C28
#define BGE_HCC_STATS_ADDR_HI           0x3C30
#define BGE_HCC_STATS_ADDR_LO           0x3C34
#define BGE_HCC_STATUSBLK_ADDR_HI       0x3C38
#define BGE_HCC_STATUSBLK_ADDR_LO       0x3C3C
#define BGE_HCC_STATS_BASEADDR          0x3C40
#define BGE_HCC_STATUSBLK_BASEADDR      0x3C44
#define BGE_FLOW_ATTN                   0x3C48
#define BGE_HCC_JUMBO_BD_CONS           0x3C50
#define BGE_HCC_STD_BD_CONS             0x3C54
#define BGE_HCC_MINI_BD_CONS            0x3C58
#define BGE_HCC_RX_RETURN_PROD0         0x3C80
#define BGE_HCC_RX_RETURN_PROD1         0x3C84
#define BGE_HCC_RX_RETURN_PROD2         0x3C88
#define BGE_HCC_RX_RETURN_PROD3         0x3C8C
#define BGE_HCC_RX_RETURN_PROD4         0x3C90
#define BGE_HCC_RX_RETURN_PROD5         0x3C94
#define BGE_HCC_RX_RETURN_PROD6         0x3C98
#define BGE_HCC_RX_RETURN_PROD7         0x3C9C
#define BGE_HCC_RX_RETURN_PROD8         0x3CA0
#define BGE_HCC_RX_RETURN_PROD9         0x3CA4
#define BGE_HCC_RX_RETURN_PROD10        0x3CA8
#define BGE_HCC_RX_RETURN_PROD11        0x3CAC
#define BGE_HCC_RX_RETURN_PROD12        0x3CB0
#define BGE_HCC_RX_RETURN_PROD13        0x3CB4
#define BGE_HCC_RX_RETURN_PROD14        0x3CB8
#define BGE_HCC_RX_RETURN_PROD15        0x3CBC
#define BGE_HCC_TX_BD_CONS0             0x3CC0
#define BGE_HCC_TX_BD_CONS1             0x3CC4
#define BGE_HCC_TX_BD_CONS2             0x3CC8
#define BGE_HCC_TX_BD_CONS3             0x3CCC
#define BGE_HCC_TX_BD_CONS4             0x3CD0
#define BGE_HCC_TX_BD_CONS5             0x3CD4
#define BGE_HCC_TX_BD_CONS6             0x3CD8
#define BGE_HCC_TX_BD_CONS7             0x3CDC
#define BGE_HCC_TX_BD_CONS8             0x3CE0
#define BGE_HCC_TX_BD_CONS9             0x3CE4
#define BGE_HCC_TX_BD_CONS10            0x3CE8
#define BGE_HCC_TX_BD_CONS11            0x3CEC
#define BGE_HCC_TX_BD_CONS12            0x3CF0
#define BGE_HCC_TX_BD_CONS13            0x3CF4
#define BGE_HCC_TX_BD_CONS14            0x3CF8
#define BGE_HCC_TX_BD_CONS15            0x3CFC

/* Host coalescing mode register */
#define BGE_HCCMODE_RESET            0x00000001
#define BGE_HCCMODE_ENABLE           0x00000002
#define BGE_HCCMODE_ATTN             0x00000004
#define BGE_HCCMODE_COAL_NOW         0x00000008
#define BGE_HCCMODE_MSI_BITS         0x00000070
#define BGE_HCCMODE_STATBLK_SIZE     0x00000180

#define BGE_STATBLKSZ_FULL           0x00000000
#define BGE_STATBLKSZ_64BYTE         0x00000080
#define BGE_STATBLKSZ_32BYTE         0x00000100

/* Host coalescing status register */
#define BGE_HCCSTAT_ERROR        0x00000004

/* Flow attention register */
#define BGE_FLOWATTN_MB_LOWAT            0x00000040
#define BGE_FLOWATTN_MEMARB              0x00000080
#define BGE_FLOWATTN_HOSTCOAL            0x00008000
#define BGE_FLOWATTN_DMADONE_DISCARD     0x00010000
#define BGE_FLOWATTN_RCB_INVAL           0x00020000
#define BGE_FLOWATTN_RXDATA_CORRUPT      0x00040000
#define BGE_FLOWATTN_RDBDI               0x00080000
#define BGE_FLOWATTN_RXLS                0x00100000
#define BGE_FLOWATTN_RXLP                0x00200000
#define BGE_FLOWATTN_RBDC                0x00400000
#define BGE_FLOWATTN_RBDI                0x00800000
#define BGE_FLOWATTN_SDC                 0x08000000
#define BGE_FLOWATTN_SDI                 0x10000000
#define BGE_FLOWATTN_SRS                 0x20000000
#define BGE_FLOWATTN_SBDC                0x40000000
#define BGE_FLOWATTN_SBDI                0x80000000

/*
 * Memory arbiter registers
 */
#define BGE_MARB_MODE               0x4000
#define BGE_MARB_STATUS             0x4004
#define BGE_MARB_TRAPADDR_HI        0x4008
#define BGE_MARB_TRAPADDR_LO        0x400C

/* Memory arbiter mode register */
#define BGE_MARBMODE_RESET                  0x00000001
#define BGE_MARBMODE_ENABLE                 0x00000002
#define BGE_MARBMODE_TX_ADDR_TRAP           0x00000004
#define BGE_MARBMODE_RX_ADDR_TRAP           0x00000008
#define BGE_MARBMODE_DMAW1_TRAP             0x00000010
#define BGE_MARBMODE_DMAR1_TRAP             0x00000020
#define BGE_MARBMODE_RXRISC_TRAP            0x00000040
#define BGE_MARBMODE_TXRISC_TRAP            0x00000080
#define BGE_MARBMODE_PCI_TRAP               0x00000100
#define BGE_MARBMODE_DMAR2_TRAP             0x00000200
#define BGE_MARBMODE_RXQ_TRAP               0x00000400
#define BGE_MARBMODE_RXDI1_TRAP             0x00000800
#define BGE_MARBMODE_RXDI2_TRAP             0x00001000
#define BGE_MARBMODE_DC_GRPMEM_TRAP         0x00002000
#define BGE_MARBMODE_HCOAL_TRAP             0x00004000
#define BGE_MARBMODE_MBUF_TRAP              0x00008000
#define BGE_MARBMODE_TXDI_TRAP              0x00010000
#define BGE_MARBMODE_SDC_DMAC_TRAP          0x00020000
#define BGE_MARBMODE_TXBD_TRAP              0x00040000
#define BGE_MARBMODE_BUFFMAN_TRAP           0x00080000
#define BGE_MARBMODE_DMAW2_TRAP             0x00100000
#define BGE_MARBMODE_XTSSRAM_ROFLO_TRAP     0x00200000
#define BGE_MARBMODE_XTSSRAM_RUFLO_TRAP     0x00400000
#define BGE_MARBMODE_XTSSRAM_WOFLO_TRAP     0x00800000
#define BGE_MARBMODE_XTSSRAM_WUFLO_TRAP     0x01000000
#define BGE_MARBMODE_XTSSRAM_PERR_TRAP      0x02000000

/* Memory arbiter status register */
#define BGE_MARBSTAT_TX_ADDR_TRAP           0x00000004
#define BGE_MARBSTAT_RX_ADDR_TRAP           0x00000008
#define BGE_MARBSTAT_DMAW1_TRAP             0x00000010
#define BGE_MARBSTAT_DMAR1_TRAP             0x00000020
#define BGE_MARBSTAT_RXRISC_TRAP            0x00000040
#define BGE_MARBSTAT_TXRISC_TRAP            0x00000080
#define BGE_MARBSTAT_PCI_TRAP               0x00000100
#define BGE_MARBSTAT_DMAR2_TRAP             0x00000200
#define BGE_MARBSTAT_RXQ_TRAP               0x00000400
#define BGE_MARBSTAT_RXDI1_TRAP             0x00000800
#define BGE_MARBSTAT_RXDI2_TRAP             0x00001000
#define BGE_MARBSTAT_DC_GRPMEM_TRAP         0x00002000
#define BGE_MARBSTAT_HCOAL_TRAP             0x00004000
#define BGE_MARBSTAT_MBUF_TRAP              0x00008000
#define BGE_MARBSTAT_TXDI_TRAP              0x00010000
#define BGE_MARBSTAT_SDC_DMAC_TRAP          0x00020000
#define BGE_MARBSTAT_TXBD_TRAP              0x00040000
#define BGE_MARBSTAT_BUFFMAN_TRAP           0x00080000
#define BGE_MARBSTAT_DMAW2_TRAP             0x00100000
#define BGE_MARBSTAT_XTSSRAM_ROFLO_TRAP     0x00200000
#define BGE_MARBSTAT_XTSSRAM_RUFLO_TRAP     0x00400000
#define BGE_MARBSTAT_XTSSRAM_WOFLO_TRAP     0x00800000
#define BGE_MARBSTAT_XTSSRAM_WUFLO_TRAP     0x01000000
#define BGE_MARBSTAT_XTSSRAM_PERR_TRAP      0x02000000

/*
 * Buffer manager control registers
 */
#define BGE_BMAN_MODE                       0x4400
#define BGE_BMAN_STATUS                     0x4404
#define BGE_BMAN_MBUFPOOL_BASEADDR          0x4408
#define BGE_BMAN_MBUFPOOL_LEN               0x440C
#define BGE_BMAN_MBUFPOOL_READDMA_LOWAT     0x4410
#define BGE_BMAN_MBUFPOOL_MACRX_LOWAT       0x4414
#define BGE_BMAN_MBUFPOOL_HIWAT             0x4418
#define BGE_BMAN_RXCPU_MBALLOC_REQ          0x441C
#define BGE_BMAN_RXCPU_MBALLOC_RESP         0x4420
#define BGE_BMAN_TXCPU_MBALLOC_REQ          0x4424
#define BGE_BMAN_TXCPU_MBALLOC_RESP         0x4428
#define BGE_BMAN_DMA_DESCPOOL_BASEADDR      0x442C
#define BGE_BMAN_DMA_DESCPOOL_LEN           0x4430
#define BGE_BMAN_DMA_DESCPOOL_LOWAT         0x4434
#define BGE_BMAN_DMA_DESCPOOL_HIWAT         0x4438
#define BGE_BMAN_RXCPU_DMAALLOC_REQ         0x443C
#define BGE_BMAN_RXCPU_DMAALLOC_RESP        0x4440
#define BGE_BMAN_TXCPU_DMAALLOC_REQ         0x4444
#define BGE_BMAN_TXCPU_DMALLLOC_RESP        0x4448
#define BGE_BMAN_HWDIAG_1                   0x444C
#define BGE_BMAN_HWDIAG_2                   0x4450
#define BGE_BMAN_HWDIAG_3                   0x4454

/* Buffer manager mode register */
#define BGE_BMANMODE_RESET            0x00000001
#define BGE_BMANMODE_ENABLE           0x00000002
#define BGE_BMANMODE_ATTN             0x00000004
#define BGE_BMANMODE_TESTMODE         0x00000008
#define BGE_BMANMODE_LOMBUF_ATTN      0x00000010

/* Buffer manager status register */
#define BGE_BMANSTAT_ERRO             0x00000004
#define BGE_BMANSTAT_LOWMBUF_ERROR    0x00000010


/*
 * Read DMA Control registers
 */
#define BGE_RDMA_MODE              0x4800
#define BGE_RDMA_STATUS            0x4804

/* Read DMA mode register */
#define BGE_RDMAMODE_RESET                  0x00000001
#define BGE_RDMAMODE_ENABLE                 0x00000002
#define BGE_RDMAMODE_PCI_TGT_ABRT_ATTN      0x00000004
#define BGE_RDMAMODE_PCI_MSTR_ABRT_ATTN     0x00000008
#define BGE_RDMAMODE_PCI_PERR_ATTN          0x00000010
#define BGE_RDMAMODE_PCI_ADDROFLOW_ATTN     0x00000020
#define BGE_RDMAMODE_PCI_FIFOOFLOW_ATTN     0x00000040
#define BGE_RDMAMODE_PCI_FIFOUFLOW_ATTN     0x00000080
#define BGE_RDMAMODE_PCI_FIFOOREAD_ATTN     0x00000100
#define BGE_RDMAMODE_LOCWRITE_TOOBIG        0x00000200
#define BGE_RDMAMODE_ALL_ATTNS              0x000003FC
#define BGE_RDMAMODE_BD_SBD_CRPT_ATTN       0x00000800
#define BGE_RDMAMODE_MBUF_RBD_CRPT_ATTN     0x00001000
#define BGE_RDMAMODE_MBUF_SBD_CRPT_ATTN     0x00002000
#define BGE_RDMAMODE_FIFO_SIZE_128          0x00020000
#define BGE_RDMAMODE_FIFO_LONG_BURST        0x00030000
#define BGE_RDMAMODE_TSO4_ENABLE            0x08000000
#define BGE_RDMAMODE_TSO6_ENABLE            0x10000000

/* Read DMA status register */
#define BGE_RDMASTAT_PCI_TGT_ABRT_ATTN      0x00000004
#define BGE_RDMASTAT_PCI_MSTR_ABRT_ATTN     0x00000008
#define BGE_RDMASTAT_PCI_PERR_ATTN          0x00000010
#define BGE_RDMASTAT_PCI_ADDROFLOW_ATTN     0x00000020
#define BGE_RDMASTAT_PCI_FIFOOFLOW_ATTN     0x00000040
#define BGE_RDMASTAT_PCI_FIFOUFLOW_ATTN     0x00000080
#define BGE_RDMASTAT_PCI_FIFOOREAD_ATTN     0x00000100
#define BGE_RDMASTAT_LOCWRITE_TOOBIG        0x00000200

/*
 * Write DMA control registers
 */
#define BGE_WDMA_MODE              0x4C00
#define BGE_WDMA_STATUS            0x4C04

/* Write DMA mode register */
#define BGE_WDMAMODE_RESET                  0x00000001
#define BGE_WDMAMODE_ENABLE                 0x00000002
#define BGE_WDMAMODE_PCI_TGT_ABRT_ATTN      0x00000004
#define BGE_WDMAMODE_PCI_MSTR_ABRT_ATTN     0x00000008
#define BGE_WDMAMODE_PCI_PERR_ATTN          0x00000010
#define BGE_WDMAMODE_PCI_ADDROFLOW_ATTN     0x00000020
#define BGE_WDMAMODE_PCI_FIFOOFLOW_ATTN     0x00000040
#define BGE_WDMAMODE_PCI_FIFOUFLOW_ATTN     0x00000080
#define BGE_WDMAMODE_PCI_FIFOOREAD_ATTN     0x00000100
#define BGE_WDMAMODE_LOCREAD_TOOBIG         0x00000200
#define BGE_WDMAMODE_ALL_ATTNS              0x000003FC
#define BGE_WDMAMODE_STATUS_TAG_FIX         0x20000000

/* Write DMA status register */
#define BGE_WDMASTAT_PCI_TGT_ABRT_ATTN      0x00000004
#define BGE_WDMASTAT_PCI_MSTR_ABRT_ATTN     0x00000008
#define BGE_WDMASTAT_PCI_PERR_ATTN          0x00000010
#define BGE_WDMASTAT_PCI_ADDROFLOW_ATTN     0x00000020
#define BGE_WDMASTAT_PCI_FIFOOFLOW_ATTN     0x00000040
#define BGE_WDMASTAT_PCI_FIFOUFLOW_ATTN     0x00000080
#define BGE_WDMASTAT_PCI_FIFOOREAD_ATTN     0x00000100
#define BGE_WDMASTAT_LOCREAD_TOOBIG         0x00000200


/*
 * RX CPU registers
 */
#define BGE_RXCPU_MODE             0x5000
#define BGE_RXCPU_STATUS           0x5004
#define BGE_RXCPU_PC               0x501C

/* RX CPU mode register */
#define BGE_RXCPUMODE_RESET                0x00000001
#define BGE_RXCPUMODE_SINGLESTEP           0x00000002
#define BGE_RXCPUMODE_P0_DATAHLT_ENB       0x00000004
#define BGE_RXCPUMODE_P0_INSTRHLT_ENB      0x00000008
#define BGE_RXCPUMODE_WR_POSTBUF_ENB       0x00000010
#define BGE_RXCPUMODE_DATACACHE_ENB        0x00000020
#define BGE_RXCPUMODE_ROMFAIL              0x00000040
#define BGE_RXCPUMODE_WATCHDOG_ENB         0x00000080
#define BGE_RXCPUMODE_INSTRCACHE_PRF       0x00000100
#define BGE_RXCPUMODE_INSTRCACHE_FLUSH     0x00000200
#define BGE_RXCPUMODE_HALTCPU              0x00000400
#define BGE_RXCPUMODE_INVDATAHLT_ENB       0x00000800
#define BGE_RXCPUMODE_MADDRTRAPHLT_ENB     0x00001000
#define BGE_RXCPUMODE_RADDRTRAPHLT_ENB     0x00002000

/* RX CPU status register */
#define BGE_RXCPUSTAT_HW_BREAKPOINT         0x00000001
#define BGE_RXCPUSTAT_HLTINSTR_EXECUTED     0x00000002
#define BGE_RXCPUSTAT_INVALID_INSTR         0x00000004
#define BGE_RXCPUSTAT_P0_DATAREF            0x00000008
#define BGE_RXCPUSTAT_P0_INSTRREF           0x00000010
#define BGE_RXCPUSTAT_INVALID_DATAACC       0x00000020
#define BGE_RXCPUSTAT_INVALID_INSTRFTCH     0x00000040
#define BGE_RXCPUSTAT_BAD_MEMALIGN          0x00000080
#define BGE_RXCPUSTAT_MADDR_TRAP            0x00000100
#define BGE_RXCPUSTAT_REGADDR_TRAP          0x00000200
#define BGE_RXCPUSTAT_DATAACC_STALL         0x00001000
#define BGE_RXCPUSTAT_INSTRFETCH_STALL      0x00002000
#define BGE_RXCPUSTAT_MA_WR_FIFOOFLOW       0x08000000
#define BGE_RXCPUSTAT_MA_RD_FIFOOFLOW       0x10000000
#define BGE_RXCPUSTAT_MA_DATAMASK_OFLOW     0x20000000
#define BGE_RXCPUSTAT_MA_REQ_FIFOOFLOW      0x40000000
#define BGE_RXCPUSTAT_BLOCKING_READ         0x80000000

/*
 * V? CPU registers
 */
#define BGE_VCPU_STATUS                       0x5100
#define BGE_VCPU_EXT_CTRL                     0x6890

#define BGE_VCPU_STATUS_INIT_DONE         0x04000000
#define BGE_VCPU_STATUS_DRV_RESET         0x08000000

#define BGE_VCPU_EXT_CTRL_HALT_CPU        0x00400000
#define BGE_VCPU_EXT_CTRL_DISABLE_WOL     0x20000000

/*
 * TX CPU registers
 */
#define BGE_TXCPU_MODE             0x5400
#define BGE_TXCPU_STATUS           0x5404
#define BGE_TXCPU_PC               0x541C

/* TX CPU mode register */
#define BGE_TXCPUMODE_RESET                0x00000001
#define BGE_TXCPUMODE_SINGLESTEP           0x00000002
#define BGE_TXCPUMODE_P0_DATAHLT_ENB       0x00000004
#define BGE_TXCPUMODE_P0_INSTRHLT_ENB      0x00000008
#define BGE_TXCPUMODE_WR_POSTBUF_ENB       0x00000010
#define BGE_TXCPUMODE_DATACACHE_ENB        0x00000020
#define BGE_TXCPUMODE_ROMFAIL              0x00000040
#define BGE_TXCPUMODE_WATCHDOG_ENB         0x00000080
#define BGE_TXCPUMODE_INSTRCACHE_PRF       0x00000100
#define BGE_TXCPUMODE_INSTRCACHE_FLUSH     0x00000200
#define BGE_TXCPUMODE_HALTCPU              0x00000400
#define BGE_TXCPUMODE_INVDATAHLT_ENB       0x00000800
#define BGE_TXCPUMODE_MADDRTRAPHLT_ENB     0x00001000

/* TX CPU status register */
#define BGE_TXCPUSTAT_HW_BREAKPOINT         0x00000001
#define BGE_TXCPUSTAT_HLTINSTR_EXECUTED     0x00000002
#define BGE_TXCPUSTAT_INVALID_INSTR         0x00000004
#define BGE_TXCPUSTAT_P0_DATAREF            0x00000008
#define BGE_TXCPUSTAT_P0_INSTRREF           0x00000010
#define BGE_TXCPUSTAT_INVALID_DATAACC       0x00000020
#define BGE_TXCPUSTAT_INVALID_INSTRFTCH     0x00000040
#define BGE_TXCPUSTAT_BAD_MEMALIGN          0x00000080
#define BGE_TXCPUSTAT_MADDR_TRAP            0x00000100
#define BGE_TXCPUSTAT_REGADDR_TRAP          0x00000200
#define BGE_TXCPUSTAT_DATAACC_STALL         0x00001000
#define BGE_TXCPUSTAT_INSTRFETCH_STALL      0x00002000
#define BGE_TXCPUSTAT_MA_WR_FIFOOFLOW       0x08000000
#define BGE_TXCPUSTAT_MA_RD_FIFOOFLOW       0x10000000
#define BGE_TXCPUSTAT_MA_DATAMASK_OFLOW     0x20000000
#define BGE_TXCPUSTAT_MA_REQ_FIFOOFLOW      0x40000000
#define BGE_TXCPUSTAT_BLOCKING_READ         0x80000000


/*
 * Low priority mailbox registers
 */
#define BGE_LPMBX_IRQ0_HI               0x5800
#define BGE_LPMBX_IRQ0_LO               0x5804
#define BGE_LPMBX_IRQ1_HI               0x5808
#define BGE_LPMBX_IRQ1_LO               0x580C
#define BGE_LPMBX_IRQ2_HI               0x5810
#define BGE_LPMBX_IRQ2_LO               0x5814
#define BGE_LPMBX_IRQ3_HI               0x5818
#define BGE_LPMBX_IRQ3_LO               0x581C
#define BGE_LPMBX_GEN0_HI               0x5820
#define BGE_LPMBX_GEN0_LO               0x5824
#define BGE_LPMBX_GEN1_HI               0x5828
#define BGE_LPMBX_GEN1_LO               0x582C
#define BGE_LPMBX_GEN2_HI               0x5830
#define BGE_LPMBX_GEN2_LO               0x5834
#define BGE_LPMBX_GEN3_HI               0x5828
#define BGE_LPMBX_GEN3_LO               0x582C
#define BGE_LPMBX_GEN4_HI               0x5840
#define BGE_LPMBX_GEN4_LO               0x5844
#define BGE_LPMBX_GEN5_HI               0x5848
#define BGE_LPMBX_GEN5_LO               0x584C
#define BGE_LPMBX_GEN6_HI               0x5850
#define BGE_LPMBX_GEN6_LO               0x5854
#define BGE_LPMBX_GEN7_HI               0x5858
#define BGE_LPMBX_GEN7_LO               0x585C
#define BGE_LPMBX_RELOAD_STATS_HI       0x5860
#define BGE_LPMBX_RELOAD_STATS_LO       0x5864
#define BGE_LPMBX_RX_STD_PROD_HI        0x5868
#define BGE_LPMBX_RX_STD_PROD_LO        0x586C
#define BGE_LPMBX_RX_JUMBO_PROD_HI      0x5870
#define BGE_LPMBX_RX_JUMBO_PROD_LO      0x5874
#define BGE_LPMBX_RX_MINI_PROD_HI       0x5878
#define BGE_LPMBX_RX_MINI_PROD_LO       0x587C
#define BGE_LPMBX_RX_CONS0_HI           0x5880
#define BGE_LPMBX_RX_CONS0_LO           0x5884
#define BGE_LPMBX_RX_CONS1_HI           0x5888
#define BGE_LPMBX_RX_CONS1_LO           0x588C
#define BGE_LPMBX_RX_CONS2_HI           0x5890
#define BGE_LPMBX_RX_CONS2_LO           0x5894
#define BGE_LPMBX_RX_CONS3_HI           0x5898
#define BGE_LPMBX_RX_CONS3_LO           0x589C
#define BGE_LPMBX_RX_CONS4_HI           0x58A0
#define BGE_LPMBX_RX_CONS4_LO           0x58A4
#define BGE_LPMBX_RX_CONS5_HI           0x58A8
#define BGE_LPMBX_RX_CONS5_LO           0x58AC
#define BGE_LPMBX_RX_CONS6_HI           0x58B0
#define BGE_LPMBX_RX_CONS6_LO           0x58B4
#define BGE_LPMBX_RX_CONS7_HI           0x58B8
#define BGE_LPMBX_RX_CONS7_LO           0x58BC
#define BGE_LPMBX_RX_CONS8_HI           0x58C0
#define BGE_LPMBX_RX_CONS8_LO           0x58C4
#define BGE_LPMBX_RX_CONS9_HI           0x58C8
#define BGE_LPMBX_RX_CONS9_LO           0x58CC
#define BGE_LPMBX_RX_CONS10_HI          0x58D0
#define BGE_LPMBX_RX_CONS10_LO          0x58D4
#define BGE_LPMBX_RX_CONS11_HI          0x58D8
#define BGE_LPMBX_RX_CONS11_LO          0x58DC
#define BGE_LPMBX_RX_CONS12_HI          0x58E0
#define BGE_LPMBX_RX_CONS12_LO          0x58E4
#define BGE_LPMBX_RX_CONS13_HI          0x58E8
#define BGE_LPMBX_RX_CONS13_LO          0x58EC
#define BGE_LPMBX_RX_CONS14_HI          0x58F0
#define BGE_LPMBX_RX_CONS14_LO          0x58F4
#define BGE_LPMBX_RX_CONS15_HI          0x58F8
#define BGE_LPMBX_RX_CONS15_LO          0x58FC
#define BGE_LPMBX_TX_HOST_PROD0_HI      0x5900
#define BGE_LPMBX_TX_HOST_PROD0_LO      0x5904
#define BGE_LPMBX_TX_HOST_PROD1_HI      0x5908
#define BGE_LPMBX_TX_HOST_PROD1_LO      0x590C
#define BGE_LPMBX_TX_HOST_PROD2_HI      0x5910
#define BGE_LPMBX_TX_HOST_PROD2_LO      0x5914
#define BGE_LPMBX_TX_HOST_PROD3_HI      0x5918
#define BGE_LPMBX_TX_HOST_PROD3_LO      0x591C
#define BGE_LPMBX_TX_HOST_PROD4_HI      0x5920
#define BGE_LPMBX_TX_HOST_PROD4_LO      0x5924
#define BGE_LPMBX_TX_HOST_PROD5_HI      0x5928
#define BGE_LPMBX_TX_HOST_PROD5_LO      0x592C
#define BGE_LPMBX_TX_HOST_PROD6_HI      0x5930
#define BGE_LPMBX_TX_HOST_PROD6_LO      0x5934
#define BGE_LPMBX_TX_HOST_PROD7_HI      0x5938
#define BGE_LPMBX_TX_HOST_PROD7_LO      0x593C
#define BGE_LPMBX_TX_HOST_PROD8_HI      0x5940
#define BGE_LPMBX_TX_HOST_PROD8_LO      0x5944
#define BGE_LPMBX_TX_HOST_PROD9_HI      0x5948
#define BGE_LPMBX_TX_HOST_PROD9_LO      0x594C
#define BGE_LPMBX_TX_HOST_PROD10_HI     0x5950
#define BGE_LPMBX_TX_HOST_PROD10_LO     0x5954
#define BGE_LPMBX_TX_HOST_PROD11_HI     0x5958
#define BGE_LPMBX_TX_HOST_PROD11_LO     0x595C
#define BGE_LPMBX_TX_HOST_PROD12_HI     0x5960
#define BGE_LPMBX_TX_HOST_PROD12_LO     0x5964
#define BGE_LPMBX_TX_HOST_PROD13_HI     0x5968
#define BGE_LPMBX_TX_HOST_PROD13_LO     0x596C
#define BGE_LPMBX_TX_HOST_PROD14_HI     0x5970
#define BGE_LPMBX_TX_HOST_PROD14_LO     0x5974
#define BGE_LPMBX_TX_HOST_PROD15_HI     0x5978
#define BGE_LPMBX_TX_HOST_PROD15_LO     0x597C
#define BGE_LPMBX_TX_NIC_PROD0_HI       0x5980
#define BGE_LPMBX_TX_NIC_PROD0_LO       0x5984
#define BGE_LPMBX_TX_NIC_PROD1_HI       0x5988
#define BGE_LPMBX_TX_NIC_PROD1_LO       0x598C
#define BGE_LPMBX_TX_NIC_PROD2_HI       0x5990
#define BGE_LPMBX_TX_NIC_PROD2_LO       0x5994
#define BGE_LPMBX_TX_NIC_PROD3_HI       0x5998
#define BGE_LPMBX_TX_NIC_PROD3_LO       0x599C
#define BGE_LPMBX_TX_NIC_PROD4_HI       0x59A0
#define BGE_LPMBX_TX_NIC_PROD4_LO       0x59A4
#define BGE_LPMBX_TX_NIC_PROD5_HI       0x59A8
#define BGE_LPMBX_TX_NIC_PROD5_LO       0x59AC
#define BGE_LPMBX_TX_NIC_PROD6_HI       0x59B0
#define BGE_LPMBX_TX_NIC_PROD6_LO       0x59B4
#define BGE_LPMBX_TX_NIC_PROD7_HI       0x59B8
#define BGE_LPMBX_TX_NIC_PROD7_LO       0x59BC
#define BGE_LPMBX_TX_NIC_PROD8_HI       0x59C0
#define BGE_LPMBX_TX_NIC_PROD8_LO       0x59C4
#define BGE_LPMBX_TX_NIC_PROD9_HI       0x59C8
#define BGE_LPMBX_TX_NIC_PROD9_LO       0x59CC
#define BGE_LPMBX_TX_NIC_PROD10_HI      0x59D0
#define BGE_LPMBX_TX_NIC_PROD10_LO      0x59D4
#define BGE_LPMBX_TX_NIC_PROD11_HI      0x59D8
#define BGE_LPMBX_TX_NIC_PROD11_LO      0x59DC
#define BGE_LPMBX_TX_NIC_PROD12_HI      0x59E0
#define BGE_LPMBX_TX_NIC_PROD12_LO      0x59E4
#define BGE_LPMBX_TX_NIC_PROD13_HI      0x59E8
#define BGE_LPMBX_TX_NIC_PROD13_LO      0x59EC
#define BGE_LPMBX_TX_NIC_PROD14_HI      0x59F0
#define BGE_LPMBX_TX_NIC_PROD14_LO      0x59F4
#define BGE_LPMBX_TX_NIC_PROD15_HI      0x59F8
#define BGE_LPMBX_TX_NIC_PROD15_LO      0x59FC

/*
 * Flow throw Queue reset register
 */
#define BGE_FTQ_RESET            0x5C00

#define BGE_FTQRESET_DMAREAD            0x00000002
#define BGE_FTQRESET_DMAHIPRIO_RD       0x00000004
#define BGE_FTQRESET_DMADONE            0x00000010
#define BGE_FTQRESET_SBDC               0x00000020
#define BGE_FTQRESET_SDI                0x00000040
#define BGE_FTQRESET_WDMA               0x00000080
#define BGE_FTQRESET_DMAHIPRIO_WR       0x00000100
#define BGE_FTQRESET_TYPE1_SOFTWARE     0x00000200
#define BGE_FTQRESET_SDC                0x00000400
#define BGE_FTQRESET_HCC                0x00000800
#define BGE_FTQRESET_TXFIFO             0x00001000
#define BGE_FTQRESET_MBC                0x00002000
#define BGE_FTQRESET_RBDC               0x00004000
#define BGE_FTQRESET_RXLP               0x00008000
#define BGE_FTQRESET_RDBDI              0x00010000
#define BGE_FTQRESET_RDC                0x00020000
#define BGE_FTQRESET_TYPE2_SOFTWARE     0x00040000

/*
 * Message Signaled Interrupt registers
 */
#define BGE_MSI_MODE              0x6000
#define BGE_MSI_STATUS            0x6004
#define BGE_MSI_FIFOACCESS        0x6008

/* MSI mode register */
#define BGE_MSIMODE_RESET                0x00000001
#define BGE_MSIMODE_ENABLE               0x00000002
#define BGE_MSIMODE_ONE_SHOT_DISABLE     0x00000020
#define BGE_MSIMODE_MULTIVEC_ENABLE      0x00000080

/* MSI status register */
#define BGE_MSISTAT_PCI_TGT_ABRT_ATTN      0x00000004
#define BGE_MSISTAT_PCI_MSTR_ABRT_ATTN     0x00000008
#define BGE_MSISTAT_PCI_PERR_ATTN          0x00000010
#define BGE_MSISTAT_MSI_FIFOUFLOW_ATTN     0x00000020
#define BGE_MSISTAT_MSI_FIFOOFLOW_ATTN     0x00000040


/*
 * DMA Completion registers
 */
#define BGE_DMAC_MODE            0x6400

/* DMA Completion mode register */
#define BGE_DMACMODE_RESET        0x00000001
#define BGE_DMACMODE_ENABLE       0x00000002


/*
 * General control registers.
 */
#define BGE_MODE_CTL                0x6800
#define BGE_MISC_CFG                0x6804
#define BGE_MISC_LOCAL_CTL          0x6808
#define BGE_CPU_EVENT               0x6810
#define BGE_EE_ADDR                 0x6838
#define BGE_EE_DATA                 0x683C
#define BGE_EE_CTL                  0x6840
#define BGE_MDI_CTL                 0x6844
#define BGE_EE_DELAY                0x6848
#define BGE_FASTBOOT_PC             0x6894

/*
 * NVRAM Control registers
 */
#define BGE_NVRAM_CMD               0x7000
#define BGE_NVRAM_STAT              0x7004
#define BGE_NVRAM_WRDATA            0x7008
#define BGE_NVRAM_ADDR              0x700c
#define BGE_NVRAM_RDDATA            0x7010
#define BGE_NVRAM_CFG1              0x7014
#define BGE_NVRAM_CFG2              0x7018
#define BGE_NVRAM_CFG3              0x701c
#define BGE_NVRAM_SWARB             0x7020
#define BGE_NVRAM_ACCESS            0x7024
#define BGE_NVRAM_WRITE1            0x7028

#define BGE_NVRAMCMD_RESET          0x00000001
#define BGE_NVRAMCMD_DONE           0x00000008
#define BGE_NVRAMCMD_START          0x00000010
#define BGE_NVRAMCMD_WR             0x00000020
#define BGE_NVRAMCMD_ERASE          0x00000040
#define BGE_NVRAMCMD_FIRST          0x00000080
#define BGE_NVRAMCMD_LAST           0x00000100

#define BGE_NVRAM_READCMD \
(BGE_NVRAMCMD_FIRST|BGE_NVRAMCMD_LAST| \
BGE_NVRAMCMD_START|BGE_NVRAMCMD_DONE)
#define BGE_NVRAM_WRITECMD \
(BGE_NVRAMCMD_FIRST|BGE_NVRAMCMD_LAST| \
BGE_NVRAMCMD_START|BGE_NVRAMCMD_DONE|BGE_NVRAMCMD_WR)

#define BGE_NVRAMSWARB_SET0        0x00000001
#define BGE_NVRAMSWARB_SET1        0x00000002
#define BGE_NVRAMSWARB_SET2        0x00000003
#define BGE_NVRAMSWARB_SET3        0x00000004
#define BGE_NVRAMSWARB_CLR0        0x00000010
#define BGE_NVRAMSWARB_CLR1        0x00000020
#define BGE_NVRAMSWARB_CLR2        0x00000040
#define BGE_NVRAMSWARB_CLR3        0x00000080
#define BGE_NVRAMSWARB_GNT0        0x00000100
#define BGE_NVRAMSWARB_GNT1        0x00000200
#define BGE_NVRAMSWARB_GNT2        0x00000400
#define BGE_NVRAMSWARB_GNT3        0x00000800
#define BGE_NVRAMSWARB_REQ0        0x00001000
#define BGE_NVRAMSWARB_REQ1        0x00002000
#define BGE_NVRAMSWARB_REQ2        0x00004000
#define BGE_NVRAMSWARB_REQ3        0x00008000

#define BGE_NVRAMACC_ENABLE        0x00000001
#define BGE_NVRAMACC_WRENABLE      0x00000002

/* Mode control register */
#define BGE_MODECTL_INT_SNDCOAL_ONLY      0x00000001
#define BGE_MODECTL_BYTESWAP_NONFRAME     0x00000002
#define BGE_MODECTL_WORDSWAP_NONFRAME     0x00000004
#define BGE_MODECTL_BYTESWAP_DATA         0x00000010
#define BGE_MODECTL_WORDSWAP_DATA         0x00000020
#define BGE_MODECTL_NO_FRAME_CRACKING     0x00000200
#define BGE_MODECTL_NO_RX_CRC             0x00000400
#define BGE_MODECTL_RX_BADFRAMES          0x00000800
#define BGE_MODECTL_NO_TX_INTR            0x00002000
#define BGE_MODECTL_NO_RX_INTR            0x00004000
#define BGE_MODECTL_FORCE_PCI32           0x00008000
#define BGE_MODECTL_STACKUP               0x00010000
#define BGE_MODECTL_HOST_SEND_BDS         0x00020000
#define BGE_MODECTL_TX_NO_PHDR_CSUM       0x00100000
#define BGE_MODECTL_RX_NO_PHDR_CSUM       0x00800000
#define BGE_MODECTL_TX_ATTN_INTR          0x01000000
#define BGE_MODECTL_RX_ATTN_INTR          0x02000000
#define BGE_MODECTL_MAC_ATTN_INTR         0x04000000
#define BGE_MODECTL_DMA_ATTN_INTR         0x08000000
#define BGE_MODECTL_FLOWCTL_ATTN_INTR     0x10000000
#define BGE_MODECTL_4X_SENDRING_SZ        0x20000000
#define BGE_MODECTL_FW_PROCESS_MCASTS     0x40000000


/* Misc. config register */
#define BGE_MISCCFG_RESET_CORE_CLOCKS     0x00000001
#define BGE_MISCCFG_TIMER_PRESCALER       0x000000FE
#define BGE_MISCCFG_BOARD_ID              0x0001E000
#define BGE_MISCCFG_BOARD_ID_5788         0x00010000
#define BGE_MISCCFG_BOARD_ID_5788M        0x00018000
#define BGE_MISCCFG_EPHY_IDDQ             0x00200000


#define BGE_32BITTIME_66MHZ        (0x41 << 1)

/* Misc. Local Control */
#define BGE_MLC_INTR_STATE              0x00000001
#define BGE_MLC_INTR_CLR                0x00000002
#define BGE_MLC_INTR_SET                0x00000004
#define BGE_MLC_INTR_ONATTN             0x00000008
#define BGE_MLC_MISCIO_IN0              0x00000100
#define BGE_MLC_MISCIO_IN1              0x00000200
#define BGE_MLC_MISCIO_IN2              0x00000400
#define BGE_MLC_MISCIO_OUTEN0           0x00000800
#define BGE_MLC_MISCIO_OUTEN1           0x00001000
#define BGE_MLC_MISCIO_OUTEN2           0x00002000
#define BGE_MLC_MISCIO_OUT0             0x00004000
#define BGE_MLC_MISCIO_OUT1             0x00008000
#define BGE_MLC_MISCIO_OUT2             0x00010000
#define BGE_MLC_EXTRAM_ENB              0x00020000
#define BGE_MLC_SRAM_SIZE               0x001C0000
#define BGE_MLC_BANK_SEL                0x00200000
#define BGE_MLC_SSRAM_TYPE              0x00400000
#define BGE_MLC_SSRAM_CYC_DESEL         0x00800000
#define BGE_MLC_AUTO_EEPROM             0x01000000

#define BGE_SSRAMSIZE_256KB             0x00000000
#define BGE_SSRAMSIZE_512KB             0x00040000
#define BGE_SSRAMSIZE_1MB               0x00080000
#define BGE_SSRAMSIZE_2MB               0x000C0000
#define BGE_SSRAMSIZE_4MB               0x00100000
#define BGE_SSRAMSIZE_8MB               0x00140000
#define BGE_SSRAMSIZE_16M               0x00180000


/* EEPROM Control register */
#define BGE_EECTL_CLKOUT_TRISTATE      0x00000001
#define BGE_EECTL_CLKOUT               0x00000002
#define BGE_EECTL_CLKIN                0x00000004
#define BGE_EECTL_DATAOUT_TRISTATE     0x00000008
#define BGE_EECTL_DATAOUT              0x00000010
#define BGE_EECTL_DATAIN               0x00000020


/* MDI (MII/GMII) access register */
#define BGE_MDI_DATA             0x00000001
#define BGE_MDI_DIR              0x00000002
#define BGE_MDI_SEL              0x00000004
#define BGE_MDI_CLK              0x00000008

#define BGE_MEMWIN_START         0x00008000
#define BGE_MEMWIN_END           0x0000FFFF


/*
 * This magic number is written to the firmware mailbox at 0xb50
 * before a software reset is issued.  After the internal firmware
 * has completed its initialization it will write the opposite of
 * this value, ~BGE_MAGIC_NUMBER, to the same location, allowing the
 * driver to synchronize with the firmware.
 */
#define BGE_MAGIC_NUMBER                0x4B657654

#define BGE_HOSTADDR(x, y)                              \
do {                                                    \
(x).addrLo = ((unsigned long long) (y) & 0xFFFFFFFF);   \
(x).addrHi = ((unsigned long long) (y) >> 32);          \
} while(0)

#define BGE_ADDR_LO(y) ((unsigned long long) (y) & 0xFFFFFFFF)
#define BGE_ADDR_HI(y) ((unsigned long long) (y) >> 32)

#define bcmRCBWrite(addr, field, val) \
writeNICMem(addr + offsetof(bcmRCB, field), val)

#define BGE_RCB_FLAG_USE_EXT_RX_BD       0x0001
#define BGE_RCB_FLAG_RING_DISABLED       0x0002

#define BGE_TXBDFLAG_TCP_UDP_CSUM        0x0001
#define BGE_TXBDFLAG_IP_CSUM             0x0002
#define BGE_TXBDFLAG_END                 0x0004
#define BGE_TXBDFLAG_IP_FRAG             0x0008
#define BGE_TXBDFLAG_IP_FRAG_END         0x0010
#define BGE_TXBDFLAG_VLAN_TAG            0x0040
#define BGE_TXBDFLAG_COAL_NOW            0x0080
#define BGE_TXBDFLAG_CPU_PRE_DMA         0x0100
#define BGE_TXBDFLAG_CPU_POST_DMA        0x0200
#define BGE_TXBDFLAG_INSERT_SRC_ADDR     0x1000
#define BGE_TXBDFLAG_CHOOSE_SRC_ADDR     0x6000
#define BGE_TXBDFLAG_NO_CRC              0x8000

#define BGE_NIC_TXRING_ADDR BGE_SEND_RING_1_TO_4

#define BGE_RXBDFLAG_END                 0x0004
#define BGE_RXBDFLAG_JUMBO_RING          0x0020
#define BGE_RXBDFLAG_VLAN_TAG            0x0040
#define BGE_RXBDFLAG_ERROR               0x0400
#define BGE_RXBDFLAG_MINI_RING           0x0800
#define BGE_RXBDFLAG_IP_CSUM             0x1000
#define BGE_RXBDFLAG_TCP_UDP_CSUM        0x2000
#define BGE_RXBDFLAG_TCP_UDP_IS_TCP      0x4000

#define BGE_RXERRFLAG_BAD_CRC            0x0001
#define BGE_RXERRFLAG_COLL_DETECT        0x0002
#define BGE_RXERRFLAG_LINK_LOST          0x0004
#define BGE_RXERRFLAG_PHY_DECODE_ERR     0x0008
#define BGE_RXERRFLAG_MAC_ABORT          0x0010
#define BGE_RXERRFLAG_RUNT               0x0020
#define BGE_RXERRFLAG_TRUNC_NO_RSRCS     0x0040
#define BGE_RXERRFLAG_GIANT              0x0080

/*
 * Offset of MAC address inside EEPROM.
 */
#define BGE_EE_MAC_OFFSET              0x7C
#define BGE_EE_MAC_OFFSET_5906         0x10
#define BGE_EE_HWCFG_OFFSET            0xC8

#define BGE_HWCFG_VOLTAGE             0x00000003
#define BGE_HWCFG_PHYLED_MODE         0x0000000C
#define BGE_HWCFG_MEDIA               0x00000030
#define BGE_HWCFG_ASF                 0x00000080

#define BGE_VOLTAGE_1POINT3           0x00000000
#define BGE_VOLTAGE_1POINT8           0x00000001

#define BGE_PHYLEDMODE_UNSPEC         0x00000000
#define BGE_PHYLEDMODE_TRIPLELED      0x00000004
#define BGE_PHYLEDMODE_SINGLELED      0x00000008

#define BGE_MEDIA_UNSPEC              0x00000000
#define BGE_MEDIA_COPPER              0x00000010
#define BGE_MEDIA_FIBER               0x00000020

#define BGE_TICKS_PER_SEC             1000000

/*
 * Ring size constants.
 */
#define BGE_STD_RX_RING_CNT 512
#define BGE_RX_RING_CNT     512
#define BGE_TX_RING_CNT     512

#define BGE_FRAMELEN        1518
#define BGE_MAX_FRAMELEN    1536
#define BGE_MIN_FRAMELEN    60

#define BGE_INC(x, y)    (x) = (x + 1) % y

typedef enum
{
    BGE_MEDIUM_AUTO = 0,
    BGE_MEDIUM_10HD,
    BGE_MEDIUM_10FD,
    BGE_MEDIUM_100HD,
    BGE_MEDIUM_100FD,
    BGE_MEDIUM_100T4
} bcmMediumType;

typedef struct
{
    UInt32 addrHi;
    UInt32 addrLo;
} bcmHostAddr;

typedef struct
{
    UInt32 statusWord;
    UInt32 statusTag;
    UInt16 unused1;
    UInt16 producerRingCons;
    UInt32 unused2;
    UInt16 returnRingProd;
    UInt16 sendRingCons;
} bcmStatusBlock;

typedef struct
{
    bcmHostAddr ringHostAddr;
    UInt32 maxlen_flags;
    UInt32 nicAddr;
} bcmRCB;

typedef struct
{
    bcmHostAddr bufHostAddr;
    UInt16 length;
    UInt16 index;
    UInt16 flags;
    UInt16 type;
    UInt16 tcp_udp_cksum;
    UInt16 ip_cksum;
    UInt16 vlanTag;
    UInt16 errorFlags;
    UInt32 rssHash;
    UInt32 opaque;
} bcmReceiveBD;

typedef struct
{
    bcmHostAddr bufHostAddr;
    UInt16 flags;
    UInt16 length;
    UInt16 vlanTag;
    UInt16 launchTime;
} bcmSendBD;

#define BGE_TX_RING_SZ (sizeof(bcmSendBD)    * BGE_TX_RING_CNT)
#define BGE_RX_RING_SZ (sizeof(bcmReceiveBD) * BGE_RX_RING_CNT)

#define BGE_TIMEOUT        100000

#define BGE_PHY_RESET 0x8000

#define BGE_MII_CTL       0x00
#define BGE_MII_STATUS    0x01
#define BGE_MII_ANAR      0x04
#define BGE_MII_ANLPAR    0x05
#define BGE_MII_INTERRUPT 0x1A

#define BGE_CAPABLE_100_T4         1 << 15
#define BGE_CAPABLE_100_TX_FD      1 << 14
#define BGE_CAPABLE_100_TX_HD      1 << 13
#define BGE_CAPABLE_10_FD          1 << 12
#define BGE_CAPABLE_10_HD          1 << 11

#define BGE_MII_CTL_FORCED_100          0x2000
#define BGE_MII_CTL_FORCED_10           0xDFFF
#define BGE_MII_CTL_AUTONEG_ENABLE      0x1000
#define BGE_MII_CTL_AUTONEG_DISABLE     0xEFFF
#define BGE_MII_CTL_DUPLEX_FULL         0x0100
#define BGE_MII_CTL_DUPLEX_HALF         0xFEFF
#define BGE_MII_CTL_RESTART_AUTONEG     0x0200

#define BGE_MII_STS_LINK                0x0004
#define BGE_MII_STS_AUTONEG_COMP        0x0020

#define BGE_MII_ANAR_T4       0x0200
#define BGE_MII_ANAR_TX_FD    0x0100
#define BGE_MII_ANAR_TX_HD    0x0080
#define BGE_MII_ANAR_10_FD    0x0040
#define BGE_MII_ANAR_10_HD    0x0020

#define BGE_MII_ANLPAR_T4       0x0200
#define BGE_MII_ANLPAR_TX_FD    0x0100
#define BGE_MII_ANLPAR_TX_HD    0x0080
#define BGE_MII_ANLPAR_10_FD    0x0040
#define BGE_MII_ANLPAR_10_HD    0x0020

#define BGE_STATUSWORD_WAS_UPDATED     0x1
#define BGE_STATUSWORD_LINK_CHANGED    0x2
#define BGE_STATUSWORD_ERROR           0x4

#define ETH_ADDR_LEN        6
#define ETH_TYPE_LEN        2
#define ETH_CRC_LEN         4
#define ETH_HDR_LEN         (ETH_ADDR_LEN*2 + ETH_TYPE_LEN)
#define ETH_VLAN_TAG_LEN    4

// Statistics

#define BFE_STAT_TX_IF_HC_OUT_OCTETS                            0x800
#define BFE_STAT_TX_ETHER_STATS_COLLISIONS                      0x808
#define BFE_STAT_TX_OUT_XON_SENT                                0x80C
#define BFE_STAT_TX_OUT_XOFF_SEND                               0x810
#define BFE_STAT_TX_DOT3_STATS_INTERNAL_MAC_TRANSMIT_ERRORS     0x818
#define BFE_STAT_TX_DOT3_STATS_SINGLE_COLLISION_FRAMES          0x81C
#define BFE_STAT_TX_DOT3_STATS_MULTIPLE_COLLISION_FRAMES        0x820
#define BFE_STAT_TX_DOT3_STATS_DEFERRED_TRANSMISSIONS           0x824
#define BFE_STAT_TX_DOT3_STATS_EXCESSIVE_COLLISIONS             0x82C
#define BFE_STAT_TX_DOT3_STATS_LATE_COLLISIONS                  0x830
#define BFE_STAT_TX_IF_HC_OUT_UCAST_PKTS                        0x86C
#define BFE_STAT_TX_IF_HC_OUT_MULTICAST_PKTS                    0x870
#define BFE_STAT_TX_IF_HC_OUT_BROADCAST_PKTS                    0x874

#define BFE_STAT_RX_IF_HC_IN_OCTETS                             0x880
#define BFE_STAT_RX_IF_HC_IN_BAD_OCTETS                         0x884
#define BFE_STAT_RX_ETHER_STATS_FRAGMENTS                       0x888
#define BFE_STAT_RX_IF_HC_IN_UCAST_PKTS                         0x88C
#define BFE_STAT_RX_IF_HC_IN_MULTICAST_PKTS                     0x890
#define BFE_STAT_RX_IF_HC_IN_BROADCAST_PKTS                     0x894
#define BFE_STAT_RX_DOT3_STATS_FCS_ERRORS                       0x898
#define BFE_STAT_RX_DOT3_STATS_ALIGNMENT_ERRORS                 0x89C
#define BFE_STAT_RX_XON_PAUSE_FRAMES_RECEIVED                   0x8A0
#define BFE_STAT_RX_XOFF_PAUSE_FRAMES_RECEIVED                  0x8A4
#define BFE_STAT_RX_MAC_CONTROL_FRAMES_RECEIVED                 0x8A8
#define BFE_STAT_RX_XOFF_STATE_ENTERED                          0x8AC
#define BFE_STAT_RX_DOT3_STATS_FRAMES_TOO_LONG                  0x8B0
#define BFE_STAT_RX_ETHER_STATS_JABBERS                         0x8B4
#define BFE_STAT_RX_ETHER_STATS_UNDERSIZE_PKTS                  0x8B8

#endif
