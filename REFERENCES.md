# REFERENCES

IEEE-formatted catalog of every standard, datasheet, and specification cited
anywhere in this repository, per [`AGENTS.md`](AGENTS.md) §1.2 and §2.

**Status: seeded 2026-08-10, INCOMPLETE.** This catalog was created alongside
the SLB9672 → OPTIGA™ Trust M swap, so only the references that change carries
are entered in full. Several documents already present in
[`PCB/datasheets/`](PCB/datasheets/) are cited by the design but not yet
cataloged here — they are listed under "Pending intake" below rather than
omitted, so the gap is visible instead of silent. Do not read an absent entry
as "no source exists"; read it as "not yet verified into this file."

Tag numbering is **shared with the sister project
[Open-Secure-ESC](https://github.com/Stab-Rabbit-coding/Open-Secure-ESC)** where
the two repos cite the same document, so that `[45]` means the same datasheet in
both. Per `AGENTS.md` §2.5 a tag is never renumbered or repurposed.

---

## Cataloged references

**[2]** Infineon Technologies AG, *OPTIGA™ TPM SLB 9672 — TPM 2.0 FW16.xx
Datasheet*, Infineon Technologies AG, Munich, Germany.
Local copy: `PCB/datasheets/Infineon-SLB9672-TPM2.0-SPI-FW16.xx-datasheet.pdf`.
**SUPERSEDED IN THIS DESIGN 2026-08-10.** The SLB9672 was `U7` and has been
removed in favour of the OPTIGA™ Trust M secure element, **[45]** — see
[`PCB/RS485-CANFD-TPM-upgrade.md`](PCB/RS485-CANFD-TPM-upgrade.md) and
[`PCB/OPTIGA-Trust-M-secure-element.md`](PCB/OPTIGA-Trust-M-secure-element.md).
Retained here because the design history cites it and because
`AGENTS.md` §2.5 forbids reusing the tag for anything else.
Section/page: not re-verified this pass — the part is no longer in the design.
Cited in: `README.md`; `PCB/RS485-CANFD-TPM-upgrade.md`;
`PCB/S32K144-MCU-swap.md`; `PCB/OPTIGA-Trust-M-secure-element.md`;
`PCB/kicad/LibreServo-v4.0.0.kicad_sym` (Trust M symbol description, as the
part it replaces).
Date accessed: not re-verified 2026-08-10.

**[45]** Infineon Technologies AG, *OPTIGA™ Trust M — Datasheet*
(SLS 32AIA010MH/S/K/L/M), document release reference Z8F80311641-D, Rev. 3.70,
Infineon Technologies AG, Munich, Germany, 2024-10-09. [Online]. Available:
https://www.infineon.com/assets/row/public/documents/30/49/infineon-optiga-trust-m-datasheet-en.pdf
**URL located but NOT independently confirmed to serve this document** — a live
fetch on 2026-08-09 (recorded in the sister repo) returned HTTP 202 with a
0-byte `text/html` body, i.e. a vendor anti-bot interstitial rather than the
PDF. Marked explicitly per `AGENTS.md` §2.3 rather than asserted.
**Provenance independently corroborated 2026-08-23:** the local copy is
**byte-for-byte identical** (MD5 `5e73fbc0d68206391f5fe19997cd2833`) to
`docs/pdf/OPTIGA_Trust_M_Datasheet_v3.70.pdf` in Infineon's own GitHub
organization repository `Infineon/optiga-trust-m-overview` at commit
`a45b86bda014efeebfb85f084f779cedb07b32dc`, which *was* successfully fetched in
this session — see [53]. The infineon.com URL above is therefore still not
directly confirmed, but the document's authenticity and revision are: an
alternate first-party source now serves the same bytes. Available:
https://github.com/Infineon/optiga-trust-m-overview/blob/main/docs/pdf/OPTIGA_Trust_M_Datasheet_v3.70.pdf
Local verified copy: `PCB/datasheets/infineon-optiga-trust-m-datasheet-en.pdf`
(45 pp.) — `VERIFIED`; every section/page below was read directly from a local
copy of this document.
Section/page, as applied in this repository:

- p. 1, "Features" — Common Criteria EAL6+ (high) certified hardware; PSA
  Level 3; up to 10 kB user memory; PG-USON-10-2,-4 package (3 mm × 3 mm /
  0.118 in × 0.118 in); I²C with Shielded Connection (encrypted communication);
  ECC NIST curves to P-521 and Brainpool r1 to 512; RSA to 2048; AES to 256;
  SHA-256; HMAC to SHA-512; TLS v1.2 PRF and HKDF to SHA-512; 4 monotonic
  counters; hibernate leakage < 2.5 µA.
- p. 7, §1.5 "Device features" — CC certificate BSI-DSZ-CC-0961 (hardware
  identifier IFX_CCI_00000Bh); I²C up to 1 MHz (FM+).
- p. 8, Table 2 "Products for V3" — sales code **SLS 32AIA010ML**, −40 °C to
  +105 °C Extended Temperature Range, PG-USON-10-2,-4 — the variant this design
  selects.
- pp. 8–9, Table 4 — the V3 crypto set (ECC P-256/384/521, Brainpool
  P256/384/512 r1, RSA 1024/2048, AES 128/192/256, HMAC and HKDF
  SHA-256/384/512, TLS v1.2 PRF).
- p. 10, §2 Figure 1 "System block diagram" — on-chip object inventory:
  ~4.5 kB arbitrary data, 4 monotonic counters, 4 X.509 certificate slots,
  3 trust-anchor slots, 4 ECC key slots, 2 RSA key slots, 1 AES key slot,
  1 platform binding secret.
- p. 11, §2 Note — the unique ECC/RSA private key and X.509 certificate are
  generated and provisioned **at Infineon's fab**, with the public key signed
  by a customer-specific CA. This is why the device identity key cannot be
  rotated in the field.
- p. 12, §3 Figure 2 "System integration schematic diagram" — the reference
  circuit this design follows: SCL (pin 8) and SDA (pin 3) each with a 10 kΩ
  pull-up to VCC, 100 nF VCC decoupling, GND pin 1, VCC pin 10. The
  accompanying note states pull-up values depend on the target circuit and I²C
  frequency, so 10 kΩ is a starting point, not a fixed answer.
- p. 15, §4.1 Figure 6 and p. 16, Figure 7 — PG-USON-10-2,-4 package outline:
  3 mm × 3 mm body, 0.5 mm pitch, 0.6 mm max height, 0.05 mm max standoff,
  1.7 × 2.5 mm exposed pad; pin arrangement and the note that the exposed pad
  is internally `n.c.` and exists for thermal dissipation.
- p. 17, Table 6 "Contact definitions and functions of PG-USON-10-2,-4
  packages" — the full 10-pin map used by
  `PCB/kicad/LibreServo-v4.0.0.kicad_sym`, including the requirement that the
  five NC contacts (02, 04, 05, 06, 07) are "Not connected/Do not connect
  externally. Shall be left floating."
- p. 20, Table 11 — VCC 1.62–5.5 V; I_CCAVG typ 14.0 mA during a typical
  authentication profile; sleep typ 70 µA; hibernate < 2.5 µA.
- p. 28, §7.1 Table 19 and §7.2 "Security policy" — the security monitor:
  `t_max` is 5 seconds (± 5%) and **only one protected operation is permitted
  per `t_max` period**. Protected events include "private key use." This is the
  constraint that keeps the secure element out of any control hot path.
- p. 32, Table 23 — `CLK_STRETCHING`, which is why SCL is modeled as
  bidirectional rather than input.
- p. 34, §A.2 — default I²C slave address 0x30.

Cited in: `README.md`; `PCB/OPTIGA-Trust-M-secure-element.md`;
`PCB/RS485-CANFD-TPM-upgrade.md`;
`PCB/kicad/LibreServo-v4.0.0.kicad_sym`;
`PCB/kicad/LibreServo-v4.0.0.kicad_sch` (schematic `U7`, `R30`, `R31`, `C43`);
`PCB/kicad/sym-lib-table`; `PCB/kicad/tools/swap_slb9672_for_optiga.py`;
`TODO.md`.
Date accessed: 2026-08-09 (document), 2026-08-10 (applied here).

**[46]** Texas Instruments Incorporated, *MSPM0G3519-Q1, MSPM0G3518-Q1 Automotive
Mixed-Signal MCUs With CAN-FD Interface*, SLASFA6B, Texas Instruments Incorporated,
Dallas, TX, USA, Nov. 2024, revised Oct. 2025. [Online]. Available:
https://www.ti.com/lit/ds/symlink/mspm0g3519-q1.pdf
`VERIFIED` — read directly from the local copy, 143 pp. One document covers both the
MSPM0G3519-Q1 and MSPM0G3518-Q1 dies (§5).
Local copy: `PCB/datasheets/mspm0g3518-q1.pdf`.
Section/page, as applied in this repository:

- p. 2, Features/Family members — MSPM0G3518-Q1: 256 KB flash, 128 KB RAM.
  MSPM0G3519-Q1: 512 KB flash, 128 KB RAM. Package options include 32-pin VQFN (RHB),
  0.5 mm pitch, wettable flank.
- p. 6, Table 5-1 "Device Comparison Table" — confirms **both** `M0G3518QRHBRQ1`
  (256/128, 5/3/2 UART/I²C/SPI, 32-pin VQFN RHB) and `M0G3519QRHBRQ1` (512/128, same
  package/peripheral counts) are offered, valid, orderable 32-VQFN parts. This directly
  resolves `TODO.md` 3.2: an earlier claim in `PCB/MSPM0G3518-MCU-swap.md` that "the
  '3519 is not offered in RHB-32" was incorrect and has been retracted in that file —
  see its 2026-08-22 correction note.
- pp. 14–33, Table 6-2 "Pin Attributes" (`RHB PIN` column, this design's package) —
  the full per-pin IOMUX function table. Entries used by this design:
  - `PA1` (RHB pin 2, register `PINCM2` @ `0x40428004`): `IOMUX PF3` = `I2C0_SCL`
    (buffer type `IOD`, open-drain digital).
  - `PA8` (RHB pin 12, register `PINCM19` @ `0x40428048`): `IOMUX PF3` = `I2C0_SDA`
    (buffer type `IOD`).
  - Cross-checked: `PA0` (pin 1) also offers `I2C0_SDA` on `PF3`, and `PA2` (pin 6),
    `PA14` (pin 18) — the other two pins freed by the TPM removal — offer **no** I²C
    function on any `PF` (checked exhaustively against every `PF` row for both pins).
  This resolves `TODO.md` 3.1/1.4.a (the blocking item): `SE_I2C_SCL` → `PA1`,
  `SE_I2C_SDA` → `PA8`, both `IOMUX PF3`, both already-spare pins from the TPM removal,
  both on the same I²C instance (`I2C0`). Wired into
  `PCB/kicad/LibreServo-v4.0.0.kicad_sch` (`U1` pins 2 and 12) 2026-08-22 — see
  `TODO.md` 3.1/3.3/4.3 and `PCB/MSPM0G3518-MCU-swap.md` §3.
- p. 58, §7.9.1 "System Oscillator (SYSOSC)", Table — factory-trimmed SYSOSC
  frequency: `SYSOSCCFG.FREQ`=00 (BASE) = 32 MHz, =01 = 4 MHz; user-trimmed 24 MHz
  and 16 MHz options also listed. Basis for the SYSOSC BASE-frequency figure used
  in `TODO.md` 7.3.
- p. 74, §8.1, Figure 8-1 "MSPM0G351x-Q1 Functional Block Diagram" — `I2C0`,
  `I2C1`, `I2C2` are drawn under "PD0 PERIPHERAL BUS (ULPCLK)".
- p. 76, Table 8-1 "Supported Functionality by Operating Mode" — `I2C0/1/2` listed
  under "PD0 Peripherals". Confirms I2C0 (this design's secure-element bus) is a
  PD0 instance, so its `BUSCLK` (per [52] §25.2.1.1) is `ULPCLK`, not `MCLK`
  directly. Basis, with the SYSOSC figure above and [52]'s MCLK/ULPCLK-tree
  citations, for the verified `LS_I2C_FUNCTIONAL_CLK_HZ` value in
  `firmware/pal/ls_board.h` — closes `TODO.md` 7.3.
- p. 84, §8.9 "Flash Memory" — dual-bank flash (up to 256 kB/512 kB total) with a
  separate 16 kB data flash bank and bank-address swap for OTA updates.
- p. 88, §8.18 "Security" — debug security, device identity, AES-128/256 (GCM/GMAC,
  CCM/CBC-MAC, CBC, CTR), flexible firewalls, secure boot, secure firmware update,
  4-key secure key storage, Customer Secure Code (CSC), hardware monotonic counter,
  TRNG, CRC-16/32.
- p. 50, §6.4, Table 6-20 "Connection of Unused Pins" — unused `PAx`/`PBx`/`PCx` pins:
  "Set corresponding pin functions to GPIO (`PINCMx.PF` = 0x1) and configure unused
  pins to output low or input with internal pullup/pulldown resistor." Basis for
  retiring `PA2` (`SPARE_PA2`) as firmware-configured rather than carrying a permanent
  external pull-up — see `TODO.md` 3.3.
- p. 101, §10.1 — points to the *MSPM0 G-Series 80MHz Microcontrollers Technical
  Reference Manual* ([52]) for complete module descriptions.

Cited in: `README.md`; `PCB/MSPM0G3518-MCU-swap.md`; `PCB/ReadMe.md`;
`PCB/kicad/LibreServo-v4.0.0.kicad_sch` (`U1`, symbol description);
`PCB/kicad/LibreServo-v4.0.0-eagle-import.kicad_sym`; `firmware/pal/ls_board.h`;
`TODO.md`.
Date accessed: 2026-08-22 (identity, pins 6.2, security 8.18, unused-pin 6.4);
2026-09-12 (SYSOSC 7.9.1, PD0 assignment 8.1/Table 8-1, added for `TODO.md` 7.3).

**[47]** Analog Devices, Inc., *ADM2582E/ADM2587E: Signal and Power Isolated RS-485
Transceiver With ±15 kV ESD Protection*, Rev. H, Analog Devices, Inc., Wilmington, MA,
USA. [Online]. Available: https://www.analog.com/ADM2587E/datasheet
`VERIFIED` — read directly from the local copy, 22 pp.
Local copy: `PCB/datasheets/ADM2582E-ADM2587E.pdf`.
Section/page, as applied in this repository:

- p. 8, Table 10 "Pin Function Description" (`U5`) — pin 1/3/9/10 `GND1` (logic-side
  ground); pin 2 `VCC` with 0.1 µF + 0.01 µF decoupling between pins 2/1; pin 8 `VCC`
  with 0.1 µF + 10 µF decoupling between pins 8/9; pin 5 `RE` active-low, pin 6 `DE`
  active-high; pins 11 and 14 `GND2` ("recommended to connect Pin 11 and Pin 14
  together through one ferrite bead to PCB ground"); pin 12 `VISOOUT` needing a 10 µF
  reservoir + 0.1 µF decoupling cap between pins 12/11, tied externally to `VISOIN`;
  pin 16 `GND2` ("Ground, Bus Side. **Do not connect this pin to Pin 14 and Pin 11**");
  pin 19 `VISOIN` needing 0.1 µF + 0.01 µF between pins 19/20; pin 20 `GND2` (bus-side
  ground). This confirms silicon pins 11/14/16/20 are not one electrical node — 16 and
  20 are the isolated bus-side ground, 11/14 are the isolated DC-DC converter ground —
  matching the discrepancy already recorded against the schematic symbol in
  `PCB/MSPM0G3507-MCU-swap.md` §7.1.
- Fig. 38 (half-duplex configuration) — `Y`+`A` tied together, `Z`+`B` tied together,
  the configuration this design uses (`U5`).

Cited in: `PCB/MSPM0G3507-MCU-swap.md` §7, §7.1; `PCB/RS485-CANFD-TPM-upgrade.md` §2;
`TODO.md`. Values quoted above match those documents' verbatim quotations from this
datasheet exactly — this entry formalizes citations that were already being applied
correctly but lacked a `REFERENCES.md` entry per `AGENTS.md` §1.2.
Date accessed: 2026-08-22.

**[48]** Analog Devices, Inc., *ADM3055E/ADM3057E: 5 kV rms/3 kV rms, Signal and Power
Isolated, CAN Transceivers for CAN FD*, Rev. D, Analog Devices, Inc., Wilmington, MA,
USA, 2018–2026. [Online]. Available:
https://www.analog.com/media/en/technical-documentation/data-sheets/adm3055e-adm3057e.pdf
`VERIFIED` — read directly from the local copy, 27 pp.
Local copy: `PCB/datasheets/ADM3055E-ADM3057E.pdf`.
Section/page, as applied in this repository:

- p. 15, Table 10 "Pin Function Descriptions" (`U6`) — pins 1/2/10 `GND1` (logic-side
  ground); pin 3 `VCC` (isoPower, 4.5–5.5 V) needing 0.1 µF + 10 µF decoupling; pin 6
  `SILENT` and pin 8 `STBY`, both "bring this input low or leave the pin unconnected
  (internal pull-down) for normal mode" — the basis for tying both to `GND` in this
  design rather than driving them; pin 9 `AUXIN` sets the `AUXOUT` output and is left
  floating/unused in this design (matches `PCB/MSPM0G3507-MCU-swap.md`'s open item on
  the internal pull-down, now confirmed present at the pin description level); pins 11
  and 15 `GND2` (bus-side ground, kept as **separate** pins from `GNDISO` on this part,
  unlike the ADM2587E's collapsed symbol); pin 12 `RS` — short to ground for full-speed
  (non-slope-limited) operation; pins 18 and 20 `GNDISO`, connected together through one
  ferrite bead to PCB ground (bus side); pin 19 `VISOOUT` needing 0.22 µF + 10 µF
  capacitors to `GNDISO`.
- p. 1, Features — ISO 11898-2:2016 compliant, data rates up to 12 Mbps CAN FD, 5 kV
  rms (ADM3055E) / 3 kV rms (ADM3057E) isolation.

Cited in: `PCB/MSPM0G3507-MCU-swap.md` §7, §7.1, §"still open" item on `U6.AUXIN`;
`PCB/RS485-CANFD-TPM-upgrade.md` §3; `PCB/MSPM0G3518-MCU-swap.md`; `TODO.md`. Same
formalization note as [47].
Date accessed: 2026-08-22.

**[49]** Texas Instruments Incorporated, *Cybersecurity Enablers in MSPM0 MCUs*,
SLAAE29A, Texas Instruments Incorporated, Dallas, TX, USA, Jan. 2023, revised
Dec. 2025. [Online]. Available: https://www.ti.com/lit/an/slaae29a/slaae29a.pdf
`VERIFIED` — read directly from the local copy, 44 pp.
Local copy: `PCB/datasheets/slaae29a.pdf`.
Section/page, as applied in this repository (all already quoted in
`PCB/MSPM0G3518-MCU-swap.md`, formalized here per `AGENTS.md` §1.2):

- Table 1-2 — MSPM0G3518/3519 sit in the **`M0Gx5 1x`** security-feature column.
- §2.6/§3.2 — NONMAIN lockdown (CSC/key-region write protection, mass-erase/factory-
  reset policy) is a one-way, permanent-per-unit decision that must be planned before
  first production, not retrofitted.
- §4.5, Fig. 4-1 — KEYSTORE provisioning flow: only the CSC can write keys, and only
  before `INITDONE`; after `INITDONE` the application selects a key slot but cannot
  read/write key material, which travels to the AES engine over a private bus not
  visible to CPU/DMA/debugger. KEYSTORE is wiped on `BOOTRST` and re-provisioned by the
  CSC each cold boot.
- Table 5-3 — AESADV throughput at 80 MHz: 128-bit key 76 cycles / 0.95 µs per block;
  256-bit key 81 cycles / 1.01 µs per block.

Cited in: `PCB/MSPM0G3518-MCU-swap.md` §2, §2.1, §2.2, §2.3; `TODO.md`.
Date accessed: 2026-08-22.

**[50]** Texas Instruments Incorporated, *MSPM0 G-Series MCUs Hardware Development
Guide*, SLAAE76E, Texas Instruments Incorporated, Dallas, TX, USA. [Online]. Available:
https://www.ti.com/lit/an/slaae76e/slaae76e.pdf
`VERIFIED (scope only)` — read directly from the local copy, 36 pp.; confirmed
in-scope for the whole MSPM0 G-series family (power supplies, reset, clocks, debugger
connections, analog peripherals, communication interfaces, GPIO, board layout), which
covers the MSPM0G351x-Q1 used here (§0 abstract, ToC). **No specific section/page of
this document is cited by a design claim in this repository yet** — cataloged as a
general hardware-design reference for the firmware/layout work still open under
`TODO.md` §7, not as support for any specific existing claim.
Local copy: `PCB/datasheets/slaae76e.pdf`.
Cited in: (none yet — reserved for `TODO.md` 7.1/7.2 firmware and layout work).
Date accessed: 2026-08-22.

**[51]** Texas Instruments Incorporated, *EMC Improvement Guide for MSPM0*, SLAAET8A,
Texas Instruments Incorporated, Dallas, TX, USA. [Online]. Available:
https://www.ti.com/lit/an/slaaet8a/slaaet8a.pdf
`VERIFIED (scope only)` — read directly from the local copy, 27 pp.; general EMC
guidance for the MSPM0 family, not device-restricted in its scope statement (§2).
**No specific section/page of this document is cited by a design claim in this
repository yet.**
Local copy: `PCB/datasheets/slaaet8a.pdf`.
Cited in: (none yet — reserved for future EMI/EMC design work; no `TODO.md` item
currently depends on it).
Date accessed: 2026-08-22.

**[52]** Texas Instruments Incorporated, *MSPM0 G-Series 80MHz Microcontrollers
Technical Reference Manual*, SLAU846E, Texas Instruments Incorporated, Dallas, TX, USA,
June 2023, revised July 2026. [Online]. Available:
https://www.ti.com/lit/ug/slau846e/slau846e.pdf
`VERIFIED` — local copy confirmed present and correctly titled/lettered (2521 pp.).
[46] p. 101 §10.1 directs to this document "for complete module descriptions." The
sections below were read directly from the local copy 2026-08-23; the rest of the
document remains unread and no claim rests on it.
Section/page, as applied in this repository:

- **§25.1.2, p. 1283** "I2C Features" — the I2C module supports controller transmit /
  controller receive / target transmit / target receive with 7-bit addressing;
  Standard-mode ≤ 100 kbps, Fast-mode ≤ 400 kbps, Fast-mode Plus ≤ 1 Mbps;
  independent 8-byte transmit and receive FIFOs; glitch suppression; arbitration,
  clock synchronization and multiple-controller support. Basis for running the
  secure-element bus in Fast-mode, matching the conditions [53] §4.4.3 Table 64
  measures its command timings at.
- **§25.2.1, p. 1285, Equation 27** — SCL frequency
  `I2C_FREQ = I2C_CLK / ((1 + TPR) × (SCL_LP + SCL_HP))` with `SCL_LP` fixed at 6 and
  `SCL_HP` fixed at 4; the functional clock source is chosen with `CLKSEL`
  (BUSCLK or MFCLK) and divided by `CLKDIV`. Used to derive the `CTPR` value in
  `firmware/pal/ls_pal_i2c.c`.
- **§25.3, p. 1310, Table 25-21** "I2C Registers" — the register offsets implemented in
  `firmware/pal/mspm0/ls_mspm0_i2c_regs.h`: `PWREN` 0x800, `RSTCTL` 0x804,
  `CLKDIV` 0x1000, `CLKSEL` 0x1004, `GFCTL` 0x1200, `CSA` 0x1210, `CCTR` 0x1214,
  `CSR` 0x1218, `CRXDATA` 0x121C, `CTXDATA` 0x1220, `CTPR` 0x1224, `CCR` 0x1228,
  `CBMON` 0x1234, `CFIFOCTL` 0x1238, `CFIFOSR` 0x123C. (The prose of §25.2 refers to
  the same controller registers by their earlier `MSA`/`MCTR`/`MSR`/`MTPR`/`MCR`
  names; §25.3 is authoritative for this revision and is what the header follows.)
- **§25.3.1, p. 1312, Table 25-23 and §25.3.2, p. 1313, Table 25-24** — `PWREN`
  requires key `0x26` in bits 31–24 to permit a write, with `ENABLE` in bit 0;
  `RSTCTL` requires key `0xB1`, with `RESETASSERT` and `RESETSTKYCLR`.
- **§25.3.32, p. 1348, Table 25-54** `CSA` — `TADDR` in bits 10–1, `DIR` in bit 0
  (0 = transmit, 1 = receive), `CMODE` in bit 15 (0 = 7-bit addressing).
- **§25.3.33, p. 1349, Table 25-55** `CCTR` — `CBLEN` in bits 27–16 (transaction
  length in bytes), `RD_ON_TXEMPTY` bit 5, `CACKOEN` bit 4, `ACK` bit 3, `STOP`
  bit 2, `START` bit 1, `BURSTRUN` bit 0.
- **§25.3.34, p. 1351, Table 25-56** `CSR` — `CBCNT` bits 27–16, `BUSBSY` bit 6,
  `IDLE` bit 5, `ARBLST` bit 4, `DATACK` bit 3 (data-NACK), `ADRACK` bit 2
  (address-NACK), `ERR` bit 1, `BUSY` bit 0.
- **§25.3.37, p. 1354, Table 25-59** `CTPR` — the `TPR` field whose value the
  Equation-27 derivation above produces.
- **§25.3.38, p. 1355, Table 25-60** `CCR` — `CLKSTRETCH` bit 2, `MCTL`
  (multi-controller) bit 1, `ACTIVE` bit 0.
- **§2.3.2.1, p. 213, Table 2-7** "MCLK Source Selection in RUN and SLEEP Mode" and
  **p. 224 note** (§2.3.4 area) — "In all BOOTRST scenarios, MCLK will be sourced
  from SYSOSC at BASE frequency." The reset-default MCLK source and rate.
- **§2.3.2.3, p. 216** "ULPCLK (Low-Power Clock)" — "When MCLK is configured to
  run from SYSOSC or LFCLK, SYSCTL disables UDIV automatically and fULPCLK=fMCLK
  as these clock sources are always <=32MHz." Establishes ULPCLK = MCLK with no
  divider whenever MCLK is SYSOSC-sourced, as it is at reset and as this firmware
  leaves it (no clock-tree reconfiguration exists — `TODO.md` 7.1).
- **§25.2.1.1, p. 1285** "Clock Select and I2C Speed" — "Use I2Cx.CLKSEL register
  to select the source of the I2C functional clock... BUSSCLK: the current bus
  clock is selected as the source for I2C. The current bus clock depends on power
  domain. If the I2C instance is in power domain 1 (PD1) refer to MCLK, if the
  I2C instance is in power domain 0 (PD0) refer to ULPCLK." Combined with [46]
  p. 74/76 (I2C0 is PD0), this is the chain that verifies
  `LS_I2C_FUNCTIONAL_CLK_HZ` = 32 MHz in `firmware/pal/ls_board.h` — closes
  `TODO.md` 7.3.
- **§25.3.6, p. 1317, Table 25-28** `CLKSEL` **[Reset = 00000000h]** — bit 3
  `BUSCLK_SEL`, bit 2 `MFCLK_SEL`, both reset to 0. **Finding:** neither bit is
  set out of reset, so the I2C module selects **no** functional clock source
  until software writes one — the prior assumption in `ls_pal_i2c.c` that
  "reset values" meant BUSCLK was wrong. `pal_i2c_init()` now writes
  `CLKSEL.BUSCLK_SEL` explicitly (2026-09-12 fix, part of closing `TODO.md` 7.3).
- **§25.3.5, p. 1316, Table 25-27** `CLKDIV` **[Reset = 00000000h]** — `RATIO`
  bits 2-0, 0h = divide by 1. Confirms the driver's existing assumption that
  leaving `CLKDIV` at its reset value gives an undivided functional clock is
  correct (unlike `CLKSEL`, above).
- **§1.4.1, p. 20** "Configuration Memory (NONMAIN)" — NONMAIN is a dedicated
  flash region holding BCR/BSL boot configuration; not affected by a mass
  erase, but erased/reset to factory defaults by a factory-reset command.
- **§1.4.2.1, pp. 22–25, Table 1-4 and §§1.4.2.1.1–1.4.2.1.3** "Serial Wire
  Debug Related Policies" — the three generic SWD security levels (0 = no
  restrictions, 1 = custom per-function restrictions, 2 = SW-DP fully
  disabled) and their recommended use; p. 24 explicitly recommends Level 1
  "for most standard production use-cases"; p. 25 warns Level 2 forecloses
  both TI failure analysis and any future SWD access with no recovery path.
- **§1.4.2.3.1–§1.4.2.3.3, pp. 26–27** "Flash Memory Protection and Integrity
  Related Policies" — static write protection of MAIN flash sectors
  (`FLASHSWP0`/`FLASHSWP1` fields) and of the NONMAIN region itself
  (`BOOTCFG4.NONMAINSWP`); p. 27's note: once NONMAIN is statically protected
  and the factory-reset/TI-FA SWD commands are disabled, "the NONMAIN is
  equivalent to immutable read-only memory, and it is no longer possible to
  change the device configuration by any means" — the one-way-door property
  `TODO.md` 4.13 is built around.
- **§1.4.4, p. 30, Table 1-6** "NONMAIN Layout Types" — **MSPM0G351x devices
  use NONMAIN layout Type F** specifically (distinct from Type A and Type E).
  Also notes "The MSPM0-SDK includes a configurator tool to help the user
  configure NONMAIN contents."
- **§1.4.5, p. 31 "NONMAIN_TYPEA Registers" and §1.4.6, p. 60 "NONMAIN_TYPEE
  Registers"** — full register-level appendices are present locally for
  Types A and E **but not for Type F**, confirmed by an exhaustive text
  search of this local copy (the only "Type F" match anywhere in the
  document is the unrelated §1.5.4 FACTORYREGION_TYPEF, p. 141). **This is a
  real, flagged gap**, not a claim resolved by this document: the exact
  Type F field layout for MSPM0G351x-Q1 is UNVERIFIED here — see `TODO.md`
  4.13.
- **§4.1.1–§4.2.1, pp. 465–467** "Secure Boot" / "Customer Secure Code (CSC)"
  — the CSC boot flow (`SYSCTL.SECCFG.SECSTATUS.INITDONE`, set with
  `1 | (0x9D << 24)` per the pseudocode on p. 467) and the security
  configuration actions CSC performs (keystore provisioning, bank-swap and
  SRAM-boundary setup, flash firewall provisioning) before locking itself out
  via `INITDONE`.
- **§4.4.5, p. 470** "Data Bank Protection" — `SYSCTL.SECCFG.FRWPROTDATA`
  configures the first 4 kB of the DATA flash bank at 1 kB sector
  granularity, independently as read-protected, write-protected, both, or
  neither; CPU/DMA/debugger accesses are all treated the same way. **No
  register offset or bit-field table for any `SYSCTL.SECCFG.*` register is
  present in this local copy** (confirmed by checking every page containing
  "SECCFG") — the field name is verified, its address/bit-position is not;
  see `TODO.md` 4.13.
- **§6.3, pp. 548–557** "Flash Controller" — the PROGRAM, ERASE, READVERIFY
  and BLANKVERIFY command sequences. Device-generic (not Type A/E/F
  dependent), unlike the NONMAIN/SECCFG provisioning-policy fields above; the
  basis for the flash read/write implementation still owed by
  `firmware/pal/ls_secure_store.c` per `TODO.md` 4.13.
Local copy: `PCB/datasheets/slau846e.pdf`.
Cited in: `firmware/pal/mspm0/ls_mspm0_i2c_regs.h`; `firmware/pal/ls_pal_i2c.c`;
`firmware/pal/ls_board.h`; `firmware/README.md`; `TODO.md` 4.13, 7.3.
Date accessed: 2026-08-23 (sections through §25.3.38); 2026-08-22 (identity);
2026-09-12 (MCLK/ULPCLK tree §§2.3.2.1/2.3.2.3, CLKSEL/CLKDIV §§25.3.5/25.3.6
for `TODO.md` 7.3; NONMAIN/BCR §§1.4.1–1.4.6, SECURITY §§4.1–4.4.5, Flash
Controller §6.3, for `TODO.md` 4.13).

**[53]** Infineon Technologies AG, *OPTIGA™ Trust M — Solution Reference Manual*,
Rev. 3.70, Infineon Technologies AG, Munich, Germany, 2024-10-09. [Online].
Available (Infineon's own GitHub organization, `Infineon/optiga-trust-m-overview`,
commit `a45b86bda014efeebfb85f084f779cedb07b32dc`, 2025-04-09):
https://github.com/Infineon/optiga-trust-m-overview/blob/main/docs/pdf/OPTIGA_Trust_M_Solution_Reference_Manual_v3.70.pdf
`VERIFIED` — fetched successfully in this session by `git clone` of the Infineon
repository (no anti-bot interstitial on the GitHub path, unlike the infineon.com path
noted under [45]); 127 pp.; every section/page below was read directly from the local
copy. This is the document [45] p. 10 refers to as "available as part of the package"
and whose absence was tracked as `TODO.md` 1.4.f.
Local copy: `PCB/datasheets/OPTIGA_Trust_M_Solution_Reference_Manual_v3.70.pdf`
(MD5 `6dc010a987091c30b9cc0a5ac20a2e85`).
Section/page, as applied in this repository:

- **§2.3.4, p. 20, Figure 12** "Pair OPTIGA™ Trust M with host (pre-shared secret
  based)" — the platform-binding-secret pairing use case; pre-condition "the platform
  binding secret data object is not locked, LcsO must be less than operational";
  post-condition "the pre-shared secret is available and locked (read/write = NEV or
  read = NEV)". Basis of `PCB/servo-bus-security-protocol.md` §4.4's provisioning
  procedure and of `firmware/trust/ls_trust_pairing.c`.
- **§2.3.6, p. 21, Figure 14** "Update platform binding secret during runtime
  (pre-shared secret based)" — runtime PBS rotation over an established Shielded
  Connection; pre-condition that 0xE140's change AC permits `Conf(0xE140)`.
- **§4.4, p. 48, Table 35** — `BASE_ADDR` = 0x30, "I2C base address default". The
  Trust M's 7-bit I²C target address as used by `firmware/pal/ls_pal_ifx_i2c_config.c`.
- **§4.4.3, pp. 70–72, Table 64** "Command performance metrics" — measured at I²C FM
  400 kHz, VCC 3.3 V, 25 °C, **without** Shielded Connection: `CalcSign` ECDSA
  P-256 ~65 ms; `VerifySign` ECDSA P-256 ~85 ms; `CalcSSec` ECDH P-256 ~60 ms;
  `GenKeyPair` ECC P-256 ephemeral ~55 ms; `DeriveKey` TLS v1.2 PRF SHA256 ~50 ms
  (40-byte key, 32-byte secret from session context); `GetDataObject` ~30 ms /
  `SetDataObject` ~55 ms at 256 bytes; `GenKeyPair` RSA-2048 minimum 2900 ms;
  `CalcSign` RSA-2048 ~310 ms. Used to size the boot-time trust sequence budget in
  `PCB/servo-bus-security-protocol.md` §4.7 and `firmware/README.md`.
- **§4.6.1, p. 74, Table 65** "Security events" — the five events the security monitor
  counts: decryption failure, key derivation (on a *persistent* data object), private
  key use, secret key use, suspect system behavior. **Each of the first four carries an
  explicit carve-out for temporary keys held in a session context**, which is the fact
  that makes a per-session ECDHE handshake affordable. Basis of the corrected §4 of
  `PCB/OPTIGA-Trust-M-secure-element.md`.
- **§4.6.2, p. 74** "Security monitor policy" — the permitted usage profile is *one*
  protected operation per `t_max`; a suspect-system-behavior event is never permitted
  and sets SEC to maximum; `t_max` default 5 s (± 5 %). Also defines the SEC credit
  (SEC_CREDIT) mechanism: cleared at power-up; SEC decremented once per event-free
  `t_max` while SEC > 0; SEC_CREDIT incremented once per event-free `t_max` while
  SEC = 0, up to a configured maximum; an event consumes SEC_CREDIT first and only
  increments SEC once SEC_CREDIT is exhausted.
- **§4.6.3, p. 75** "Security monitor configurations" — data object 0xE0C9 holds
  `t_max` (default 5 s; **set to 0 disables the security monitor entirely**; values
  above 5 s are clamped to 5 s internally), SEC_CREDIT_MAX (default 5) and the delayed
  SEC-decrement synchronization count (default/minimum 1).
- **§4.6.4, pp. 76–77, Figure 30** "Throttling down profile" — the delay on protected
  operations **starts only once SEC reaches 128** and rises to `t_max` at SEC = 255.
  This is the citation that corrects this repository's earlier characterization of the
  budget as a flat "one protected operation per 5 s" hard limit.
- **§4.6.4, p. 77, Figure 29** "Power profile" — recommendation not to switch VCC off
  before SEC has returned to 0, and the **200 000-cycle lifetime limit on VCC
  off/on cycling**. A hard constraint on any power-cycling scheme for `U7`.
- **§5.1, pp. 78–79** "Overview data and key store" — NVM endurance: maximum
  **2 million** tearing-safe programming cycles across all objects; each monotonic
  counter up to 600 000 updates; a SEC increment-then-decrement costs two
  tearing-safe cycles; data-retention-after-cycling declines to ½ year beyond about
  40 000 programming cycles of an object. Basis of the PBS-rotation-interval guidance
  in `PCB/servo-bus-security-protocol.md` §4.4.
- **§5.4, pp. 85–87, Table 68 and Table 69** "Common and application-specific objects
  and ACs" — the object identifier map used by `firmware/trust/ls_trust_oid.h`:
  0xE0C2 coprocessor UID; 0xE0C5 SEC; 0xE0C9 security monitor configurations;
  0xE0E0 device certificate (Infineon-issued, type "device identity");
  0xE0E1–0xE0E3 certificates 2–4; 0xE0E8–0xE0E9 root CA trust anchors 1–2;
  0xE0EF trust anchor 8 (platform integrity); 0xE120–0xE123 monotonic counters 1–4;
  0xE140 shared platform binding secret; 0xE0F0 device private ECC key 1
  (read/change = NEV); 0xE0F1–0xE0F3 device private ECC keys 2–4;
  0xE100–0xE103 session contexts 1–4; 0xE200 device symmetric key 1;
  0xF1D0–0xF1DB arbitrary data objects type 3; 0xF1E0–0xF1E1 type 2.
- **§5.6, p. 97, Table 77** "Security monitor configurations" — the byte layout of
  data object 0xE0C9: offset 0 is `t_max` expressed as **milliseconds / 100**
  (default 50, i.e. 5000 ms), offset 2 is SEC_CREDIT_MAX (default 5), offset 3 is the
  delayed SEC-decrement synchronization count (default 1).
- **§5.7, p. 97, Table 79** "Data structure arbitrary data object" — type 2 objects
  are 1500 bytes, type 3 objects are 140 bytes.
- **§6.5.8, p. 107** "Shielded connection" (security guidance) — "the recommended
  length of platform binding shared secret is **32 bytes or more**"; the security level
  of the shielded connection "is as high as a typical microcontroller/host side hardware
  security level"; runtime PBS update is recommended, with the NVM-endurance caveat.
- **§6.6, p. 107; §6.6.1 "Setup" and §6.6.2 "Usage", p. 108** — Shielded Connection V1
  setup and usage: pairing is the precondition; `pal_os_datastore_read` /
  `pal_os_datastore_write` are the host-side abstraction for the PBS and must be
  adapted per platform; the feature is compiled in with `OPTIGA_COMMS_SHIELDED_CONNECTION`
  and defaulted with `OPTIGA_COMMS_DEFAULT_PROTECTION_LEVEL`; per-call protection is
  raised with `OPTIGA_UTIL_SET_COMMS_PROTECTION_LEVEL` /
  `OPTIGA_CRYPT_SET_COMMS_PROTECTION_LEVEL`, and the setting reverts to the default
  after one invocation.
- **§6.6.3, pp. 108–110** "Host authenticates OPTIGA™ Trust M" — the Shielded
  Connection alone carries no host-supplied nonce, so it provides no freshness on its
  own; freshness requires an explicit nonce exchange, which this document specifies in
  three variants (write/read nonce to a data object; derive keys using a nonce; derive
  keys using a nonce plus a static additional pre-shared secret).
Cited in: `PCB/servo-bus-security-protocol.md`;
`PCB/OPTIGA-Trust-M-secure-element.md`; `firmware/README.md`;
`firmware/trust/ls_trust_oid.h`; `firmware/trust/ls_trust.c`;
`firmware/trust/ls_trust_pairing.c`; `firmware/config/optiga_lib_config.h`;
`firmware/pal/ls_pal_ifx_i2c_config.c`.
Date accessed: 2026-08-23.

**[54]** Infineon Technologies AG, *IFX I2C Protocol — Protocol Specification*,
Rev. 2.03, Infineon Technologies AG, Munich, Germany, 2020-09-22. [Online].
Available (Infineon's own GitHub organization, `Infineon/optiga-trust-m-overview`,
commit `a45b86bda014efeebfb85f084f779cedb07b32dc`):
https://github.com/Infineon/optiga-trust-m-overview/blob/main/docs/pdf/Infineon_I2C_Protocol_v2.03.pdf
`VERIFIED (identity only)` — fetched in this session and confirmed present, correctly
titled and revisioned (44 pp.). This is the specification [53] §6.6 defers to for the
internal details of the Shielded Connection (presentation layer: negotiation and the
protection algorithms). **No specific section/page of this document is cited by a
design claim in this repository yet** — the host library implements this protocol layer
and this design does not re-derive it. Flagged `UNVERIFIED — needs primary source (see
TODO.md)` for any future claim that would rely on a specific section, per `AGENTS.md` §3.
Local copy: `PCB/datasheets/Infineon_I2C_Protocol_v2.03.pdf`
(MD5 `1e013799a8631279d1964433631d34cb`).
Cited in: `PCB/servo-bus-security-protocol.md` (as the deferred-to specification for
Shielded Connection internals, not for a specific numeric claim).
Date accessed: 2026-08-23.

**[55]** Infineon Technologies AG, *OPTIGA™ Trust M — Keys and Certificates*
(SLS 32AIA010MK/L, Product Version V3), Rev. 3.10, Infineon Technologies AG, Munich,
Germany, 2020-10-01. [Online]. Available (Infineon's own GitHub organization,
`Infineon/optiga-trust-m-overview`, commit
`a45b86bda014efeebfb85f084f779cedb07b32dc`):
https://github.com/Infineon/optiga-trust-m-overview/blob/main/docs/pdf/OPTIGA_Trust_M_Keys_And_Certificates_v3.10.pdf
`VERIFIED (identity only)` — fetched in this session, 12 pp., title page confirms
"SLS 32AIA010MK/L … Product Version: V3", which covers the sales code on this
design's BOM (SLS 32AIA010ML, [45] p. 8 Table 2). Describes the default PKI
infrastructure and what is stored on-chip as private keys and certificates. **No
specific section/page is cited by a design claim in this repository yet**; the
default-PKI facts this design currently relies on are taken from [56] Table 1, which
states them in the comparison form the design needed.
Local copy: `PCB/datasheets/OPTIGA_Trust_M_Keys_And_Certificates_v3.10.pdf`
(MD5 `f733902beb200e17b148e06ab7525860`).
Cited in: (none yet — see `TODO.md` 4.8, which will need it for the fleet PKI design).
Date accessed: 2026-08-23.

**[56]** Infineon Technologies AG, *OPTIGA™ Trust M configurations — Configuration
Guide* (SLS 32AIA010MH/S/K/M), Rev. 2.2, Infineon Technologies AG, Munich, Germany,
2024-01-17. [Online]. Available (Infineon's own GitHub organization,
`Infineon/optiga-trust-m-overview`, commit
`a45b86bda014efeebfb85f084f779cedb07b32dc`):
https://github.com/Infineon/optiga-trust-m-overview/blob/main/docs/pdf/OPTIGA_Trust_M_ConfigGuide_v2.2.pdf
`VERIFIED` — fetched in this session, 14 pp.; the sections below were read directly
from the local copy.
**Scope caveat, recorded rather than glossed:** this document's title page enumerates
sales codes **SLS 32AIA010MH/S/K/M** and does **not** list the **/L** code that is on
this design's BOM ([45] p. 8 Table 2, SLS 32AIA010ML). Its subject is the *provisioning
variant* (V1 / V3 / Express / MTR), which is orthogonal to the temperature/packing
suffix, and [55]'s title page does carry **/L** for the same V3 product version — so
the V3 column below is taken as applying to SLS 32AIA010ML. This inference is recorded
here explicitly because it *is* an inference; see `TODO.md` 4.11.
Section/page, as applied in this repository:

- **§1, p. 4** — V1 and V3 ship in a "standard configuration": default data per [53]
  and a default PKI, with an ECC NIST P-256 end-device certificate in 0xE0E0 and its
  private key in 0xE0F0. Express and MTR are the same silicon as V3, provisioned
  additionally for cloud onboarding (CIRRENT™ Cloud ID) and Matter respectively.
- **§2, pp. 5–7, Table 1** "Comparison of OPTIGA™ Trust M configurations", row
  **0xE140 — Platform binding secret**: on **V3** the object ships with life-cycle
  state **Creation**, value **Default**, read AC **ALW** and change AC
  `LcsO < operational || Conf(0xE140)`. On Express and MTR it instead ships
  Operational, with a chip-unique value and read AC **NEV**. **This is the citation
  that makes host pairing mandatory rather than optional on this design**: an
  unpaired V3 part's platform binding secret is a published default value that any
  bus observer can read, so the Shielded Connection keyed from it protects nothing
  until the pairing step of [53] §2.3.4 has been executed and 0xE140 locked.
- **§2, p. 7** — definitions of the access-condition terms used in Table 1: `ALW`,
  `NEV`, `LcsO(X)`, `Auto(X)`, and `Conf(X)` — "the action is only possible in case
  the data involved … are confidentiality protected with key given by X. This enforces
  the shielded connection during the operations".
- **§2, pp. 5–7, Table 1**, row **0xF1D0 — Arbitrary data** — on V3 the object type is
  "Not configured" (it is configured as `AUTOREF` only on Express/MTR), i.e. it is
  available to this design as a general-purpose 140-byte type-3 object ([53] §5.7).
Cited in: `PCB/servo-bus-security-protocol.md` §4.4; `firmware/README.md`;
`firmware/trust/ls_trust_pairing.c`.
Date accessed: 2026-08-23.

**[57]** Infineon Technologies AG, *OPTIGA™ Trust M Host Library for C*, version
"Ver 5.8.1" (`include/optiga_lib_version.h`), source repository at commit
`67cfd0e589fc09f936d4e4ce4fa35eacc43af72f` (2026-08-13), MIT License
(SPDX-License-Identifier: MIT; Copyright © 2018–2024 Infineon Technologies AG).
[Online]. Available: https://github.com/Infineon/optiga-trust-m
`VERIFIED` — cloned in this session; the API signatures, PAL contract, configuration
macros and OID/metadata encodings cited below were read directly from the cloned
source, not from documentation or from model memory.
Section/file, as applied in this repository:

- `include/pal/pal.h`, `pal_i2c.h`, `pal_gpio.h`, `pal_os_event.h`, `pal_os_lock.h`,
  `pal_os_memory.h`, `pal_os_timer.h`, `pal_os_datastore.h`, `pal_logger.h`,
  `pal_ifx_i2c_config.h` — the platform abstraction layer contract that
  `firmware/pal/` implements for the MSPM0G3518-Q1. Notably
  `pal_os_datastore.h` defines `OPTIGA_PLATFORM_BINDING_SHARED_SECRET_ID` (0x11),
  `OPTIGA_COMMS_MANAGE_CONTEXT_ID` (0x22), `OPTIGA_HIBERNATE_CONTEXT_ID` (0x33),
  `OPTIGA_SHARED_SECRET_MAX_LENGTH` (0x40 = 64) and `APP_CONTEXT_SIZE` (0x08).
- `include/optiga_lib_config_m_v3.h` — the upstream V3 feature-macro set that
  `firmware/config/optiga_lib_config.h` derives from and narrows;
  source of `OPTIGA_MAX_COMMS_BUFFER_SIZE` (0x615 = 1557),
  `OPTIGA_CMD_MAX_REGISTRATIONS` (0x06) and the reset-type encoding
  (0 = cold, 1 = soft, 2 = warm).
- `include/optiga_util.h`, `include/optiga_crypt.h` — service-layer API signatures used
  by `firmware/trust/`: `optiga_util_create`/`_destroy`/`_open_application`/
  `_close_application`/`_read_data`/`_write_data`/`_read_metadata`/`_write_metadata`,
  and `optiga_crypt_create`/`_destroy`/`_random`/`_hash`/`_ecc_generate_keypair`/
  `_ecdsa_sign`/`_ecdsa_verify`/`_ecdh`/`_tls_prf_sha256`.
- `examples/optiga/usecases/example_pair_host_and_optiga_using_pre_shared_secret.c`
  — the reference implementation of [53] §2.3.4's pairing sequence, including the
  0xE140 final-metadata TLV block (tags 0xC0 LcsO, 0xD0 change, 0xD1 read, 0xD3
  execute, 0xE8 data-object type 0x22 "platform binding secret") that
  `firmware/trust/ls_trust_pairing.c` reproduces with this design's own AC choice.
Local copy: not vendored. `firmware/tools/fetch_optiga_host_library.sh` fetches this
exact commit; see `firmware/README.md` §"Upstream dependency and attribution" for the
attribution chain required by `AGENTS.md` and by the MIT license terms.
Cited in: `firmware/README.md`; `firmware/config/optiga_lib_config.h`;
`firmware/pal/*.c`; `firmware/trust/*.c`; `firmware/trust/*.h`.
Date accessed: 2026-08-23.

**[58]** T. Dierks and E. Rescorla, *The Transport Layer Security (TLS) Protocol
Version 1.2*, RFC 5246, Internet Engineering Task Force, August 2008. [Online].
Available: https://www.rfc-editor.org/rfc/rfc5246.txt
`VERIFIED` — fetched successfully in this session (HTTP 200, 222 395 bytes) and the
section below was read directly from the retrieved text.
Section/page, as applied in this repository:

- **§5 "HMAC and the Pseudorandom Function", pp. 14–15** — the TLS 1.2 PRF:
  `P_hash(secret, seed) = HMAC_hash(secret, A(1) + seed) + HMAC_hash(secret, A(2) + seed) + ...`
  with `A(0) = seed` and `A(i) = HMAC_hash(secret, A(i-1))`, and
  `PRF(secret, label, seed) = P_<hash>(secret, label + seed)`. Also the requirement
  that the label "should be included in the exact form it is given without a length
  byte or trailing null character". Implemented as `pal_crypt_tls_prf_sha256` in
  `firmware/pal/ls_pal_crypt.c`, which the Shielded Connection of [53]/[54]
  requires the host to supply.
Local copy: not retained. Unlike the vendor datasheets in `PCB/datasheets/`, which
sit behind anti-bot interstitials (see the note on [45]), the IETF and NIST URLs
above are stable, public and were fetched successfully in this session; a local
mirror would add a second thing to keep in sync without adding assurance.
Cited in: `firmware/pal/ls_pal_crypt.c`; `firmware/pal/ls_crypto_backend.h`;
`firmware/tests/README.md`; `firmware/README.md`;
`PCB/servo-bus-security-protocol.md` §4.4.4.
Date accessed: 2026-08-23.

**[59]** M. Dworkin, *Recommendation for Block Cipher Modes of Operation: The CCM
Mode for Authentication and Confidentiality*, NIST Special Publication 800-38C,
National Institute of Standards and Technology, Gaithersburg, MD, USA, May 2004
(errata update 2007-07-20). [Online]. Available:
https://nvlpubs.nist.gov/nistpubs/Legacy/SP/nistspecialpublication800-38c.pdf
`VERIFIED` — fetched successfully in this session (HTTP 200, 287 840 bytes) and the
sections below were read directly from the retrieved PDF. The date and errata
status above are the document's own title page, read from the retrieved copy.
Section/page, as applied in this repository:

- **§6.1 "Generation-Encryption Process"** — the eight-step specification:
  format `(N, A, P)` into `B0…Br`; `Y0 = CIPH_K(B0)`; `Yi = CIPH_K(Bi XOR Yi-1)`;
  `T = MSB_Tlen(Yr)`; generate counter blocks `Ctr0…Ctrm`; `Sj = CIPH_K(Ctrj)`;
  `C = (P XOR MSB_Plen(S)) || (T XOR MSB_Tlen(S0))`.
- **§6.2 "Decryption-Verification Process"** — including step 1, "If Clen ≤ Tlen,
  then return INVALID", which is why `pal_crypt_decrypt_aes128_ccm` rejects a
  ciphertext with an empty payload, and step 10's MAC comparison.
- **Appendix A.1 "Length Requirements"** — `t` is an element of
  {4, 6, 8, 10, 12, 14, 16}; `q` of {2, …, 8}; `n` of {7, …, 13}; `n + q = 15`;
  `a < 2^64`. These are the parameter checks in `ls_ccm_check_parameters`.
- **Appendix A.2.1, Tables 1 and 2** — the `B0` flags octet (bit 6 `Adata`,
  bits 5–3 `(t-2)/2`, bits 2–0 `q-1`) and the nonce/`Q` layout of the remaining
  15 octets.
- **Appendix A.2.2** — associated-data length encoding: `[a]16` (two octets) for
  `0 < a < 2^16 - 2^8`, `0xff || 0xfe || [a]32` and `0xff || 0xff || [a]64` for the
  larger cases. Only the two-octet case is implemented, and larger values are
  rejected rather than mis-encoded.
- **Appendix A.2.3** — payload blocks zero-padded to a 16-octet boundary.
- **Appendix A.3, Tables 3 and 4** — counter-block format, with bits 7–3 of the
  flags octet zero "to ensure that all the counter blocks are distinct from B0".
- **Appendix C, examples C.1–C.3** — the three worked vectors, which
  `firmware/tests/generate_vectors.py` reproduces and
  `firmware/tests/test_pal_crypt_ccm.c` checks.
Cited in: `firmware/pal/ls_pal_crypt.c`; `firmware/pal/ls_crypto_backend.h`;
`firmware/tests/README.md`; `firmware/tests/generate_vectors.py`;
`firmware/README.md`; `PCB/servo-bus-security-protocol.md` §4.4.4.
Date accessed: 2026-08-23.

**[60]** Winsok Semiconductor Co., Ltd., *WSD3069DN56: N-Ch and P-Channel MOSFET*,
Rev. 2, Winsok Semiconductor Co., Ltd., Shenzhen, China, Apr. 2019. [Online].
Available: https://www.winsok.tw (product page for WSD3069DN56; direct PDF URL not
captured — the copy below was provided directly by the project owner, not fetched
by an agent in this session).
`VERIFIED` — read directly from the supplied copy, 5 pp. Manufacturer identity,
part number, and revision/date all confirmed from the document's own header/footer
on every page ("WiNSOK SEMICONDUCTOR", "WSD3069DN56", "Rev 2: Apr.2019",
"www.winsok.tw"); internally consistent across the Product Summary table,
per-channel Absolute Maximum Ratings, and per-channel Electrical Characteristics
tables — not a claim taken from a single isolated figure.
Local copy: `PCB/datasheets/WSD3069DN56.pdf` (MD5 `2caafbfdf2a36473c6a0e67deb91fb51`).
Section/page, as applied in this repository:

- p. 1, "Product Summery" [*sic*, manufacturer's own spelling] table and
  N-Channel "Absolute Maximum Ratings" table — `BVDSS` 30 V (N) / −30 V (P);
  `RDSON` 15 mΩ; `ID` (continuous drain current, `TC` = 25°C) **16 A** (N) /
  −16 A (P) — exactly the figure `README.md` cites and the BOM
  (`PCB/LibreServo-v2.3_BOM.txt` lines 31–32, parts `M2`/`M3`) already carried.
  Derates to 10.5 A (N) / −12.5 A (P) at `TC` = 100°C — not previously stated
  anywhere in this repo, added as a caveat alongside the headline figure.
- p. 1, "DFN5X6C-8 Pin Configuration" — confirms the DFN5X6, 8-pin package the BOM
  and `PCB/kicad/LibreServo-v4.0.0.pretty/` footprint library assume, with two
  independent MOSFETs (`D1`/`G1`/`S1` = N-channel, `D2`/`G2`/`S2` = P-channel) in
  one package, matching the BOM's "1PCS N-Channel+1PCS P-Channel" description.
- p. 2, N-Channel "Electrical Characteristics" table — `VGS(th)` (gate threshold)
  1.5 V typ. at `IDS` = 250 µA — matches the BOM's "1.5V@250uA" figure exactly;
  `RDS(ON)` 15 mΩ typ. / 19.5 mΩ max at `VGS` = 10 V, `IDS` = 10 A — matches the
  BOM's "15mΩ@10V,10A" figure exactly.
Cited in: `README.md`.
Date accessed: 2026-09-13 (document supplied to this session on this date;
manufacturer's own product page not independently re-fetched, per the network
constraint recorded in this session — see `TODO.md` 1.4.e).

**[61]** Broadcom Inc., *AEAT-8800-Q24: Magnetic Encoder IC, 10- to 16-Bit
Programmable Angular Magnetic Encoder*, pub-005892, Broadcom Inc., San Jose, CA,
USA, May 17, 2017 (copyright 2016–2017). [Online]. Available:
https://docs.broadcom.com/docs/pub-005892 (URL located by this session via web
search; not independently re-fetched — the copy below was provided directly by
the project owner, not fetched by an agent in this session).
`VERIFIED` — read directly from the supplied copy, 25 pp. Manufacturer identity,
part number, publication number, and date all confirmed from the document's own
cover page ("AEAT-8800-Q24", "BROADCOM", "Data Sheet") and its closing page
("pub-005892 – May 17, 2017", "Copyright © 2016–2017 by Broadcom") — the same
publication number ("pub-005892") independently located via this session's own
web search for the part, corroborating provenance.
Local copy: `PCB/datasheets/AEAT-8800-Q24.pdf` (MD5 `0572d15797af9d171f208c9bc8547093`).
Section/page, as applied in this repository:

- p. 1, "Description" and "Key Features" — "provides accurate angular
  measurement over a full 360 degrees of rotation"; "Selectable 10, 12, 14, or
  16 bits of absolute resolution" — the basis for the README's "16 bits of
  resolution! 360 degrees" claim. **16 bits is the maximum of a selectable,
  one-time-programmable range (10/12/14/16), not the part's only or fixed
  resolution** — this repo's claim happens to describe the top of that range
  correctly but does not itself state that it's selectable; noted here rather
  than silently left implicit.
- p. 4, "Pin Assignment" / Figure 4 and "Pinout Description" — QFN-24 package
  (5 mm × 5 mm per p. 1 Key Features), matching the BOM's `PCB/LibreServo-v2.3_BOM.txt`
  line 67 (`U10`, part `AEAT-8800-Q24`, package `QFN24`) exactly, including the
  three-wire SSI absolute interface (`SSI_SCL_SPI_CLK`, `SSI_NSL_SPI_DI`,
  `SSI_DO_SPI_DO`) and incremental `A`/`B`/`I` outputs referenced in
  `PCB/RS485-CANFD-TPM-upgrade.md`'s note on the encoder's separate SSI bus.
- p. 5, "Recommended Operating Conditions" — supply voltage 4.5–5.5 V (5 V
  operation) or 3.0–3.6 V (3.3 V operation); incremental output frequency up to
  1.0 MHz — corroborates `PCB/RS485-CANFD-TPM-upgrade.md`'s characterization of
  the encoder's SSI link as "1–1.2 MHz" (this datasheet's ceiling is the
  incremental-output frequency, a related but distinct figure from the SSI
  clock rate; not fully reconciled, noted rather than asserted as identical).
Cited in: `README.md`.
Date accessed: 2026-09-13 (document supplied to this session on this date; the
publication URL was located by web search the same session but not itself
fetched).

**[62]** Allegro MicroSystems, Inc., *ACS711: Hall-Effect Linear Current Sensor
with Overcurrent Fault Output for <100 V Isolation Applications*, ACS711A-DS,
Rev. 9, Allegro MicroSystems, Inc., Manchester, NH, USA, Jan. 24, 2025. [Online].
Available:
https://www.allegromicro.com/~/media/files/datasheets/acs711-datasheet.ashx
(URL located by this session via web search; not independently re-fetched — the
copy below was provided directly by the project owner, not fetched by an agent
in this session).
`VERIFIED` — read directly from the supplied copy, 22 pp. Manufacturer identity,
part number, and revision/date all confirmed from the document's own header
("ALLEGRO microsystems", "ACS711") and footer ("ACS711A-DS, Rev. 9",
"January 24, 2025", "MCO-0000224", Allegro's Manchester, NH address) on every
page.
Local copy: `PCB/datasheets/ACS711-Datasheet.pdf` (MD5 `cd4a0a48a0ebbd086b7bde10410b3055`).
Section/page, as applied in this repository:

- p. 2, "SELECTION GUIDE" — the BOM's exact part, `ACS711KEXLT-15AB-T`
  (`PCB/LibreServo-v2.3_BOM.txt` line 65, `U4`), has Optimized Accuracy Range
  `I_P` = **±15.5 A**, sensitivity 90 mV/A, 12-contact QFN package with fused
  current loop. **Finding: README.md's "±15A ACS711" was never an exact figure
  this part offers** — the datasheet's actual full-scale sensing ranges are
  ±12.5 A / ±25 A (LC package) and ±15.5 A / ±31 A (EX package); the BOM itself
  already carried the correct ±15.5 A figure, so this was purely a README
  rounding that had drifted from its own BOM. Corrected in place.
- p. 1, "FEATURES AND BENEFITS" / "DESCRIPTION" — Hall-effect linear current
  sensor, no external sense resistor, <100 V isolation, output voltage
  proportional to AC or DC current, 100 kHz bandwidth, ratiometric output.
  General confirmation of the part's function as used in this design.
- p. 3, "PINOUT DIAGRAMS" (EX package) and "TERMINAL LIST TABLE" — matches the
  BOM's `ACS711-QFN` footprint reference and package description
  (`QFN-12(3x3)`).
Cited in: `README.md`.
Date accessed: 2026-09-13 (document supplied to this session on this date; the
publication URL was located by web search the same session but not itself
fetched).

Cross-repository tag note: tags **[46]–[62]** were assigned sequentially after the
highest tag already in use in this file ([45]) because this session has no access to
the sister repository (`Open-Secure-ESC`) to confirm whether it already cites these
same documents under different tag numbers. Per `AGENTS.md` §2.5 an existing tag is
never renumbered, so if a future pass finds a collision or an existing sister-repo tag
for one of these documents, that document's citations must be **migrated to the
sister-repo tag** (with a superseded-tag note left here), not the other way around.

---

## Pending intake

These documents are present in `PCB/datasheets/` but are either historical-only or
not yet read into full citations.

| File | Document | Status |
| --- | --- | --- |
| `S32K1xx.pdf`, `S32K-RM.pdf` | NXP S32K1xx data sheet and reference manual | Historical only — the S32K144 MCU pass was superseded by the MSPM0G351x-Q1. No current design claim depends on these; not cataloged with a tag unless a future claim needs one. |
| `Infineon-SLB9672-TPM2.0-SPI-FW16.xx-datasheet.pdf` | Infineon SLB9672 TPM 2.0 datasheet, tag [2] | **Deleted from the working tree** (the part left the design 2026-08-10; see the [2] entry above and `PCB/OPTIGA-Trust-M-secure-element.md`). The deletion is staged but the [2] entry is retained for design history per `AGENTS.md` §2.5. |

These documents are **not** in `PCB/datasheets/` and are known gaps:

| Document | Why it is wanted | Tracked as |
| --- | --- | --- |
| Texas Instruments errata for MSPM0G3518-Q1 / MSPM0G3519-Q1 specifically | SLAZ742G covers a different die — see "Considered and found not applicable" below | `TODO.md` 1.4.d |
| Infineon, *OPTIGA™ Trust M Release Notes*, v3.02 | Present in `Infineon/optiga-trust-m-overview` `docs/pdf/` but deliberately not intaken in the 2026-08-23 pass — no current design claim depends on a release-note item. Fetch it before relying on any firmware-revision-specific behaviour of `U7`. | `TODO.md` 4.12 |
| Infineon, *OPTIGA™ Trust M Host Library Documentation* (`.chm`) | The Windows-help form of the [57] API reference; the header comments in the cloned source were sufficient and were used instead | (not tracked — [57] source supersedes it) |

---

## Considered and found not applicable

| Document | Finding |
| --- | --- |
| Texas Instruments, *Errata: MSPM0G3x0x, MSPM0G1x0x, MSPM0G3x0x-Q1 Microcontrollers*, SLAZ742G (local copy `PCB/datasheets/slaz742g.pdf`) | **Does not apply to this design.** Checked 2026-08-22: SLAZ742G's own title and Table 1-1 device-revision scope cover the **MSPM0G3x0x / G1x0x / G3x0x-Q1** die family (e.g. MSPM0G3505/3506/3507) — the die used in the *superseded* `PCB/MSPM0G3507-MCU-swap.md` pass. The current MCU is the **MSPM0G351x-Q1** die (`M0G3518QRHBRQ1`), a different TI die covered by its own datasheet ([46], SLASFA6B) with its own (unread, in this pass) errata document. Citing SLAZ742G against the current design would misattribute its advisories to the wrong silicon; it is recorded here specifically so that mistake is not made. No SLAZ742G-derived claim exists anywhere in this repository. **Follow-up needed:** locate and intake the correct errata document for MSPM0G351x-Q1 (not yet present in `PCB/datasheets/`) before relying on any MSPM0G3518-Q1 errata claim. |

---

## Removed / superseded citations

| Tag | Document | Disposition |
| --- | --- | --- |
| [2] | Infineon SLB9672 TPM 2.0 | Part removed from the design 2026-08-10, replaced by [45]. Entry retained for design history; tag not reused. |

---

*This file was seeded by Claude Opus 5 (`claude-opus-5`) under human direction,
2026-08-10; extended by Claude Sonnet 5 (`claude-sonnet-5`) under human direction,
2026-08-22 (entries [46]–[52] and the SLAZ742G applicability finding); and extended
again by Claude Opus 5 (`claude-opus-5`) under human direction, 2026-08-23 (entries
[53]–[59], the SLAU846E §25 section citations added to [52], and the MD5-identity
corroboration added to [45]). Every section/page citation above was read from a local
copy of the document; nothing here is reproduced from model memory.*
