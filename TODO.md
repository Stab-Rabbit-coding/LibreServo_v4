# TODO — LibreServo v4.0.0 (Work Breakdown Structure)

Formal WBS per the project governance in [`AGENTS.md`](AGENTS.md).

Legend: `[ ]` open · `[~]` in progress / partially resolved · `[x]` closed

---

## 1. Governance & Documentation

- [x] 1.1 Adopt `AGENTS.md` (2026-08-10, from the sister Open-Secure-ESC
      project — see the provenance note at the top of that file).
- [x] 1.2 Create `CLAUDE.md` stub directing AI agents to `AGENTS.md`.
- [x] 1.3 Create `REFERENCES.md`.
- [x] 1.4 **Complete the `REFERENCES.md` intake** (2026-08-22). Tags [46]–[52]
      added with validated URLs and section/page citations read from the local
      PDFs. See sub-items.
  - [x] 1.4.a **(was blocking 3.1)** TI MSPM0G351x-Q1 SLASFA6B → [46]. Table
        6-2 read directly: `SE_I2C_SCL`→`PA1` (`PF3`=`I2C0_SCL`), `SE_I2C_SDA`
        →`PA8` (`PF3`=`I2C0_SDA`), both spare, both `I2C0`. Unblocks 3.1.
  - [x] 1.4.b ADM2582E/ADM2587E (`U5`) → [47]; ADM3055E/ADM3057E (`U6`) → [48].
        Formalizes citations already quoted verbatim in
        `PCB/MSPM0G3507-MCU-swap.md` but not previously in `REFERENCES.md`.
  - [x] 1.4.c The five TI MSPM0 application notes / TRM / errata → [49]–[52]
        cataloged (two with full section citations already used in-repo, two
        scope-verified but with no specific in-repo claim yet, honestly
        marked as such). **Finding:** `slaz742g.pdf` (errata) does **not**
        apply to the current MSPM0G351x-Q1 die — its title/scope covers the
        MSPM0G3x0x/G1x0x family from the *superseded* MSPM0G3507 pass. Recorded
        under REFERENCES.md "Considered and found not applicable" rather than
        cataloged with a tag; the correct MSPM0G351x-Q1 errata document is not
        yet in `PCB/datasheets/` and remains a real gap — see 1.4.d.
  - [ ] 1.4.d Locate and intake the TI errata document for MSPM0G3518-Q1/
        MSPM0G3519-Q1 specifically (SLAZ742G covers a different die — see
        1.4.c finding). Not yet in `PCB/datasheets/`.
- [x] 1.5 Create `PROJECT_INDEX.md`.
- [x] 1.6 Audit the repo for claims that cite no source (2026-08-22). Findings,
      all in `README.md`:
  - **Fixed:** "Max Speed 9Mbps" for "Isolated RS-485 and CAN-FD" didn't match
        either transceiver's datasheet — RS-485 (ADM2587E [47]) is 500 kbps,
        CAN-FD (ADM3055E [48]) is up to 12 Mbps demonstrated / 5 Mbps rated.
        Corrected in place with per-bus cited figures.
  - **Marked `UNVERIFIED` in place** (no local datasheet exists in
        `PCB/datasheets/` to check the figure against): the 16 A WSD3069DN56
        motor-driver rating, the AEAT-8800 encoder's 16-bit/360° claim, the
        ACS711 ±15 A current-sensor rating, and the 4.5–18 V board voltage
        range (no cited component derivation on file). Follow-up: intake
        datasheets for WSD3069DN56, AEAT-8800, and ACS711 — see 1.4.e.
  - `PCB/*-swap.md` and `PCB/ReadMe.md` were also checked: their technical
        claims already carry inline datasheet section/page citations (now
        cross-verified against [46]–[48] in this same pass) and were not
        found to need further sourcing.
  - [ ] 1.4.e Intake datasheets for WSD3069DN56, AEAT-8800, and ACS711 (none
        present in `PCB/datasheets/`) and cite their README.md claims, or
        correct the claims if the primary source disagrees.
  - [ ] 1.4.f Intake the OPTIGA™ Trust M **Solution Reference Manual** ([45]
        p.10 says it ships "as part of the package"; not present in
        `PCB/datasheets/`). Needed before 4.4's Shielded Connection
        platform-binding-secret pairing procedure can be implemented — see
        `PCB/servo-bus-security-protocol.md` §4.4.

## 2. Repository / Naming

- [x] 2.1 Rename the repository and working folder `LibreServo_v2-Sec` →
      `LibreServo_v4`, matching the `origin` rename, and update the `origin`
      remote URL (2026-08-10). The `upstream` remote still points at
      `Luisonson/LibreServo_v2` and must **not** be retargeted — that is the
      genuine fork source.

## 3. Hardware — MCU Subsystem

- [x] 3.1 **Resolve the MSPM0G351x pin map, and specifically an I²C-capable
      pin pair for the secure element** (2026-08-22, [46] Table 6-2, direct
      PDF text extraction with `pymupdf` preserved the column structure).
      Result: `PA1` (pin 2, `PINCM2`, `PF3`=`I2C0_SCL`) and `PA8` (pin 12,
      `PINCM19`, `PF3`=`I2C0_SDA`) — both spares freed by the TPM removal (see
      3.3), both `I2C0`, both confirmed as the *only* I²C-capable options
      among the four TPM-era spare pins (`PA1`, `PA2`, `PA8`, `PA14` — `PA2`
      and `PA14` were checked exhaustively against every `PF` row and offer no
      I²C function at all). Wired in
      `PCB/kicad/LibreServo-v4.0.0.kicad_sch` — see 4.3.
- [x] 3.2 Reconcile the MCU part number (2026-08-22). `README.md`, the
      schematic title block, and `PCB/MSPM0G3518-MCU-swap.md` all named
      **MSPM0G3518-Q1**; the placed `U1` symbol had drifted to
      **MSPM0G3519** (`M0G3519QRHBRQ1`, the 512 KB sibling — confirmed a real,
      distinct, separately-orderable part per [46] Table 5-1, not a typo).
      Standardized on **MSPM0G3518-Q1** per the majority-consistent,
      human-authored design intent recorded in `PCB/MSPM0G3518-MCU-swap.md`.
      Renamed the library symbol `MSPM0G3519`→`MSPM0G3518` and corrected
      Value/Description in both `kicad/LibreServo-v4.0.0.kicad_sch` and
      `kicad/LibreServo-v4.0.0-eagle-import.kicad_sym`; RHB-32 pinout (Table
      6-2) is identical between the two dies so this was a bookkeeping fix,
      not a pin remap. See `PCB/MSPM0G3518-MCU-swap.md` §6.2 for the detailed
      record, including a retraction of that document's own earlier (wrong)
      claim that the '3519 isn't offered in RHB-32.
- [x] 3.3 Re-purpose or formally retire the two pins the TPM removal freed,
      now labelled `SPARE_PA2` (pin 6) and `SPARE_PIRQ` (pin 2 / PA1)
      (2026-08-22). `PA1`/`SPARE_PIRQ` → **repurposed** as `SE_I2C_SCL` (3.1);
      its old 10 kΩ pull-up `R25` set `dnp` (do-not-populate) since the I²C
      bus already gets its pull-up from `R31` per [45] p.12 Fig. 2 and a
      second parallel 10 kΩ would halve the effective pull-up undocumented.
      `PA2`/`SPARE_PA2` → **formally retired**: confirmed via [46] Table 6-2
      it has no I²C (or other currently-useful) function among its `PF`
      options, so it stays a plain spare GPIO; its old pull-up `R24` set
      `dnp` too, since TI's unused-pin guidance ([46]/SLASFA6B Table 6-20) is
      to configure it in firmware (GPIO output-low or input with internal
      pull), not to carry a permanent external pull-up to `+3V3`.

## 4. Hardware — Trust / Security Subsystem

- [x] 4.1 ~~SLB9672 TPM 2.0 as `U7`~~ — **superseded 2026-08-10.** Removed in
      favour of a secure element; see 4.2. The servo needs an independent root
      of trust, not the overhead of a full TPM 2.0 stack.
- [x] 4.2 Infineon OPTIGA™ Trust M V3 secure element as `U7` [45] — symbol,
      footprint, schematic placement, 10 kΩ I²C pull-ups (`R30`, `R31`),
      100 nF decoupling (`C43`), and no-connect flags on the five NC contacts.
      Circuit per [45] p.12 §3 Figure 2.
- [x] 4.3 **Wiring completed** (2026-08-22, unblocked by 3.1). `SE_RST` was
      already connected and verified (`U1` pin 18 / `PA14` ↔ `U7` pin 9).
      `SE_I2C_SCL` now also reaches `U1` pin 2 (`PA1`) and `SE_I2C_SDA` reaches
      `U1` pin 12 (`PA8`), completing the MCU-to-secure-element I²C link
      alongside the existing pull-ups (`R30`, `R31`) and the secure element
      (`U7`) in `PCB/kicad/LibreServo-v4.0.0.kicad_sch`. Net-label-only
      connections (KiCad same-name local labels), pin-coordinate-verified
      against the symbol's own pin geometry — no new wire geometry was
      guessed. `.kicad_pcb` routing is separate follow-up work — see 5.3.
- [~] 4.4 **(High)** Enable the Trust M I²C **Shielded Connection** and
      provision the platform binding secret ([45] p.1 Features, p.10 Fig. 1).
      **Decision recorded** 2026-08-22 in
      [`servo-bus-security-protocol.md`](PCB/servo-bus-security-protocol.md)
      §4.4: enable it for all `U7` traffic. **Still open:** the platform
      binding secret provisioning procedure needs the Trust M Solution
      Reference Manual, not yet intaken — see 1.4.f. Not implemented (no
      firmware exists yet, `TODO.md` 7).
- [~] 4.5 **(High)** Define an anti-replay freshness scheme for the servo bus.
      **Decision recorded** 2026-08-22 in
      [`servo-bus-security-protocol.md`](PCB/servo-bus-security-protocol.md)
      §4.5: MAC'd per-frame monotonic counter with a receiver high-water
      mark, not the Trust M's 4 (boot-scale, and per 4.7 out of the per-frame
      path anyway) monotonic counters. Counter width/scoping left to 7.2.
- [~] 4.6 **(High, safety-critical)** Define the behaviour on message
      authentication failure. **Decision recorded** 2026-08-22 in
      [`servo-bus-security-protocol.md`](PCB/servo-bus-security-protocol.md)
      §4.6, explicitly as a judgment call (`AGENTS.md` §4 — no cited standard
      governs this repo's fail-behaviour): fail-operational on an isolated
      MAC failure, fail-safe only after a consecutive-failure threshold,
      never fail-open. Threshold value and per-mechanism safe state are left
      to `TODO.md` 7's control-loop design.
- [x] 4.7 **(Medium)** Decide where per-frame message authentication runs
      (2026-08-22). **The MCU's own AESADV/CMAC engine** ([46] §8.18/§8.20,
      confirmed against 1.4.a) — not the Trust M ([45] p.28 §7.2's 5 s
      `t_max` budget rules it out independent of the latency numbers in
      §4.4). See
      [`servo-bus-security-protocol.md`](PCB/servo-bus-security-protocol.md)
      §4.7. Firmware placement (key-slot selection) is 7.2 scope.
- [~] 4.8 **(Medium)** Design fleet key lifecycle and revocation. **Partial
      decision recorded** 2026-08-22 in
      [`servo-bus-security-protocol.md`](PCB/servo-bus-security-protocol.md)
      §4.8: revocation state lives at the fleet controller (allowlist), not
      as an on-part CRL — forced by the 4 cert-slot / 3 trust-anchor limit.
      **Still open:** how a distrusted node is kept from relaying traffic on
      a physical daisy chain — needs the `TODO.md` 7 bus-protocol design,
      which doesn't exist yet.
- [x] 4.9 **(Low)** Pin the ECC curve in the provisioning profile (2026-08-22,
      [`servo-bus-security-protocol.md`](PCB/servo-bus-security-protocol.md)
      §4.9): **P-256 minimum** (matches the MCU's own CSC secure-boot curve,
      [46] §8.18), **P-384 preferred where the latency budget allows** (not
      hardened into a firm requirement — [45] publishes no P-384-specific
      timing to weigh against P-256). RSA is excluded entirely, not just
      RSA-1024 ([45] pp. 8–9 Table 4 lists it as available; not selected).
- [x] 4.10 No PQC path exists. Neither the MCU nor the Trust M offers ML-KEM /
      ML-DSA / SLH-DSA, and the Trust M identity key is fab-provisioned
      ([45] p.11 §2 Note) so it cannot be migrated in the field. Accepted
      limitation of the device class; recorded so it is not rediscovered.
      Re-verified 2026-08-22 against both [45] and [46] directly — no PQC
      primitive found in either. Closed as a recorded, accepted limitation
      per its own text, not an open action.

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
