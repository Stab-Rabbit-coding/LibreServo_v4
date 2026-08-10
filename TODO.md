# TODO — LibreServo v4.0.0 (Work Breakdown Structure)

Formal WBS per the project governance in [`AGENTS.md`](AGENTS.md).

Legend: `[ ]` open · `[~]` in progress / partially resolved · `[x]` closed

---

## 1. Governance & Documentation

- [x] 1.1 Adopt `AGENTS.md` (2026-08-10, from the sister Open-Secure-ESC
      project — see the provenance note at the top of that file).
- [x] 1.2 Create `CLAUDE.md` stub directing AI agents to `AGENTS.md`.
- [x] 1.3 Create `REFERENCES.md`.
- [~] 1.4 **Complete the `REFERENCES.md` intake.** The catalog is seeded, not
      finished: only [2] and [45] carry full entries. Every document under
      "Pending intake" in that file needs an IEEE entry with a validated URL
      (or an explicit note on why the URL is unverifiable) and specific
      section/page citations. Until then, claims resting on them must be
      marked `UNVERIFIED` per `AGENTS.md` §3.
  - [ ] 1.4.a **(Blocking, see 3.1)** TI MSPM0G351x-Q1 SLASFA6B — needed for
        the MCU pin map generally and the I²C pin pair specifically.
  - [ ] 1.4.b ADM2582E/ADM2587E (`U5`), ADM3055E/ADM3057E (`U6`).
  - [ ] 1.4.c The five TI MSPM0 application notes / TRM / errata.
- [x] 1.5 Create `PROJECT_INDEX.md`.
- [ ] 1.6 Audit the repo for claims that cite no source. The upstream README
      and the `PCB/*-swap.md` notes predate `AGENTS.md` and were not written to
      its evidentiary standard.

## 2. Repository / Naming

- [x] 2.1 Rename the repository and working folder `LibreServo_v2-Sec` →
      `LibreServo_v4`, matching the `origin` rename, and update the `origin`
      remote URL (2026-08-10). The `upstream` remote still points at
      `Luisonson/LibreServo_v2` and must **not** be retargeted — that is the
      genuine fork source.

## 3. Hardware — MCU Subsystem

- [~] 3.1 **Resolve the MSPM0G3519 pin map, and specifically an I²C-capable
      pin pair for the secure element.** This is the one item blocking a
      complete root-of-trust wiring. The datasheet's pin-attribute /
      multiplexing table (Table 6-2) does not survive PDF text extraction with
      its column structure intact, so no PAx↔I2Cn_SDA/SCL association could be
      confirmed against the primary source, and guessing one would be a
      fabricated pin claim (`AGENTS.md` §1.3). Resolve by reading Table 6-2
      directly, or from TI's published pinmux tooling, then wire
      `SE_I2C_SDA` / `SE_I2C_SCL` to real pins.
- [ ] 3.2 Reconcile the MCU part number. `README.md` and `PCB/*.md` say
      **MSPM0G3518-Q1**; the schematic symbol placed as `U1` is
      **MSPM0G3519**. One of the two is wrong. These are different devices.
- [ ] 3.3 Re-purpose or formally retire the two pins the TPM removal freed,
      now labelled `SPARE_PA2` (pin 6) and `SPARE_PIRQ` (pin 2 / PA1). Both
      still carry their old pull-up resistors (`R24`, `R25`).

## 4. Hardware — Trust / Security Subsystem

- [x] 4.1 ~~SLB9672 TPM 2.0 as `U7`~~ — **superseded 2026-08-10.** Removed in
      favour of a secure element; see 4.2. The servo needs an independent root
      of trust, not the overhead of a full TPM 2.0 stack.
- [x] 4.2 Infineon OPTIGA™ Trust M V3 secure element as `U7` [45] — symbol,
      footprint, schematic placement, 10 kΩ I²C pull-ups (`R30`, `R31`),
      100 nF decoupling (`C43`), and no-connect flags on the five NC contacts.
      Circuit per [45] p.12 §3 Figure 2.
- [~] 4.3 **Wiring is incomplete by design.** `SE_RST` is connected and
      verified (`U1` pin 18 / PA14 ↔ `U7` pin 9), because a reset line needs
      only a plain GPIO. `SE_I2C_SDA` and `SE_I2C_SCL` reach the pull-ups and
      the secure element but **not** the MCU — blocked on 3.1.
- [ ] 4.4 **(High)** Enable the Trust M I²C **Shielded Connection** and
      provision the platform binding secret ([45] p.1 Features, p.10 Fig. 1).
      Without it the session key crosses an exposed 2-wire bus in clear and
      the asymmetric layer buys nothing against an attacker with board access.
- [ ] 4.5 **(High)** Define an anti-replay freshness scheme for the servo bus.
      A MAC authenticates content, not recency; a recorded position command
      replays as valid. The Trust M's 4 monotonic counters ([45] p.10 Fig. 1)
      are boot-scale and cannot serve per-frame.
- [ ] 4.6 **(High, safety-critical)** Define the behaviour on message
      authentication failure. If a failed MAC hard-stops a servo under load, a
      single corrupted frame — or a transient bus fault — becomes a shutdown
      primitive, i.e. the security control becomes the attack. Decide
      fail-operational vs fail-safe and over what window of failures.
- [ ] 4.7 **(Medium)** Decide where per-frame message authentication runs.
      It **must not** run on the Trust M: [45] p.28 §7.2 permits only one
      protected operation per 5 s `t_max` period. The MCU's own AES/CMAC
      engine is the intended home; confirm against 1.4.a.
- [ ] 4.8 **(Medium)** Design fleet key lifecycle and revocation. With 4
      certificate slots and 3 trust anchors there is no room for a
      conventional CRL, and no defined process for retiring a compromised
      servo from a daisy chain.
- [ ] 4.9 **(Low)** Pin the ECC curve in the provisioning profile (P-256
      minimum, P-384 preferred). The device also supports RSA-1024
      ([45] pp. 8–9 Table 4), which must not be selected.
- [ ] 4.10 No PQC path exists. Neither the MCU nor the Trust M offers ML-KEM /
      ML-DSA / SLH-DSA, and the Trust M identity key is fab-provisioned
      ([45] p.11 §2 Note) so it cannot be migrated in the field. Accepted
      limitation of the device class; recorded so it is not rediscovered.

## 5. Hardware — PCB Layout

- [ ] 5.1 Place `U7`, `R30`, `R31`, `C43` on `LibreServo-v4.0.0.kicad_pcb`.
      The secure element is currently schematic-only.
- [ ] 5.2 Remove the SLB9672 footprint and its support parts from the layout.
- [ ] 5.3 Route the I²C pair with attention to length and coupling once 3.1
      fixes the MCU pins.

## 6. Schematic Hygiene

- [ ] 6.1 Annotate the sheet. The netlist export still warns
      "schematic has annotation errors."
- [ ] 6.2 Work down the ERC backlog: 65 violations / 23 errors as of
      2026-08-10 (improved from 67 / 25 by this pass). Dominated by
      `endpoint_off_grid` (25) and `power_pin_not_driven` (17), both inherited
      from the EAGLE import.
- [ ] 6.3 Resolve the EAGLE-import artifacts generally — `similar_label_and_power`,
      `multiple_net_names`, and `label_multiple_wires` all stem from it.

## 7. Firmware

- [ ] 7.1 Firmware is a full rewrite, not a port: `Src/`, `Inc/` and
      `Test_LibreServo_v2.ioc` target ST's HAL and the STM32F302/G431 register
      set, while this fork's MCU is a TI MSPM0. Nothing in `Src/` reflects the
      current hardware.
- [ ] 7.2 Write the Trust M driver layer (I²C, Shielded Connection, ECDSA
      device authentication, ECDHE session-key agreement), honouring the 5 s
      protected-operation budget from 4.7.

---

*Seeded by Claude Opus 5 (`claude-opus-5`) under human direction, 2026-08-10,
alongside the SLB9672 → OPTIGA™ Trust M swap. Items outside §2/§4 are recorded
from repository inspection and are not claimed to be an exhaustive backlog.*
