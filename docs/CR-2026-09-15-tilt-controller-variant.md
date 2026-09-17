# Change Request — LibreServo_v4.1-TC, nacelle-tilt controller variant

**Raised:** 2026-09-15, from Serenity-UAV Rev T5b (owner decision D5;
`Serenity-UAV/docs/TILT_ACTUATOR_SELECTION.md` §4).
**Author:** Steve Griffing, PE(CSE), CISSP-ISSEP, CPP
**AI note:** Drafted by Claude (model: Claude Opus 5, Anthropic) under the
author's direction, per `AGENTS.md` AI attribution.
**Scope:** a **variant** board, `LibreServo_v4.1-TC`, for the two Serenity nacelle-tilt
actuators. `v4.0.0` is unchanged for the winch and door servos.

## Why

Serenity's nacelle tilt moved from a DS3225 servo body (limit pin removed) to a
brushed gearmotor driving a multi-start worm, with a spring-applied pin brake
(`BRK-1..3`). LibreServo_v4 is a **servo** controller; four of its assumptions
no longer hold.

**Revision 2026-09-16 (Serenity Rev T5e):** the gearmotor is now the **Pololu 20D
25:1 CB 6 V (#3712)** driving a **six-start** worm — the 25D could not clear the
worm wheel (Serenity finding T5d-1). TC-1's current figures drop accordingly
(2.9 A stall, 0.74 A at max efficiency, 0.15 A free-run); the rest of this CR
stands. The board now sits on card-edge rails on the bracket web's OUTBOARD
face, component side toward the web with a 4 mm standoff — components taller
than 4 mm on v4.0.0 must be identified (TC-7).

## Requirements delta (v4.0.0 → v4.1-TC)

| # | Item | v4.0.0 | v4.1-TC requirement | Source |
|---|---|---|---|---|
| TC-1 | Motor drive | discrete N+P bridge (`MOSFET_N+P` ×2, FAN3227 drivers) sized for the DS3225's 2.3 A stall | **Rev T5e: 2.9 A stall, 0.74 A continuous at max efficiency, 0.15 A free-run at 6 V** (was 6.0 / 1.4 / 0.42 A for the 25D); the v4.0.0 bridge is within 30 % of this — re-rate (FET SOA at 2.9 A stall, copper), the ACS711 ±12.5 A variant is no longer needed (±6 A is fine) | Pololu 20D #3712 product page (Serenity REF-ACT-001) |
| TC-2 | Position feedback | AEAT-8800 single-turn absolute on the output | **Revised T5c:** keep the AEAT-8800 — it reads a Ø6 diametric magnet in the worm's brake collar (worm-shaft angle; 9.6 turns over the sweep at the Rev T5e 23.8:1 (14.4 at T5b), so firmware accumulates turns and the AK7455 over the bus gives the absolute nacelle angle). **No quadrature input needed**; the gearmotor is the no-encoder #1571. The sensor sits ~35 mm from the board (carrier pocket in the brake guide) — a short flex/cable to the AEAT footprint, or a remote sensor daughter, is the only board change here | Serenity `TILT_DRIVE_CONTROL_SPEC.md` §1.1/§1.2 |
| TC-3 | Brake driver | none | one low-side driver for a **pull solenoid**, ~0.5 A continuous or PWM-held, flyback clamp, **de-energised = engaged**; brake state readable on the bus; release only on command after the loop has unloaded the pin (BRK-3) | `TILT_DRIVE_CONTROL_SPEC.md` §5.2 |
| TC-4 | Power input | +7 V via MPM3610 from the 6 V servo bus | **own fused feed from VBAT: 22.2 V nominal, 25.2 V full charge**, 3 A branch fuse (F_TILT_P/S). **Verify the MPM3610's maximum input voltage against 25.2 V** (its rating is not in this repo's `REFERENCES.md`); if under, change the front-end regulator | Serenity `POWER_DISTRIBUTION.md` §3.3a/§5 |
| TC-7 | Mechanical fit | board plane free | Rev T5e rails: component side faces the bracket web at a 4 mm standoff, bare back to the hull skin, board slides in along the rails from aft; list every v4.0.0 part taller than 4 mm (inductor, connectors) — relocate or raise the standoff (the envelope has 0 mm to spare outboard) | Serenity `tilt_actuator_bracket.scad` Rev T5e |
| TC-5 | Firmware | servo position loop | cascade: AEAT-8800 (worm shaft) velocity/position inner loop; AK7455 outer loop received over CAN-FD/RS-485; brake sequencing (release → move → settle → engage); differential-tilt trip input; jam detection on current with the AK7455 static | `TILT_DRIVE_CONTROL_SPEC.md` §2, §5.3, §5.4 |
| TC-6 | Sense declaration | — | actuator-positive = nacelle-positive on **port**; the starboard bracket is a mirror but the worm is the same right-hand part, so starboard sense is **reversed** — declared per side in firmware, never discovered | `tilt_actuator_bracket.scad` header |

Unchanged: MSPM0G3518-Q1, OPTIGA Trust M, ADM3055E CAN-FD + ADM2587E RS-485,
board outline 36.5 × 42.9 mm (the Serenity bracket carries it on card-edge
rails, `tilt_actuator_bracket.scad` BOARD_*).

## Mechanical interface (Serenity side)

Bracket rails: board plane parallel to the hull sidewall, 4 mm component
clearance below the board, board slides in along +Y and is retained by a cable
tie. Connectors must exit on the board's aft (+Y) or inboard (−X) edge — the
outboard edge faces the 2 mm rail lip.

## Open

- [ ] TC-4 MPM3610 input rating vs 25.2 V — datasheet not in `PCB/datasheets/`.
- [ ] TC-1 FET/driver re-selection and ACS711 variant — needs the WSD3069DN56
      and ACS711 datasheets (`TODO.md` 1.4.e) first.
- [ ] Decide variant mechanics: separate `LibreServo-v4.1-TC.kicad_*` project vs
      DNP/populate options on one layout.
