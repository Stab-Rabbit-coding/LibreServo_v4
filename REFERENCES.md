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
`PCB/kicad/LibreServo-v4.0.0-eagle-import.kicad_sym`; `TODO.md`.
Date accessed: 2026-08-22.

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
`VERIFIED (identity only)` — local copy confirmed present and correctly titled/lettered
(2521 pp.); not read section-by-section in this pass. [46] p. 101 §10.1 directs to this
document "for complete module descriptions." **No specific section/page is cited by a
design claim in this repository yet** — flagged `UNVERIFIED — needs primary source (see
TODO.md)` for any future claim that would rely on a specific section of it, per
`AGENTS.md` §3, until such a section is read and cited here.
Local copy: `PCB/datasheets/slau846e.pdf`.
Cited in: (none yet).
Date accessed: 2026-08-22.

Cross-repository tag note: tags **[46]–[52]** were assigned sequentially after the
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
2026-08-10, and extended by Claude Sonnet 5 (`claude-sonnet-5`) under human direction,
2026-08-22 (entries [46]–[52] and the SLAZ742G applicability finding). Every
section/page citation above was read from a local copy of the document; nothing here
is reproduced from model memory.*
