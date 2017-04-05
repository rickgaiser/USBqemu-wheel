#ifndef USBINTERNAL_H
#define USBINTERNAL_H

#include "vl.h"

#ifdef _DEBUG
//#define DEBUG_OHCI
#endif
/* Dump packet contents.  */
//#define DEBUG_PACKET
/* This causes frames to occur 1000x slower */
//#define OHCI_TIME_WARP 1

/* Number of Downstream Ports on the root hub.  */

#define OHCI_MAX_PORTS 15 // status regs from 0x0c54 but usb snooping
                          // reg is at 0x0c80, so only 11 ports?

extern int64_t usb_frame_time;
extern int64_t usb_bit_time;

typedef struct OHCIPort {
    USBPort port;
    uint32_t ctrl;
} OHCIPort;

typedef uint32_t target_phys_addr_t;

typedef struct OHCIState {
    target_phys_addr_t mem_base;
    int mem;
    int num_ports;

    uint64_t eof_timer;
    int64_t sof_time;

    /* OHCI state */
    /* Control partition */
    uint32_t ctl, status;
    uint32_t intr_status;
    uint32_t intr;

    /* memory pointer partition */
    uint32_t hcca;
    uint32_t ctrl_head, ctrl_cur;
    uint32_t bulk_head, bulk_cur;
    uint32_t per_cur;
    uint32_t done;
    int done_count;

    /* Frame counter partition */
    uint32_t fsmps:15;
    uint32_t fit:1;
    uint32_t fi:14;
    uint32_t frt:1;
    uint16_t frame_number;
    uint16_t padding;
    uint32_t pstart;
    uint32_t lst;

    /* Root Hub partition */
    uint32_t rhdesc_a, rhdesc_b;
    uint32_t rhstatus;
    OHCIPort rhport[OHCI_MAX_PORTS];
    
    /* Active packets.  */
    uint32_t old_ctl;
    uint8_t usb_buf[8192];
    
} OHCIState;

/* Host Controller Communications Area */
struct ohci_hcca {
    uint32_t intr[32];
    uint16_t frame, pad;
    uint32_t done;
};

// No offsetof in C++.
#ifndef offsetof
#define offsetof_(x,z) (intptr_t(&((x)->z)) - intptr_t(x))
#define offsetof(x,z) offsetof_((x*)0,z)
#endif

#define HCCA_WRITEBACK_OFFSET   128 //offsetof(struct ohci_hcca, frame)
#define HCCA_WRITEBACK_SIZE     8 /* frame, pad, done */

#define ED_WBACK_OFFSET 8 //offsetof(struct ohci_ed, head)
#define ED_WBACK_SIZE   4

/* Bitfields for the first word of an Endpoint Descriptor.  */
#define OHCI_ED_FA_SHIFT  0  //device address
#define OHCI_ED_FA_MASK   (0x7f<<OHCI_ED_FA_SHIFT)
#define OHCI_ED_EN_SHIFT  7 //endpoint number
#define OHCI_ED_EN_MASK   (0xf<<OHCI_ED_EN_SHIFT)
#define OHCI_ED_D_SHIFT   11 //direction
#define OHCI_ED_D_MASK    (3<<OHCI_ED_D_SHIFT)
#define OHCI_ED_S         (1<<13) //speed 0 - full, 1 - low
#define OHCI_ED_K         (1<<14) //skip ED if 1
#define OHCI_ED_F         (1<<15) //format 0 - inter, bulk or setup, 1 - isoch
//#define OHCI_ED_MPS_SHIFT 7
//#define OHCI_ED_MPS_MASK  (0xf<<OHCI_ED_FA_SHIFT)

#define OHCI_ED_MPS_SHIFT 16 //max packet size
#define OHCI_ED_MPS_MASK  (0x7ff<<OHCI_ED_MPS_SHIFT)

/* Flags in the head field of an Endpoint Descriptor.  */
#define OHCI_ED_H         1 //halted
#define OHCI_ED_C         2

/* Bitfields for the first word of a Transfer Descriptor.  */
#define OHCI_TD_R         (1<<18)
#define OHCI_TD_DP_SHIFT  19
#define OHCI_TD_DP_MASK   (3<<OHCI_TD_DP_SHIFT)
#define OHCI_TD_DI_SHIFT  21
#define OHCI_TD_DI_MASK   (7<<OHCI_TD_DI_SHIFT)
#define OHCI_TD_T0        (1<<24)
#define OHCI_TD_T1        (1<<24)
#define OHCI_TD_EC_SHIFT  26
#define OHCI_TD_EC_MASK   (3<<OHCI_TD_EC_SHIFT)
#define OHCI_TD_CC_SHIFT  28
#define OHCI_TD_CC_MASK   (0xf<<OHCI_TD_CC_SHIFT)

/* Bitfields for the first word of an Isochronous Transfer Descriptor.  */
/* CC & DI - same as in the General Transfer Descriptor */
#define OHCI_TD_SF_SHIFT  0
#define OHCI_TD_SF_MASK   (0xffff<<OHCI_TD_SF_SHIFT)
#define OHCI_TD_FC_SHIFT  24
#define OHCI_TD_FC_MASK   (7<<OHCI_TD_FC_SHIFT)

/* Isochronous Transfer Descriptor - Offset / PacketStatusWord */
#define OHCI_TD_PSW_CC_SHIFT 12
#define OHCI_TD_PSW_CC_MASK  (0xf<<OHCI_TD_PSW_CC_SHIFT)
#define OHCI_TD_PSW_SIZE_SHIFT 0
#define OHCI_TD_PSW_SIZE_MASK  (0xfff<<OHCI_TD_PSW_SIZE_SHIFT)

#define OHCI_PAGE_MASK    0xfffff000
#define OHCI_OFFSET_MASK  0xfff

#define OHCI_DPTR_MASK    0xfffffff0

#define OHCI_BM(val, field) \
  (((val) & OHCI_##field##_MASK) >> OHCI_##field##_SHIFT)

#define OHCI_SET_BM(val, field, newval) do { \
    val &= ~OHCI_##field##_MASK; \
    val |= ((newval) << OHCI_##field##_SHIFT) & OHCI_##field##_MASK; \
    } while(0)

/* endpoint descriptor */
struct ohci_ed {
    uint32_t flags;
    uint32_t tail;
    uint32_t head;
    uint32_t next;
};

/* General transfer descriptor */
struct ohci_td {
    uint32_t flags;
    uint32_t cbp;
    uint32_t next;
    uint32_t be;
};

/* Isochronous transfer descriptor */
struct ohci_iso_td {
    uint32_t flags;
    uint32_t bp;
    uint32_t next;
    uint32_t be;
    uint16_t offset[8];
};

#define USB_HZ                      12000000

/* OHCI Local stuff */
#define OHCI_CTL_CBSR         ((1<<0)|(1<<1))
#define OHCI_CTL_PLE          (1<<2)
#define OHCI_CTL_IE           (1<<3)
#define OHCI_CTL_CLE          (1<<4)
#define OHCI_CTL_BLE          (1<<5)
#define OHCI_CTL_HCFS         ((1<<6)|(1<<7))
#define OHCI_USB_RESET        0x00
#define OHCI_USB_RESUME       0x40
#define OHCI_USB_OPERATIONAL  0x80
#define OHCI_USB_SUSPEND      0xc0
#define OHCI_CTL_IR           (1<<8)
#define OHCI_CTL_RWC          (1<<9)
#define OHCI_CTL_RWE          (1<<10)

#define OHCI_STATUS_HCR       (1<<0)
#define OHCI_STATUS_CLF       (1<<1)
#define OHCI_STATUS_BLF       (1<<2)
#define OHCI_STATUS_OCR       (1<<3)
#define OHCI_STATUS_SOC       ((1<<6)|(1<<7)) //TODO LSI has SOC at bits 16,17?

#define OHCI_INTR_SO          (1U<<0) /* Scheduling overrun */
#define OHCI_INTR_WD          (1U<<1) /* HcDoneHead writeback */
#define OHCI_INTR_SF          (1U<<2) /* Start of frame */
#define OHCI_INTR_RD          (1U<<3) /* Resume detect */
#define OHCI_INTR_UE          (1U<<4) /* Unrecoverable error */
#define OHCI_INTR_FNO         (1U<<5) /* Frame number overflow */
#define OHCI_INTR_RHSC        (1U<<6) /* Root hub status change */
#define OHCI_INTR_OC          (1U<<30) /* Ownership change */
#define OHCI_INTR_MIE         (1U<<31) /* Master Interrupt Enable */

#define OHCI_HCCA_SIZE        0x100
#define OHCI_HCCA_MASK        0xffffff00

#define OHCI_EDPTR_MASK       0xfffffff0

#define OHCI_FMI_FI           0x00003fff
#define OHCI_FMI_FSMPS        0xffff0000
#define OHCI_FMI_FIT          0x80000000

#define OHCI_FR_RT            (1U<<31)

#define OHCI_LS_THRESH        0x628

#define OHCI_RHA_RW_MASK      0x00000000 /* Mask of supported features.  */
#define OHCI_RHA_PSM          (1<<8)
#define OHCI_RHA_NPS          (1<<9)
#define OHCI_RHA_DT           (1<<10)
#define OHCI_RHA_OCPM         (1<<11)
#define OHCI_RHA_NOCP         (1<<12)
#define OHCI_RHA_POTPGT_MASK  0xff000000

#define OHCI_RHS_LPS          (1U<<0)
#define OHCI_RHS_OCI          (1U<<1)
#define OHCI_RHS_DRWE         (1U<<15)
#define OHCI_RHS_LPSC         (1U<<16)
#define OHCI_RHS_OCIC         (1U<<17)
#define OHCI_RHS_CRWE         (1U<<31)

#define OHCI_PORT_CCS         (1<<0)
#define OHCI_PORT_PES         (1<<1)
#define OHCI_PORT_PSS         (1<<2)
#define OHCI_PORT_POCI        (1<<3)
#define OHCI_PORT_PRS         (1<<4)
#define OHCI_PORT_PPS         (1<<8)
#define OHCI_PORT_LSDA        (1<<9)
#define OHCI_PORT_CSC         (1<<16)
#define OHCI_PORT_PESC        (1<<17)
#define OHCI_PORT_PSSC        (1<<18)
#define OHCI_PORT_OCIC        (1<<19)
#define OHCI_PORT_PRSC        (1<<20)
#define OHCI_PORT_WTC         (OHCI_PORT_CSC|OHCI_PORT_PESC|OHCI_PORT_PSSC \
                               |OHCI_PORT_OCIC|OHCI_PORT_PRSC)

#define OHCI_TD_DIR_SETUP     0x0
#define OHCI_TD_DIR_OUT       0x1
#define OHCI_TD_DIR_IN        0x2
#define OHCI_TD_DIR_RESERVED  0x3

#define OHCI_CC_NOERROR             0x0
#define OHCI_CC_CRC                 0x1
#define OHCI_CC_BITSTUFFING         0x2
#define OHCI_CC_DATATOGGLEMISMATCH  0x3
#define OHCI_CC_STALL               0x4
#define OHCI_CC_DEVICENOTRESPONDING 0x5
#define OHCI_CC_PIDCHECKFAILURE     0x6
#define OHCI_CC_UNDEXPETEDPID       0x7 // the what?
#define OHCI_CC_DATAOVERRUN         0x8
#define OHCI_CC_DATAUNDERRUN        0x9
#define OHCI_CC_BUFFEROVERRUN       0xc
#define OHCI_CC_BUFFERUNDERRUN      0xd

OHCIState *ohci_create(uint32_t base, int ports);

uint32_t ohci_mem_read(OHCIState *ohci, uint32_t addr );
void ohci_mem_write(OHCIState *ohci, uint32_t addr, uint32_t value );
void ohci_frame_boundary(void *opaque);

void ohci_hard_reset(OHCIState *ohci);
int ohci_bus_start(OHCIState *ohci);
void ohci_bus_stop(OHCIState *ohci);
#endif
