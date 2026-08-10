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

---

## Pending intake

These documents are present in `PCB/datasheets/` and are relied on by the
design, but have **not** yet been read into full IEEE entries with verified
section/page citations. Until they are, any claim resting on them must be
marked `UNVERIFIED — needs primary source (see TODO.md)` per `AGENTS.md` §3.
Tracked in [`TODO.md`](TODO.md) §1.

| File | Document | Needed for |
| --- | --- | --- |
| `mspm0g3518-q1.pdf` | TI *MSPM0G351x-Q1 Automotive Mixed-Signal MCUs With CAN-FD Interface*, SLASFA6B | The project MCU (`U1`). **Blocking item:** its pin-multiplexing table (Table 6-2) is what an I²C pin pair for the secure element must be chosen from. |
| `ADM2582E-ADM2587E.pdf` | Analog Devices isolated RS-485 transceiver | `U5` |
| `ADM3055E-ADM3057E.pdf` | Analog Devices isolated CAN-FD transceiver | `U6` |
| `S32K1xx.pdf`, `S32K-RM.pdf` | NXP S32K1xx data sheet and reference manual | Historical only — the S32K144 MCU pass was superseded by the MSPM0G351x. |
| `slaae29a.pdf`, `slaae76e.pdf`, `slaaet8a.pdf`, `slau846e.pdf`, `slaz742g.pdf` | TI application notes / TRM / errata for the MSPM0 family | Firmware and MCU integration |

---

## Removed / superseded citations

| Tag | Document | Disposition |
| --- | --- | --- |
| [2] | Infineon SLB9672 TPM 2.0 | Part removed from the design 2026-08-10, replaced by [45]. Entry retained for design history; tag not reused. |

---

*This file was seeded by Claude Opus 5 (`claude-opus-5`) under human direction,
2026-08-10. Every section/page citation above was read from a local copy of the
document; nothing here is reproduced from model memory.*
