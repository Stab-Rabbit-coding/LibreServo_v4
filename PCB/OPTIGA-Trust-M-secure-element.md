# Hardware upgrade notes: SLB9672 TPM removed, OPTIGA™ Trust M secure element added as `U7`

**Date:** 2026-08-10
**Applies to:** `PCB/kicad/LibreServo-v4.0.0.kicad_sch` (`U7`, `R30`, `R31`, `C43`)
**Governing rules:** [`AGENTS.md`](../AGENTS.md). Every claim below traces to
[`REFERENCES.md`](../REFERENCES.md) or is marked `UNVERIFIED` / `OPEN`.

Follow-up to [`RS485-CANFD-TPM-upgrade.md`](RS485-CANFD-TPM-upgrade.md) (which
added the SLB9672 as `U7`) and [`S32K144-MCU-swap.md`](S32K144-MCU-swap.md)
(which removed it in the EAGLE era in favour of on-chip CSEc).

---

## 1. Why

`S32K144-MCU-swap.md` §2 flagged a one-way door when it dropped the TPM:

> "**message signing**" in the asymmetric sense … [is a] capabilit[y] the
> SLB9672 offered that CSEc does not … If the project's real goal includes
> public-key servo-to-servo authentication … or attestation to an external
> verifier, CSEc alone doesn't cover that — it would need … keeping some form
> of external secure element.

That is exactly the gap this change closes, and it closes it the cheap way. A
symmetric MAC engine on the MCU is the right home for per-frame authentication
on a servo bus. What it structurally cannot do is prove a servo's *identity* to
a party holding no shared secret, agree a key with a peer it has never met, or
validate a certificate. Those need asymmetric crypto and a key store that
survives compromise of the application MCU.

**A TPM is the wrong shape for that job here.** The SLB9672 is a platform
integrity module: PCRs, attestation state machine, a full TPM 2.0 software
stack, a 32-pin package, and a dedicated SPI bus. A servo controller needs a
key vault, not platform attestation. The **Infineon OPTIGA™ Trust M V3** [45]
is that key vault: 10 pins, I²C, 3 mm × 3 mm (0.118 in × 0.118 in).

**This is not a reinstatement of the TPM.** Different device class, different
bus, different pin count. It reuses only the `U7` designator.

---

## 2. What changed in the schematic

Applied by [`kicad/tools/swap_slb9672_for_optiga.py`](kicad/tools/swap_slb9672_for_optiga.py),
committed so the edit is reproducible rather than an untraceable hand pass.

### Removed

- The `SLB9672_TPM` symbol and the `U7` instance.
- All 16 wires on former `U7` pins.
- The six TPM-side label stubs: `TPM_CS#`, `TPM_PIRQ#`, `TPM_RST#`, `SPI_CLK`,
  `SPI_IN`, `SPI_OUT`. **The SPI nets themselves survive** — they are shared
  with other devices; only the TPM's tap on them is gone.
- Two `NC_MARKER` instances (`NC6`, `NC7`) orphaned on the old TPM's unused
  pins. These were themselves ERC errors.

### Renamed

So the sheet stops claiming a TPM exists:

| Old label | New label | Meaning |
| --- | --- | --- |
| `TPM_RST#` | `SE_RST` | Kept and genuinely reconnected to the new part |
| `TPM_CS#` | `SPARE_PA2` | MCU pin 6, now free |
| `TPM_PIRQ#` | `SPARE_PIRQ` | MCU pin 2 (PA1), now free |

Both spare pins still carry their old pull-ups (`R24`, `R25`) — see
[`TODO.md`](../TODO.md) §3.3.

### Added

Reference circuit per [45] p.12 §3 Figure 2:

| Ref | Part | Notes |
| --- | --- | --- |
| `U7` | OPTIGA™ Trust M V3, SLS 32AIA010ML, PG-USON-10-2,-4 | ETR −40 °C to +105 °C |
| `R30` | 10 kΩ 0402 | SDA pull-up to +3V3 |
| `R31` | 10 kΩ 0402 | SCL pull-up to +3V3 |
| `C43` | 100 nF 0402 | VCC decoupling |

The five NC contacts (2, 4, 5, 6, 7) carry explicit no-connect flags. [45] p.17
Table 6 states they are "Not connected/Do not connect externally. Shall be left
floating" — a datasheet requirement, not a layout preference. **They must never
be tied to a rail.**

Pull-up value: 10 kΩ is the datasheet's own reference value, and [45] p.12
notes the correct value depends on bus capacitance and I²C frequency. Confirm
against the final bus before fab.

---

## 3. Wiring status

**Resolved 2026-08-22** (`TODO.md` §3.1/§4.3). `SE_RST` was already connected
and verified by netlist export: `U1` pin 18 (`PA14`) ↔ `U7` pin 9 (`RST`). A
reset line needs only a plain GPIO, and `PA14` was freed by this very change.

`SE_I2C_SDA` and `SE_I2C_SCL` now also reach the MCU: `PA8` (pin 12,
`PINCM19`, `IOMUX PF3` = `I2C0_SDA`) and `PA1` (pin 2, `PINCM2`, `PF3` =
`I2C0_SCL`) respectively, read directly from [46] (SLASFA6B) Table 6-2 with a
column-structure-preserving PDF extraction (`pymupdf`, not the earlier
extraction that lost the table's columns). Both are TPM-era spare pins (see
`TODO.md` §3.3); `PA2` and `PA14`, the other two spares, were checked
exhaustively against every `PINCMx.PF` option and offer no I²C function at
all, so `PA1`/`PA8` were not a guess among several candidates — they are the
only I²C-capable pair available among the freed pins, and happen to both be
`I2C0`. The MCU-side connection is a same-name KiCad local label pair placed
exactly at each pin's own coordinate (verified against the symbol's pin
geometry, not estimated), matching the existing `SE_RST` convention on this
sheet. `.kicad_pcb` routing is separate, still-open layout work — `TODO.md`
§5.3.

---

## 4. The constraint that governs how this part is used

[45] p.28 §7.1–§7.2: the security monitor sets `t_max` to 5 seconds (± 5%) and
permits **one protected operation per `t_max` period**. "Private key use" is a
protected event.

Consequences, which are firmware obligations, not layout ones:

1. **Device authentication is a boot-time event.** One ECDSA signature with the
   identity key per 5 s, worst case. Authenticating per frame or per command is
   impossible by construction.
2. **The Trust M must never sit in a control hot path.** A servo update loop
   would exceed the budget by orders of magnitude and be throttled into
   failure. Per-frame authentication belongs on the MCU's own AES/CMAC engine.
3. **Session-context keys are exempt**, which is what makes an ECDHE handshake
   practical — but the identity key use that bootstraps the session is not.

Security-protocol decisions — anti-replay freshness, enabling the I²C
Shielded Connection, the fail-behaviour on authentication failure
(safety-critical), key revocation, and curve selection — are recorded in
[`servo-bus-security-protocol.md`](servo-bus-security-protocol.md), tracked
against [`TODO.md`](../TODO.md) §4.4–§4.10. The sister ESC design records a
parallel analysis at `Open-Secure-ESC/docs/secure-element-architecture.md`
(not accessible from this repository/session; the decisions in
`servo-bus-security-protocol.md` were derived independently from this
repo's own primary sources, not copied from it).

---

## 5. Verification

- ERC (`kicad-cli` 9.0.2, `--severity-all`): 67 violations / 25 errors before,
  **65 / 23 after** — a net improvement, from removing the orphaned
  `NC_MARKER`s.
- Netlist export confirms `SE_RST` as a real two-node net.
- Placement checked by SVG render.
- Pin map VERIFIED from [45] p.17 Table 6; package geometry from p.15 Figure 6
  and p.16 Figure 7.
- **Footprint caveat:** the land pattern is an IPC-7351-style engineering
  derivation, **not** an Infineon recommendation — [45] publishes none. It was
  authored by the sister Open-Secure-ESC repo and copied here unchanged; see
  that symbol's `Verification` property for the full derivation record.
- **Not done:** PCB placement. `U7` and its passives are schematic-only.

---

*Authored by Claude Opus 5 (`claude-opus-5`) under human direction, 2026-08-10.*
