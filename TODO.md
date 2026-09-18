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
        →`PA8` (~~`PF3`~~ → **`PF4`**=`I2C0_SDA`, corrected 2026-08-23, see
        3.4), both spare, both `I2C0`. Unblocks 3.1.
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
  - [x] 1.4.f **Intake the OPTIGA™ Trust M Solution Reference Manual**
        (2026-08-23). Located in Infineon's own GitHub organization —
        `Infineon/optiga-trust-m-overview`, commit `a45b86bd` — which serves
        the document the vendor website's anti-bot interstitial had blocked.
        Cataloged as **[53]** with section/page citations read from the local
        copy, alongside three supporting documents from the same repository:
        **[54]** IFX I2C Protocol v2.03, **[55]** Keys and Certificates v3.10,
        **[56]** Configuration Guide v2.2, plus **[57]** the OPTIGA™ Trust M
        Host Library for C at pinned commit `67cfd0e58`. All four PDFs are now
        in `PCB/datasheets/`. **Bonus finding:** the repo's existing copy of
        [45] is byte-for-byte identical (MD5 `5e73fbc0…`) to the datasheet in
        that same Infineon repository, which independently corroborates [45]'s
        provenance — noted in its `REFERENCES.md` entry. Unblocks 4.4.

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
      `PINCM19`, ~~`PF3`~~ → **`PF4`**=`I2C0_SDA` — the `PF` value here was
      wrong and is corrected in 3.4; the *pin* choice stands) — both spares
      freed by the TPM removal (see
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
- [x] 3.4 **IOMUX `PF` value for `SE_I2C_SDA` corrected** (2026-08-23). Items
      3.1 and `PCB/OPTIGA-Trust-M-secure-element.md` §3 both recorded `PA8`'s
      I²C function as `IOMUX PF3`. Re-read from [46] Table 6-2 (p. 18) with a
      column-preserving extraction (`pdftotext -layout`), `PA8` is
      `PF1`=`PA8`, `PF2`=`UART1_TX`, `PF3`=**`SPI0_CS0`**, `PF4`=**`I2C0_SDA`**.
      `PA1`'s `PF3`=`I2C0_SCL` (p. 15) *is* correct; the likely origin of the
      error is `PA0`, whose `PF3` genuinely is `I2C0_SDA`. **Pin selection,
      net names and the schematic are unaffected** — `PA8` is still the right
      pin — but firmware written from "PF3" would mux an SPI chip-select onto
      the secure element's data line and the bus would never come up. Correct
      values now in `firmware/pal/ls_board.h`; both documents amended.

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
- [x] 4.4 **(High) Shielded Connection enabled and platform-binding-secret
      provisioning implemented** (2026-08-23, unblocked by 1.4.f). The
      procedure is [53] §2.3.4 p. 20 Figure 12, written up in
      [`servo-bus-security-protocol.md`](PCB/servo-bus-security-protocol.md)
      §4.4.1–§4.4.4 and implemented in
      [`firmware/trust/ls_trust_pairing.c`](firmware/trust/ls_trust_pairing.c);
      the Shielded Connection itself is compiled in and defaulted to full
      command+response protection in
      [`firmware/config/ls_optiga_lib_config.h`](firmware/config/ls_optiga_lib_config.h).
      **The finding that changes the design's character:** per [56] §2 Table 1
      an OPTIGA™ Trust M **V3** part — the variant on this BOM — ships 0xE140
      with a **Default value and read AC `ALW`**. Until pairing runs, the
      Shielded Connection is keyed by a published constant and protects
      nothing, so pairing is a mandatory manufacturing step, not a hardening
      option. Two divergences from the upstream reference sequence are
      deliberate and documented in §4.4.2: the host store is written and
      verified **before** the irreversible metadata lock, and the final LcsO
      is **operational**, not the example's `creation`. Two consequential
      sub-findings are split out as **4.13** (where the host keeps the secret
      — *not* the MCU KEYSTORE) and **7.6** (the host-side AES-128-CCM /
      TLS-PRF the Shielded Connection obliges the MCU to supply).
- [~] 4.5 **(High)** Define an anti-replay freshness scheme for the servo bus.
      **Decision recorded** 2026-08-22 in
      [`servo-bus-security-protocol.md`](PCB/servo-bus-security-protocol.md)
      §4.5: MAC'd per-frame monotonic counter with a receiver high-water
      mark, not the Trust M's 4 (boot-scale, and per 4.7 out of the per-frame
      path anyway) monotonic counters. **Still open:** counter width and
      per-link vs. global scoping, which §4.5 explicitly deferred to the
      bus-protocol design. 7.2 delivered the *key* the counter is MAC'd under
      (`ls_trust_establish_session_key`), but the frame format itself belongs
      to the bus protocol of 7.1, which does not exist yet — so this stays
      `[~]` rather than being closed on 7.2's completion.
- [~] 4.6 **(High, safety-critical)** Define the behaviour on message
      authentication failure. **Decision recorded** 2026-08-22 in
      [`servo-bus-security-protocol.md`](PCB/servo-bus-security-protocol.md)
      §4.6, explicitly as a judgment call (`AGENTS.md` §4 — no cited standard
      governs this repo's fail-behaviour): fail-operational on an isolated
      MAC failure, fail-safe only after a consecutive-failure threshold,
      never fail-open. Threshold value and per-mechanism safe state are left
      to `TODO.md` 7's control-loop design.
- [x] 4.7 **(Medium)** Decide where per-frame message authentication runs
      (2026-08-22; **rationale corrected and strengthened 2026-08-23**).
      **The MCU's own AESADV/CMAC engine** ([46] §8.18/§8.20) — not the
      Trust M. The conclusion is unchanged; the *reason* given for it was
      overstated. This repo had recorded [45] p.28 §7.2 as a flat "one
      protected operation per 5 s, hard limit". [53] §4.6 shows that is the
      permitted **sustained** usage profile: the hardware throttle only
      begins at SEC = 128 and reaches `t_max` at SEC = 255 (§4.6.4), credit
      accumulates while idle (§4.6.2), and — the load-bearing detail —
      §4.6.1 Table 65 carves *session-context temporary keys* out of all
      three key-use events. So a boot-time burst is affordable and an
      ECDHE handshake is cheap; what remains impossible is per-frame use at
      servo rates, which would pin SEC at its ceiling. Both
      [`servo-bus-security-protocol.md`](PCB/servo-bus-security-protocol.md)
      §4.7 and
      [`OPTIGA-Trust-M-secure-element.md`](PCB/OPTIGA-Trust-M-secure-element.md)
      §4 amended. Firmware honours it: `ls_trust.c` reads OID 0xE0C5 before
      spending budget and refuses rather than issuing into a throttle.
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
- [ ] 4.11 **(Low)** Confirm that [56] (Configuration Guide, Rev 2.2) applies to
      the **/L** sales code. Its title page enumerates SLS 32AIA010M**H/S/K/M**
      and does not list the **/L** on this BOM ([45] p. 8 Table 2,
      SLS 32AIA010ML). The document's subject is the *provisioning variant*
      (V1/V3/Express/MTR), which is orthogonal to the temperature/packing
      suffix, and [55]'s title page does carry **/L** for the same V3 product
      version — so the V3 column is taken as applying. **That is an inference,
      recorded as one** in `REFERENCES.md` [56]. 4.4's whole rationale rests on
      the V3 0xE140 row, so it is worth confirming with Infineon rather than
      leaving inferred.
- [ ] 4.12 **(Low)** Intake the *OPTIGA™ Trust M Release Notes* v3.02, present
      in `Infineon/optiga-trust-m-overview` `docs/pdf/` but deliberately not
      taken in the 2026-08-23 pass — no current design claim depends on a
      release-note item. Fetch it before relying on any firmware-revision-
      specific behaviour of `U7`.
- [ ] 4.13 **(High, blocks provisioning)** Decide and configure **where the MCU
      stores the platform binding secret**, and the flash/debug protection
      around it. Opened by a correction: `servo-bus-security-protocol.md`
      §4.4 had proposed the MCU **KEYSTORE**, and it cannot serve —
      [46] §8.21 p. 89 describes the Keystore as a deposit-then-use-by-AESADV
      store for 128/256-bit keys "without leaking any key data to observers"
      (no software readback), while [53] §6.6.1 requires the host library to
      read the up-to-64-byte secret back through `pal_os_datastore_read`. The
      secret must live in readable MCU NVM protected by static write
      protection, flash read-out protection and the one-way NONMAIN debug
      lockdown of [49] §§2.6/3.2. **This blocks provisioning any unit**,
      because NONMAIN lockdown is a one-way per-unit door. The KEYSTORE
      remains correct for the *derived session CMAC key* of 4.7. Interface is
      already carved out at
      [`firmware/pal/ls_secure_store.h`](firmware/pal/ls_secure_store.h).
- [ ] 4.14 **(Medium)** Implement runtime platform-binding-secret rotation
      ([53] §2.3.6 p. 21 Figure 14), which 4.4's access-condition choice
      deliberately keeps possible (the change AC retains its `Conf(0xE140)`
      branch). [53] §6.5.8 recommends rotation; [53] §5.1's NVM budget
      (2 million tearing-safe programming cycles across all objects; retention
      declining toward ½ year past ~40 000 cycles of an object) is why it must
      be scheduled rather than done casually. Needs a rotation interval
      derived from the expected service life, not a guess.

## 5. Hardware — PCB Layout

> **Finding, 2026-08-23 — this section's premise was wrong, and the items below
> are re-scoped accordingly.** `LibreServo-v4.0.0.kicad_pcb` is still the
> **upstream LibreServo v2.3.1 EAGLE import**, not a v4 layout. Enumerating its
> footprints: `U1` is an **STM32F301/2** in QFN32, `U5` is a **SiT3485**
> RS-485 transceiver in MSOP-8, and there is no `U6`, `U7`, `R24`, `R25`,
> `R30`, `R31` or `C43` anywhere in the file. **None** of the v4 schematic work
> has reached the board: not the MSPM0G3518-Q1 MCU swap (§3), not the
> ADM2587E/ADM3055E isolated-transceiver upgrade, and not the secure element.
>
> Placing `U7` onto that layout in isolation — item 5.1 as written — would
> produce a board that *looks* further along than it is while remaining
> unbuildable, because the MCU it must connect to is a different part, in a
> different package, with a different pinout. That is a worse outcome than
> leaving it undone, so it has not been done.

- [ ] 5.0 **(Blocks 5.1–5.3)** Re-lay `LibreServo-v4.0.0.kicad_pcb` against the
      current schematic, or decide explicitly that the v4 board is a new layout
      rather than an edit of the v2.3.1 import. Until this is settled, the
      three items below have no valid starting point.
- [ ] 5.1 Place `U7`, `R30`, `R31`, `C43` on the v4 layout. **Blocked on 5.0.**
      The secure element is currently schematic-only.
- [x] 5.2 ~~Remove the SLB9672 footprint and its support parts from the
      layout.~~ **Moot, closed 2026-08-23.** There is no SLB9672 footprint in
      `LibreServo-v4.0.0.kicad_pcb` — the TPM never reached this layout in the
      first place (see the finding above). Nothing to remove.
- [ ] 5.3 Route the `SE_I2C_SCL`/`SE_I2C_SDA` pair with attention to length and
      coupling. **Blocked on 5.0 and 5.1.** Two inputs are ready when it
      unblocks: the MCU pins are fixed (§3.1, §3.4 — `PA1` pin 2 and `PA8`
      pin 12), and the firmware runs the bus at Fast-mode 400 kHz
      (`firmware/pal/ls_board.h`), which is the speed [53] §4.4.3 Table 64
      measures the part's own command timings at. Running Fm+ instead would
      require re-checking `R30`/`R31` against the fabricated board's measured
      bus capacitance, per [45] p. 12.

## 6. Schematic Hygiene

- [x] 6.1 **Annotate the sheet** (2026-09-08/09). The 9 orphaned `U$N`
      EAGLE-import placeholders (`U$1`/`U$2` SLOT→`J1`/`J2`, `U$3`/`U$4`
      FFC-5→`J4`/`J3`, `U$5` HEADER→`J5`, `U$6` POTEN_SERVO→`RV1`,
      `U$8`/`U$9` JST_PH-4→`J6`/`J7`, `U$100` OSCILATOR→`Y1`) all now carry
      real reference designators. Verified via `analyze_schematic.py`: 117
      components, 0 duplicates, 0 remaining `U$`-form refs.
- [~] 6.2 **ERC backlog: 64 violations / 19 errors (2026-08-10 baseline)
      → 56 violations / 6 errors (2026-09-08/09)**, verified throughout via
      `kicad-cli sch erc --severity-all` and `kicad-cli sch export netlist`
      (the authoritative ground truth — see the note on
      `analyze_schematic.py`'s net dict below). Two **real, previously-
      undetected wiring defects** were found and fixed in this pass, not
      just hygiene:
  - **`U5` (ADM2587E) RS-485 differential-pair short.** `Y`/`Z`/`A`/`B`
        were all one electrical node (a stray wire bridged the
        `RS485_P`-labeled bus to the `RS485_N`-labeled wire). Split
        correctly into `Y`+`A`→`/RS485_P`, `Z`+`B`→`/RS485_N`, matching
        `PCB/RS485-CANFD-TPM-upgrade.md` §2's documented intent. As drawn
        before this fix, the RS-485 bus could not have worked.
  - **`U6` (ADM3055E) CAN-FD bus short.** `GND1`/`VCC`/`VIO`/`RXD`/`TXD`
        were all bussed onto one wire trunk reaching a `+3V3` symbol —
        shorting logic ground, the isolated 5 V input, the 3.3 V IO
        supply, and both CAN-FD signal pins together. Separated onto
        `GND`/(undriven, see below)/`+3V3`/`/CAN0_RX`/`/CAN0_TX`
        respectively; confirmed `RXD`/`TXD` were already correctly wired
        to `U1` pins 17/16.
  - Also found and fixed: two power-symbol instances (`#P+5`, `#P+6`)
        used the `+5V` symbol type (which asserts membership in a global
        `+5V` net via the symbol's own pin name) while their **Value**
        property had been hand-edited to display `Vmot` — changing the
        Value text does **not** change a KiCad power symbol's underlying
        net tie, so these were silently on a nonexistent `+5V` net
        instead of the real `VMOT` rail. Replaced both with plain local
        `VMOT` labels, unifying `VMOT` as one real net (`M4`/`M5`
        FAN3227 gate-driver `VDD`, `U4` ACS711 `P$3`).
  - Added `power:PWR_FLAG` on 15 nets that ERC correctly reports as
        undriven because nothing in the schematic supplies them from a
        pin ERC recognizes as `power_out`, but which **are** genuinely
        supplied: `U5`/`U6`'s isolated-side `GNDISO`/`GND2`/`VISOIN`/
        `VISOOUT` (isoPower-converter-generated, not modeled as a driving
        pin on either symbol), board `+7V`/`GND`/`VMOT` (external from the
        battery/motor connector), `U1` `VCORE` (MCU-internal core rail,
        decoupling-only per TI's usual guidance), `U2`'s true power input
        (routed through `M1`, a P-FET reverse-polarity protection stage
        whose drain-current path ERC's simple pin-type model can't
        recognize as "driving"), and `U3`'s LDO input (fed from `U2`'s
        regulated output, which the EAGLE-imported symbol types generic
        `output` rather than `power_out`).
  - Tied `U2`'s `AGND` pin (previously a genuinely isolated 2-pin island:
        only reaching feedback resistor `R1`, never board `GND`) to the
        main `GND` net. The MPM3610 datasheet's likely-preferred treatment
        — a dedicated low-noise star-ground point for the feedback
        divider, separate from switching `PGND` — is a **PCB-copper**
        concern to verify at layout time (§5.3/§7 routing), not a
        schematic-connectivity one; the datasheet isn't in
        `PCB/datasheets/` to confirm further (see new item 1.4.g below).
  - **Cannot fix, wiring-verified correct — documented waiver, not an ERC
        exclusion** (kicad-cli 9.0.2 doesn't expose a safe scripted path to
        write `.kicad_pro`'s `erc_exclusions` array without risking file
        corruption; do this from the GUI at the next KiCad session instead):
    - `#+3V1` (`power_pin_not_driven`, 1 error): this `+3V3` power-symbol
          instance's local wire network was traced (via
          `kicad-cli sch export netlist`, cross-checked against raw wire/
          pin geometry) all the way to `C9`'s pin 1, which the netlist
          confirms is on the real, `U3`-driven `+3V3` net. kicad-cli's ERC
          appears to evaluate `power_pin_not_driven` per pre-merge local
          subgraph rather than crediting the post-merge named net for this
          specific instance — a verified false positive, not a defect.
    - `U2` `P$4`/`P$5`/`P$6` (`SW`, `pin_not_connected`, 1 error): the
          MPM3610 (`MPM36XX` symbol) is a Monolithic-Power-style integrated
          power module (inductor-in-package); its `SW` node may be
          correctly left unconnected on this class of part, but the
          MPM3610 datasheet isn't in `PCB/datasheets/` to confirm — see new
          item 1.4.g.
    - `U6` `VCC` (`pin_not_connected` + `power_pin_not_driven`, 2 errors):
          **deliberately undriven**, per `PCB/RS485-CANFD-TPM-upgrade.md`
          §3's own recorded, accepted open item — "wired to a new
          `+5V_ISO_CANFD` net that nothing currently drives ... deliberately
          left as an open BOM/regulator-selection decision." Not this
          pass's decision to make.
    - `J1`/`J2` `P$2` (SERVO_SLOT connectors, `pin_not_connected`, 2
          errors): `P$1` on each correctly reaches the corresponding motor
          half-bridge output (`M2`/`M3`); `P$2`'s purpose (a second,
          possibly-redundant pad for the same physical motor-brush
          contact, or something else) needs physical verification against
          the SERVO_SLOT footprint/mechanical drawing — no datasheet exists
          for this custom part. New item, see 1.4.g.
  - **Still open, mechanical/cosmetic, not attempted this pass**:
        `endpoint_off_grid` (28), `unconnected_wire_endpoint` (11),
        `label_multiple_wires` (3), `no_connect_connected` (1) — these are
        exactly the kind of bulk grid-snap/dangling-endpoint cleanup
        KiCad's own GUI "Cleanup Schematic" / manual review handles far
        more safely than further hand S-expression editing (this pass hit
        two near-misses doing exactly that: an insertion at the wrong
        `lib_symbols` boundary, and a wire deletion that turned out to
        still be load-bearing — both caught by immediate
        `kicad-cli sch export netlist` re-verification and reverted before
        committing). Do this pass in the KiCad 9 GUI.
- [~] 6.3 EAGLE-import artifact classes: `similar_label_and_power` (4→0)
      and `multiple_net_names` (3→0, then a 4th appeared and was resolved
      transiently during 6.2's VMOT fix) are now clear. `label_multiple_wires`
      (3) remains, folded into 6.2's "still open, mechanical" list above.
- [ ] 1.4.g **(new)** Intake datasheets for the MPM3610-class part behind
      the `MPM36XX` symbol (AGND star-ground and `SW`-pin treatment) and
      for the SERVO_SLOT connector footprint's mechanical drawing (`J1`/
      `J2` `P$2` purpose). Both block closing 6.2's last two error
      categories with a real fix rather than a waiver.

## 7. Firmware

- [ ] 7.1 Firmware is a full rewrite, not a port: `Src/`, `Inc/` and
      `Test_LibreServo_v2.ioc` target ST's HAL and the STM32F302/G431 register
      set, while this fork's MCU is a TI MSPM0. Nothing in `Src/` reflects the
      current hardware. **Unchanged by 7.2** — the new `firmware/` tree is the
      security subsystem only and deliberately does not touch `Src/`/`Inc/`.
- [x] 7.2 **Trust M driver layer written** (2026-08-23) — I²C, Shielded
      Connection, ECDSA device authentication, ECDHE session-key agreement.
      Delivered as [`firmware/`](firmware/); see
      [`firmware/README.md`](firmware/README.md) for the architecture, the
      upstream pinning and attribution chain, and the findings.
      - `firmware/config/ls_optiga_lib_config.h` — feature selection, a
        deliberate narrowing of [57]'s V3 profile, selected through the
        upstream `OPTIGA_LIB_EXTERNAL` hook rather than by shadowing a header.
      - `firmware/pal/` — the full platform abstraction layer for the
        MSPM0G3518-Q1: I²C (register map read from [52] ch. 25), GPIO, OS
        timer/event/lock/memory, datastore, logger, and the host-side
        AES-128-CCM and TLS-1.2-PRF the Shielded Connection requires.
      - `firmware/trust/` — the servo-facing API: `ls_trust_init`,
        `ls_trust_authenticate` (ECDSA over 0xE0F0),
        `ls_trust_establish_session_key` (ephemeral keypair → ECDH → TLS-PRF,
        all in session context 0xE100), `ls_trust_pair_with_host` (4.4), plus
        the OID map and security-monitor constants from [53] §5.4/§4.6.
      - `firmware/tests/` — host verification of the two specified
        constructions: **52/52** AES-128-CCM outputs byte-identical to
        `python-cryptography`'s `AESCCM` across every legal nonce and tag
        length of [59] Appendix A.1, including [59] Appendix C's three worked
        vectors; **24/24** TLS-PRF outputs identical to an independent
        RFC 5246 §5 implementation.
      - Verification: all 12 sources compile clean under
        `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wcast-qual` against
        the real upstream headers, and a link against the full host library
        leaves **only** the eight symbols of the three declared
        application-supplied contracts unresolved (7.3, 4.13, 7.6) — no
        accidental gaps.
      Follow-ups split out below rather than left implicit: 7.3, 7.4, 7.5, 7.6.
- [ ] 7.3 **(Blocks bring-up)** Design the MCU clock tree and correct
      `LS_I2C_FUNCTIONAL_CLK_HZ` in
      [`firmware/pal/ls_board.h`](firmware/pal/ls_board.h). It currently holds
      32 MHz, marked `UNVERIFIED — needs primary source`, chosen only because
      that is the value [52] §25.2.1 works its own TPR example with, so the
      derived divisor is checkable against the document. `pal_i2c_init()`
      recomputes TPR from this constant — correct the constant, never
      hand-patch the TPR.
- [ ] 7.4 **(Low)** Convert `firmware/pal/ls_pal_i2c.c` from blocking transfers
      to interrupt-driven ones. The PAL contract permits blocking and it is the
      right first implementation — 4.7 keeps `U7` out of the control hot path,
      so the worst blocking window is one APDU at 400 kHz during boot. **Do not
      do this before 7.1's control loop exists** to measure against; optimising
      against a guess is how the wrong thing gets optimised.
- [ ] 7.5 **(Low)** Report two upstream header-guard bugs to Infineon
      ([57] at commit `67cfd0e58`), both found while narrowing the feature set
      and both invisible with the default V3 profile:
      (a) `include/cmd/optiga_cmd.h:596` guards `optiga_cmd_decrypt_sym` on
      `SYM_DECRYPT || HMAC_VERIFY || CLEAR_AUTO_STATE`, but its parameter type
      `optiga_decrypt_sym_params_t` (`include/common/optiga_lib_common.h:524`)
      is guarded on `SYM_ENCRYPT || SYM_DECRYPT` alone;
      (b) `src/crypt/optiga_crypt.c` wraps all three `optiga_crypt_tls_prf_shaXXX`
      bodies (lines 506–642) in one combined guard while the enum values they
      reference are each guarded individually. Workarounds and reasoning are in
      `firmware/config/ls_optiga_lib_config.h`.
- [ ] 7.6 **(High, blocks bring-up)** Bind
      [`firmware/pal/ls_crypto_backend.h`](firmware/pal/ls_crypto_backend.h) to
      a vetted AES-128 block cipher and HMAC-SHA256. Opened by 4.4: enabling
      the Shielded Connection makes `pal_crypt_tls_prf_sha256`,
      `pal_crypt_encrypt_aes128_ccm` and `pal_crypt_decrypt_aes128_ccm` a
      link-time requirement, because the IFX I²C presentation layer [54]
      protects every APDU with AES-128-CCM on the host side — a cost that does
      not appear in [45]. The *constructions* are implemented and verified
      (`firmware/pal/ls_pal_crypt.c`, `firmware/tests/`); the primitives
      underneath are deliberately not, and must not be hand-rolled. Candidates:
      MSPM0 AESADV ([46] §8.20; [49] Table 5-3 gives 0.95 µs per 128-bit block
      at 80 MHz) plus a software SHA-256, or Mbed TLS as [57]'s own reference
      PALs use. Whichever is chosen must be constant-time with respect to key
      material.

## 8. Variant — LibreServo_v4.1-TC (Serenity nacelle-tilt controller) — SUPERSEDED

Raised 2026-09-15 by Serenity-UAV Rev T5b (`docs/CR-2026-09-15-tilt-controller-variant.md`).
**Superseded 2026-09-17:** the tilt controller is an Open-Secure-ESC build
(`Open-Secure-ESC/builds/6s/10A/BRUSHED_CAN_485_isolation/`), not a LibreServo
variant — see the CR's status banner. v4.0.0 stays as-is for the winch/door
servos. Every item below is closed as superseded; none carries forward here.

- [x] 8.1 TC-1 Bridge re-rate — SUPERSEDED 2026-09-17 → Open-Secure-ESC build
      (DRV8874-Q1 integrated bridge). Never resolved here; moot.
- [x] 8.2 TC-2 Remote AEAT-8800 — SUPERSEDED → Open-Secure-ESC build (6-pin
      encoder header J7; `symbols/AEAT_8800_Q24`).
- [x] 8.3 TC-3 Brake driver — SUPERSEDED → Open-Secure-ESC build (TPL7407L,
      Holding Brake axis).
- [x] 8.4 TC-4 VBAT front end / MPM3610 rating — SUPERSEDED → Open-Secure-ESC
      build (TPS54560B buck). The MPM3610 rating was never obtained; moot.
- [x] 8.5 TC-5 Firmware cascade — SUPERSEDED → Open-Secure-ESC build README
      "Firmware requirements".
- [x] 8.6 TC-6 Per-side sense — SUPERSEDED → Open-Secure-ESC build README
      "Host constraints".
- [x] 8.7 Variant mechanics — SUPERSEDED; no variant project is created.

---

*Seeded by Claude Opus 5 (`claude-opus-5`) under human direction, 2026-08-10,
alongside the SLB9672 → OPTIGA™ Trust M swap; extended by Claude Sonnet 5
(`claude-sonnet-5`) under human direction, 2026-08-22 (§1.4, §1.6, §3, §4.4–4.10);
extended again by Claude Opus 5 (`claude-opus-5`) under human direction,
2026-08-23 (1.4.f, 3.4, 4.4, 4.7, 4.11–4.14, §5 re-scope, 7.2–7.6), on intake of
the OPTIGA™ Trust M Solution Reference Manual and the delivery of `firmware/`.
Items outside §2/§4 are recorded from repository inspection and are not claimed
to be an exhaustive backlog.*
