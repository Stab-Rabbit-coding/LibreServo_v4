/**
 * @file    ls_mspm0_i2c_regs.h
 * @brief   Register map for the TI MSPM0G3518-Q1 I2C peripheral, as used by
 *          the LibreServo OPTIGA(TM) Trust M PAL port.
 *
 * Every offset, field position and magic key in this file was read from the
 * cited primary sources in this session.  Nothing is reproduced from memory
 * and nothing is inferred from a similar TI part (AGENTS.md section 1.3).
 *
 * References (see REFERENCES.md):
 *   [46] TI, MSPM0G3519-Q1/MSPM0G3518-Q1 Mixed-Signal Microcontrollers,
 *        SLASFA6B — section 8.8.2 Table 8-6 "Peripherals Summary" for base
 *        addresses.
 *   [52] TI, MSPM0 G-Series 80MHz Microcontrollers Technical Reference Manual,
 *        SLAU846E — chapter 25 "I2C" for offsets and bit fields.
 *   [53] Infineon, OPTIGA(TM) Trust M Solution Reference Manual, Rev 3.70.
 *
 * Naming note: SLAU846E section 25.2's prose refers to the controller registers
 * by their older `MSA`/`MCTR`/`MSR`/`MTPR`/`MCR`/`MBMON` names, while
 * section 25.3 Table 25-21 — the normative register list for this revision —
 * names them `CSA`/`CCTR`/`CSR`/`CTPR`/`CCR`/`CBMON`.  This header follows
 * section 25.3.  Where the older name is likely to be searched for, it is given
 * in a trailing comment.
 *
 * Written by Claude Opus 5 (claude-opus-5) under human direction, 2026-08-23.
 */

#ifndef LS_MSPM0_I2C_REGS_H_
#define LS_MSPM0_I2C_REGS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------
 * Peripheral base addresses — [46] section 8.8.2, Table 8-6
 * ------------------------------------------------------------------------ */

/** I2C0 base address.  SE_I2C_SCL (PA1) and SE_I2C_SDA (PA8) are both I2C0
 *  functions — see TODO.md 3.1 and [46] Table 6-2. */
#define LS_I2C0_BASE        (0x400F0000UL)

/** GPIOA base address.  SE_RST is PA14 (U1 pin 18). */
#define LS_GPIOA_BASE       (0x400A0000UL)

/** IOMUX base address, for selecting the PF3 (I2C0) alternate function on
 *  PA1/PA8 and the GPIO function on PA14. */
#define LS_IOMUX_BASE       (0x40428000UL)

/* ------------------------------------------------------------------------
 * I2C register offsets — [52] section 25.3, p. 1310, Table 25-21
 * ------------------------------------------------------------------------ */

#define LS_I2C_O_PWREN      (0x0800UL)  /**< Power enable            25.3.1  */
#define LS_I2C_O_RSTCTL     (0x0804UL)  /**< Reset control           25.3.2  */
#define LS_I2C_O_CLKDIV     (0x1000UL)  /**< Clock divider           25.3.5  */
#define LS_I2C_O_CLKSEL     (0x1004UL)  /**< Clock select            25.3.6  */
#define LS_I2C_O_RIS        (0x1030UL)  /**< Raw interrupt status    25.3.10 */
#define LS_I2C_O_ICLR       (0x1048UL)  /**< Interrupt clear         25.3.13 */
#define LS_I2C_O_GFCTL      (0x1200UL)  /**< Glitch filter control   25.3.29 */
#define LS_I2C_O_CSA        (0x1210UL)  /**< Target address (MSA)    25.3.32 */
#define LS_I2C_O_CCTR       (0x1214UL)  /**< Controller ctrl (MCTR)  25.3.33 */
#define LS_I2C_O_CSR        (0x1218UL)  /**< Controller status (MSR) 25.3.34 */
#define LS_I2C_O_CRXDATA    (0x121CUL)  /**< Controller RX (MRXDATA) 25.3.35 */
#define LS_I2C_O_CTXDATA    (0x1220UL)  /**< Controller TX (MTXDATA) 25.3.36 */
#define LS_I2C_O_CTPR       (0x1224UL)  /**< Timer period (MTPR)     25.3.37 */
#define LS_I2C_O_CCR        (0x1228UL)  /**< Controller config (MCR) 25.3.38 */
#define LS_I2C_O_CBMON      (0x1234UL)  /**< Bus monitor (MBMON)     25.3.39 */
#define LS_I2C_O_CFIFOCTL   (0x1238UL)  /**< Controller FIFO control 25.3.40 */
#define LS_I2C_O_CFIFOSR    (0x123CUL)  /**< Controller FIFO status  25.3.41 */

/* ------------------------------------------------------------------------
 * PWREN — [52] section 25.3.1, p. 1312, Table 25-23
 * ------------------------------------------------------------------------ */

/** Write key required in bits 31-24 for PWREN to accept a write.
 *  "26h = KEY to allow write access to this register". */
#define LS_I2C_PWREN_KEY            (0x26UL << 24)
/** Bit 0: 1h = Enable Power. */
#define LS_I2C_PWREN_ENABLE         (1UL << 0)

/* ------------------------------------------------------------------------
 * RSTCTL — [52] section 25.3.2, p. 1313, Table 25-24
 * ------------------------------------------------------------------------ */

/** Write key required in bits 31-24. "B1h = KEY to allow write access". */
#define LS_I2C_RSTCTL_KEY           (0xB1UL << 24)
/** 1h = Assert reset. */
#define LS_I2C_RSTCTL_RESETASSERT   (1UL << 0)
/** 1h = Clear reset sticky bit in STAT. */
#define LS_I2C_RSTCTL_RESETSTKYCLR  (1UL << 1)

/* ------------------------------------------------------------------------
 * CSA — controller target address — [52] section 25.3.32, p. 1348, Table 25-54
 * ------------------------------------------------------------------------ */

/** Target address field, bits 10-1. */
#define LS_I2C_CSA_TADDR_Pos        (1U)
#define LS_I2C_CSA_TADDR_Msk        (0x3FFUL << LS_I2C_CSA_TADDR_Pos)
/** Bit 0: 0h = transmit, 1h = receive. */
#define LS_I2C_CSA_DIR_RX           (1UL << 0)
/** Bit 15: 0h = 7-bit addressing mode, 1h = 10-bit.  The Trust M is 7-bit
 *  ([53] section 4.4 Table 35, BASE_ADDR 0x30), so this bit stays clear. */
#define LS_I2C_CSA_CMODE_10BIT      (1UL << 15)

/* ------------------------------------------------------------------------
 * CCTR — controller control — [52] section 25.3.33, p. 1349, Table 25-55
 * ------------------------------------------------------------------------ */

/** Transaction length in bytes, bits 27-16 ("CBLEN"; called MBLEN in the
 *  section 25.2 prose).  Maximum FFFh. */
#define LS_I2C_CCTR_CBLEN_Pos       (16U)
#define LS_I2C_CCTR_CBLEN_Msk       (0xFFFUL << LS_I2C_CCTR_CBLEN_Pos)
#define LS_I2C_CCTR_CBLEN_MAX       (0xFFFU)
/** Bit 5: transmit the whole TX FIFO before turning the bus around for a
 *  programmed burst read. */
#define LS_I2C_CCTR_RD_ON_TXEMPTY   (1UL << 5)
/** Bit 4: controller ACK override enable. */
#define LS_I2C_CCTR_CACKOEN         (1UL << 4)
/** Bit 3: 1h = the last received data byte of a transaction is acknowledged
 *  automatically by the controller. */
#define LS_I2C_CCTR_ACK             (1UL << 3)
/** Bit 2: 1h = the controller generates the STOP condition. */
#define LS_I2C_CCTR_STOP            (1UL << 2)
/** Bit 1: 1h = the controller generates the START or repeated START. */
#define LS_I2C_CCTR_START           (1UL << 1)
/** Bit 0: 1h = the controller is able to transmit or receive data. */
#define LS_I2C_CCTR_BURSTRUN        (1UL << 0)

/* ------------------------------------------------------------------------
 * CSR — controller status — [52] section 25.3.34, p. 1351, Table 25-56
 * ------------------------------------------------------------------------ */

/** Remaining/transferred byte count, bits 27-16. */
#define LS_I2C_CSR_CBCNT_Pos        (16U)
#define LS_I2C_CSR_CBCNT_Msk        (0xFFFUL << LS_I2C_CSR_CBCNT_Pos)
/** Bit 6: bus busy (another controller or an in-progress transfer). */
#define LS_I2C_CSR_BUSBSY           (1UL << 6)
/** Bit 5: controller idle.  Reset value is 1. */
#define LS_I2C_CSR_IDLE             (1UL << 5)
/** Bit 4: arbitration lost. */
#define LS_I2C_CSR_ARBLST           (1UL << 4)
/** Bit 3: data byte was not acknowledged by the target. */
#define LS_I2C_CSR_DATACK           (1UL << 3)
/** Bit 2: address byte was not acknowledged by the target. */
#define LS_I2C_CSR_ADRACK           (1UL << 2)
/** Bit 1: error. */
#define LS_I2C_CSR_ERR              (1UL << 1)
/** Bit 0: controller busy with a transaction. */
#define LS_I2C_CSR_BUSY             (1UL << 0)

/** Every CSR bit that means "this transfer did not complete cleanly".
 *  Grouped here so no caller can check a subset by accident. */
#define LS_I2C_CSR_ANY_ERROR                                                  \
    (LS_I2C_CSR_ERR | LS_I2C_CSR_ARBLST | LS_I2C_CSR_DATACK | LS_I2C_CSR_ADRACK)

/* ------------------------------------------------------------------------
 * CTPR — timer period — [52] section 25.3.37, p. 1354, Table 25-59
 * and the clock derivation of section 25.2.1, p. 1285, Equation 27.
 * ------------------------------------------------------------------------ */

/** TPR field, bits 6-0. */
#define LS_I2C_CTPR_TPR_Msk         (0x7FUL)

/** SCL low-phase constant.  [52] section 25.2.1: "SCL_LP is the low phase of
 *  SCL (which must be fixed at 6)". */
#define LS_I2C_SCL_LP               (6U)
/** SCL high-phase constant.  [52] section 25.2.1: "SCL_HP ... must be fixed
 *  at 4". */
#define LS_I2C_SCL_HP               (4U)

/**
 * @brief Compute the CTPR.TPR value for a target SCL frequency.
 *
 * [52] section 25.2.1, Equation 27:
 *     I2C_FREQ = I2C_CLK / ((1 + TPR) * (SCL_LP + SCL_HP))
 * rearranged for TPR, with SCL_LP + SCL_HP = 10.
 *
 * Integer division truncates, which biases TPR low, which biases the resulting
 * SCL frequency HIGH.  That is the wrong way to be wrong on a bus with a rated
 * maximum, so the quotient is rounded up here: a slightly slow bus is always
 * legal, a slightly fast one may not be.
 *
 * @param i2c_clk_hz  I2C functional clock after CLKSEL/CLKDIV, in Hz.
 * @param scl_hz      Desired SCL frequency in Hz.  Must be non-zero.
 * @return TPR field value; the caller must range-check against
 *         #LS_I2C_CTPR_TPR_Msk.
 */
static inline uint32_t ls_i2c_tpr_from_clocks(uint32_t i2c_clk_hz, uint32_t scl_hz) {
    const uint32_t divisor = scl_hz * (LS_I2C_SCL_LP + LS_I2C_SCL_HP);
    /* Round the quotient up, then subtract the constant 1 of Equation 27. */
    const uint32_t rounded_up = (i2c_clk_hz + divisor - 1U) / divisor;
    return (rounded_up == 0U) ? 0U : (rounded_up - 1U);
}

/* ------------------------------------------------------------------------
 * CCR — controller configuration — [52] section 25.3.38, p. 1355, Table 25-60
 * ------------------------------------------------------------------------ */

/** Bit 2: enable clock-stretching detection.  Enabling it "ensures compliance
 *  to the I2C standard but could limit the speed due the clock stretching". */
#define LS_I2C_CCR_CLKSTRETCH       (1UL << 2)
/** Bit 1: multi-controller mode. */
#define LS_I2C_CCR_MCTL             (1UL << 1)
/** Bit 0: device active.  "After this bit has been set, it should not be set
 *  again unless it has been cleared by writing a 0 or by a reset". */
#define LS_I2C_CCR_ACTIVE           (1UL << 0)

/* ------------------------------------------------------------------------
 * RIS / ICLR — controller interrupt status — [52] section 25.3.10, p. 1322,
 * Table 25-32 (RIS) and section 25.3.13, p. 1328 (ICLR, same bit layout).
 *
 * These are the flow-control signals a polled driver should use.  CFIFOSR
 * carries the note "this Register should only be read when BUSY is 0"
 * ([52] section 25.3.41), which rules it out for polling mid-burst; RIS carries
 * no such restriction.
 * ------------------------------------------------------------------------ */

/** Bit 0: controller receive done. */
#define LS_I2C_INT_CRXDONE          (1UL << 0)
/** Bit 1: controller transmit done. */
#define LS_I2C_INT_CTXDONE          (1UL << 1)
/** Bit 2: controller RX FIFO reached its trigger level (see CFIFOCTL.RXTRIG). */
#define LS_I2C_INT_CRXFIFOTRG       (1UL << 2)
/** Bit 3: controller TX FIFO reached its trigger level (see CFIFOCTL.TXTRIG). */
#define LS_I2C_INT_CTXFIFOTRG       (1UL << 3)
/** Bit 4: controller RX FIFO full. */
#define LS_I2C_INT_CRXFIFOFULL      (1UL << 4)
/** Bit 5: controller TX FIFO empty. */
#define LS_I2C_INT_CTXEMPTY         (1UL << 5)
/** Bit 7: controller received a NACK from the target. */
#define LS_I2C_INT_CNACK            (1UL << 7)
/** Bit 8: controller START condition detected. */
#define LS_I2C_INT_CSTART           (1UL << 8)
/** Bit 9: controller STOP condition detected. */
#define LS_I2C_INT_CSTOP            (1UL << 9)
/** Bit 10: controller lost arbitration. */
#define LS_I2C_INT_CARBLOST         (1UL << 10)
/** Bit 14/15: SCL low timeout A/B expired. */
#define LS_I2C_INT_TIMEOUTA         (1UL << 14)
#define LS_I2C_INT_TIMEOUTB         (1UL << 15)

/** Controller-side interrupt sources that mean the transfer has failed. */
#define LS_I2C_INT_ANY_ERROR                                                  \
    (LS_I2C_INT_CNACK | LS_I2C_INT_CARBLOST | LS_I2C_INT_TIMEOUTA             \
     | LS_I2C_INT_TIMEOUTB)

/** Every controller-side bit this driver ever inspects, for a clean slate at
 *  the start of a transaction. */
#define LS_I2C_INT_CONTROLLER_ALL                                             \
    (LS_I2C_INT_CRXDONE | LS_I2C_INT_CTXDONE | LS_I2C_INT_CRXFIFOTRG          \
     | LS_I2C_INT_CTXFIFOTRG | LS_I2C_INT_CRXFIFOFULL | LS_I2C_INT_CTXEMPTY   \
     | LS_I2C_INT_CNACK | LS_I2C_INT_CSTART | LS_I2C_INT_CSTOP                \
     | LS_I2C_INT_CARBLOST | LS_I2C_INT_TIMEOUTA | LS_I2C_INT_TIMEOUTB)

/* ------------------------------------------------------------------------
 * CFIFOCTL — controller FIFO control — [52] section 25.3.40, p. 1357,
 * Table 25-62
 * ------------------------------------------------------------------------ */

/** TX FIFO trigger level, bits 2-0.  "0h = Trigger when the TX FIFO is empty",
 *  "7h = Trigger when TX FIFO contains <= 7 byte". */
#define LS_I2C_CFIFOCTL_TXTRIG_Pos  (0U)
#define LS_I2C_CFIFOCTL_TXTRIG_Msk  (0x7UL << LS_I2C_CFIFOCTL_TXTRIG_Pos)
/** Bit 7: flush the TX FIFO while set. */
#define LS_I2C_CFIFOCTL_TXFLUSH     (1UL << 7)
/** RX FIFO trigger level, bits 10-8.  "0h = Trigger when RX FIFO contains
 *  >= 1 byte". */
#define LS_I2C_CFIFOCTL_RXTRIG_Pos  (8U)
#define LS_I2C_CFIFOCTL_RXTRIG_Msk  (0x7UL << LS_I2C_CFIFOCTL_RXTRIG_Pos)
/** Bit 15: flush the RX FIFO while set. */
#define LS_I2C_CFIFOCTL_RXFLUSH     (1UL << 15)

/** The trigger configuration this driver uses: interrupt/flag as soon as a
 *  single byte can be moved in either direction.  Both FIFOs are 8 bytes
 *  ([52] section 25.1.2) and OPTIGA APDUs run to 1557 bytes
 *  (OPTIGA_MAX_COMMS_BUFFER_SIZE, [57]), so the driver streams either way and a
 *  deeper trigger would only add latency without reducing the poll count. */
#define LS_I2C_CFIFOCTL_BYTEWISE                                              \
    ((0UL << LS_I2C_CFIFOCTL_RXTRIG_Pos) | (7UL << LS_I2C_CFIFOCTL_TXTRIG_Pos))

/* ------------------------------------------------------------------------
 * CFIFOSR — controller FIFO status — [52] section 25.3.41, p. 1358, Table 25-63
 *
 * NOTE the register's own caveat: "this Register should only be read when BUSY
 * is 0".  Defined here for completeness and for post-transfer checks; the
 * mid-burst flow control uses RIS instead.
 * ------------------------------------------------------------------------ */

/** Bits 3-0: "Number of Bytes which could be read from the RX FIFO". */
#define LS_I2C_CFIFOSR_RXFIFOCNT_Pos (0U)
#define LS_I2C_CFIFOSR_RXFIFOCNT_Msk (0xFUL << LS_I2C_CFIFOSR_RXFIFOCNT_Pos)
/** Bit 7: RX flush active. */
#define LS_I2C_CFIFOSR_RXFLUSH       (1UL << 7)
/** Bits 11-8: "Number of Bytes which could be put into the TX FIFO"
 *  (i.e. free space, not occupancy).  Reset value 8h. */
#define LS_I2C_CFIFOSR_TXFIFOCNT_Pos (8U)
#define LS_I2C_CFIFOSR_TXFIFOCNT_Msk (0xFUL << LS_I2C_CFIFOSR_TXFIFOCNT_Pos)
/** Bit 15: TX flush active. */
#define LS_I2C_CFIFOSR_TXFLUSH       (1UL << 15)

/* ------------------------------------------------------------------------
 * Register access helpers
 * ------------------------------------------------------------------------ */

/* The cast goes through uintptr_t rather than straight from uint32_t so that
 * these helpers also compile warning-clean in a 64-bit host build (which is how
 * the CI syntax check runs them); on the Cortex-M0+ target the two widths are
 * the same and the cast is a no-op. */

/** Read a 32-bit peripheral register. */
static inline uint32_t ls_reg_read(uint32_t base, uint32_t offset) {
    return *(volatile uint32_t *)(uintptr_t)(base + offset);
}

/** Write a 32-bit peripheral register. */
static inline void ls_reg_write(uint32_t base, uint32_t offset, uint32_t value) {
    *(volatile uint32_t *)(uintptr_t)(base + offset) = value;
}

#ifdef __cplusplus
}
#endif

#endif /* LS_MSPM0_I2C_REGS_H_ */
