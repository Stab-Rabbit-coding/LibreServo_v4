# Servo bus security protocol — design decisions (TODO.md §4.4–4.10)

**Date:** 2026-08-22
**Status:** design decisions recorded; **no firmware exists yet** — this feeds
`TODO.md` 7.2 (Trust M driver layer) and 7.1 (full firmware rewrite). Nothing
here is implemented; it is the record `AGENTS.md` §4 requires before it is.
**Governing rules:** [`AGENTS.md`](../AGENTS.md). Every claim below traces to
[`REFERENCES.md`](../REFERENCES.md) or is marked `UNVERIFIED`/`OPEN`/
`JUDGMENT CALL` (the last per `AGENTS.md` §4: "if a decision is a judgment
call with no governing standard, say so explicitly").

This document does not re-derive [45]'s hard constraint that the Trust M
permits only one protected operation per 5 s `t_max` — see
[`OPTIGA-Trust-M-secure-element.md`](OPTIGA-Trust-M-secure-element.md) §4 for
that. Everything below is downstream of it.

---

## 4.4 — I²C Shielded Connection and the platform binding secret

**Decision: enable Shielded Connection for all I²C traffic to `U7`, keyed
from a platform binding secret provisioned at manufacture.**

What [45] establishes directly:

- p. 1, Features: "I²C interface with Shielded Connection (encrypted
  communication)."
- p. 10, Fig. 1 "System block diagram" — the object model includes exactly
  **1 Platform Binding Secret slot**. The `COMMS` layer of the Trust M Host
  Library provides "wrapper APIs for communication (optional encrypted
  communication using Shielded Connection)... which internally uses Infineon
  I2C Protocol (IFX I2C)."
- p. 34, §A.2, Table 24 — the `PRESENT_LAYER` flag in the `I2C_STATE`
  register "indicates the optional availability of the presentation layer,
  which is providing confidentiality and integrity protection of payloads
  (APDUs) transferred across the I2C interface. The presentation layer is
  used as part of Shielded Connection." Confirms Shielded Connection is an
  APDU-level (application-protocol-data-unit) confidentiality+integrity
  wrapper, not a bus-electrical property — it protects the *payload*, not the
  wire.
- pp. 26–27, Table 18 "Crypto performance for V3" — enabling Shielded
  Connection adds a small, consistent latency tax (e.g. ECDSA sign ~65 ms →
  ~70 ms; AES-128 encrypt ~28 ms → ~35 ms; measured at I²C FM 400 kHz,
  VCC 3.3 V, 25 °C). Material for the 4.7 budget discussion below, not
  disqualifying on its own.

**Why this is the decision, not a preference:** without Shielded Connection,
[45] p.12 §3's reference circuit (adopted verbatim in
`OPTIGA-Trust-M-secure-element.md`) puts `SE_I2C_SDA`/`SE_I2C_SCL` on an
exposed 2-wire bus between `U1` and `U7` — a bus that, once 4.3 is wired
(done, 2026-08-22), is physically probeable from anywhere the board's
copper is accessible. The asymmetric identity/session-agreement layer this
part exists for (README.md, `OPTIGA-Trust-M-secure-element.md` §1) is
pointless if the ECDHE session key it produces then crosses that same bus in
clear on every subsequent transaction. This is `TODO.md` 4.4's own stated
rationale and [45] gives no reason to disagree.

**`UNVERIFIED — needs primary source (see TODO.md)`, tracked as new item
1.4.f:** the *procedure* for provisioning the platform binding secret (how it
is generated, how the same value is safely deposited into both `U7`'s
platform-binding-secret slot and the MCU's KEYSTORE at manufacture, and what
key-derivation function seeds the Shielded Connection session from it) is not
in the datasheet — p. 10 explicitly defers to the **"Solution Reference
Manual document available as part of the package,"** which is not present in
`PCB/datasheets/` and was not reachable in this session. Do not implement
firmware-side Shielded Connection pairing from memory of similar protocols;
intake that document first.

**Provisioning-flow interaction:** per [49] (SLAAE29A) §2.6/§3.2, MCU-side
NONMAIN lockdown is a one-way, per-unit decision. The platform binding secret
must be deposited into both parts **in the same production step** that
performs that lockdown — provisioning the Trust M side without a matching,
protected MCU-side secret store defeats the point. This is a manufacturing
process requirement, not just a firmware one; flag for 4.8 (key lifecycle)
below.

---

## 4.5 — Anti-replay freshness scheme

**Decision: a monotonically increasing per-link frame counter, MAC'd inside
the authenticated payload, with a receiver-side high-water-mark window — not
the Trust M's monotonic counters.**

Why not the Trust M's counters: [45] p.10 Fig. 1 lists exactly 4 monotonic
counters, and per §4.7 below the Trust M cannot sit in the per-frame path at
all (5 s `t_max` budget) — so they are structurally unusable for per-frame
freshness regardless of count. **`JUDGMENT CALL` — no cited standard
mandates a specific anti-replay construction for a servo command bus**; the
scheme below follows the pattern any AEAD/MAC'd link with a bounded number of
peers typically uses (e.g. the same shape as an 802.1AE/MACsec replay window
or a TLS record sequence number, cited here as *prior art shape*, not as a
governing requirement this project is bound by):

1. Each frame carries an explicit N-bit counter, MAC'd together with the
   command payload (so the counter itself is authenticated — an attacker
   cannot replay an old frame with a forged new counter without the key).
2. The transmitter's counter increments once per frame and **never wraps
   silently** — reaching the counter's maximum is a fault condition (see 4.6),
   not a rollover, because a silent wrap reopens the replay window.
3. The receiver accepts a frame only if its counter is strictly greater than
   the last accepted counter for that link (a simple high-water mark, not a
   sliding window with reordering tolerance) — this is a point-to-point,
   daisy-chained servo command bus per README.md, not a lossy/reordering
   network, so a strict monotonic check is the simplest correct option and
   deliberately does not add sliding-window complexity the transport doesn't
   need.
4. Counter state must survive a normal reset (stored in a location that
   isn't wiped by `SYSRST`) so that a servo power-cycle doesn't reopen a
   replay window for an attacker who recorded pre-cycle traffic — this
   interacts directly with 4.6's fail-safe/fail-operational decision and
   with 7.2's firmware implementation. **Where exactly this counter is
   stored (which non-volatile region, whether it needs its own wear-leveling)
   is firmware design, deliberately left open here rather than guessed.**

**`OPEN`:** counter width and per-link vs. global scoping (one counter per
daisy-chain, or one per addressed servo) are firmware/protocol decisions for
7.2, not hardware ones, and are not fixed by this document.

---

## 4.6 — Behaviour on message authentication failure (safety-critical)

**Decision: fail-operational on isolated MAC failures, fail-safe only after a
consecutive-failure threshold — never a single-frame hard stop.**

`JUDGMENT CALL` — `AGENTS.md` has no cited standard governing servo
fail-behaviour, and this project has adopted none by reference (no IEC 61508/
ISO 13849 SIL claim exists anywhere in this repo). This decision is recorded
as a judgment call, not derived from a standard, per `AGENTS.md` §4.

Rationale, directly from `TODO.md` 4.6's own framing: a MAC authenticates
content but a *single* MAC failure is not distinguishable, on its own, from
ordinary bus noise/EMI on an exposed multidrop RS-485/CAN-FD daisy chain
(README.md; `PCB/RS485-CANFD-TPM-upgrade.md` documents this bus as sharing a
board `Vmot`/`GND` with other servos in the chain — a plausible noise
coupling path, flagged as its own open item in
`PCB/MSPM0G3518-MCU-swap.md` §6.4). Treating one corrupted frame as an attack
and hard-stopping a servo under mechanical load turns transient bus noise
into an actuation fault — exactly the "security control becomes the attack"
failure mode `TODO.md` 4.6 warns against.

The decision:

1. **A single MAC failure on an otherwise-valid frame is discarded silently
   at the protocol layer** (logged, not acted on) — the servo continues
   executing its last-known-good command (fail-operational for one frame).
2. **N consecutive MAC failures** (N is a firmware-tunable threshold, not
   fixed here — this document does not have the control-loop timing data to
   set it) **escalate to fail-safe**: the servo enters a defined safe state
   (decelerate-to-stop or hold, whichever `TODO.md` 7's eventual control-loop
   design designates as safe for the mechanism being driven — not decided by
   this document, which is bus/security scope only).
3. **A MAC failure is never, by itself, sufficient reason to disable the
   authentication requirement** (i.e. no "fail open" fallback to accepting
   unauthenticated frames after N failures) — that would let a jamming
   attacker strip authentication by inducing exactly the condition that's
   supposed to trigger it.
4. Anti-replay rejections (4.5) are counted **separately** from MAC failures
   for this threshold, since a replay rejection under normal operation
   (e.g. a legitimately duplicated frame from a bus retry) is a different,
   generally more benign, condition than a MAC mismatch, and conflating them
   would make the threshold harder to tune correctly.

**`OPEN`, explicitly not decided here:** the numeric value of N, the exact
"safe state" per mechanism, and whether the threshold is time-windowed or a
raw consecutive count. These require the control-loop engineering that
`TODO.md` 7.1 hasn't started yet (`Src/` is still STM32-era, unrelated to the
current MCU) and must not be guessed at here.

---

## 4.7 — Where per-frame authentication runs

**Decision: the MCU's own AESADV/CMAC engine — confirmed, not the Trust M.**

This is close to fully settled by 1.4.a/[46] and [45], not really a judgment
call:

- [45] p.28 §7.1–§7.2 (already the basis for `OPTIGA-Trust-M-secure-element.md`
  §4): one protected operation per 5 s `t_max`, hard security-monitor limit.
  A servo control loop authenticating every frame at any realistic frame
  rate (tens of Hz to kHz) exceeds this by orders of magnitude — this alone
  rules the Trust M out for per-frame work, independent of the 4.4 latency
  numbers above.
- [46] (SLASFA6B) p. 88 §8.18 and p. 89 §8.20 "AESADV" — confirms the MCU
  (`M0G3518QRHBRQ1`) has its own on-die AES-128/256 engine with CBC-MAC/CMAC/
  GCM/GMAC modes, independent of the Trust M, matching the README.md claim
  "AES-256 with CMAC/GCM."
- [49] (SLAAE29A) Table 5-3 — AESADV throughput at 80 MHz: 128-bit key 76
  cycles/0.95 µs per block, 256-bit key 81 cycles/1.01 µs per block. This is
  comfortably inside any servo frame period this project's stated 9 Mbps-era
  bus (now corrected, README.md, to the real per-transceiver figures — see
  `TODO.md` 1.6) could deliver frames at.

Firmware placement (which CMAC key slot, whether it's the KEYSTORE-resident
session key derived from the Trust M's ECDHE handshake) is `TODO.md` 7.2
scope, not decided here — this document fixes *which engine*, not the
implementation.

---

## 4.8 — Fleet key lifecycle and revocation

**`JUDGMENT CALL`, partially open.** `TODO.md` 4.8 correctly notes the
constraint: [45] p.10 Fig. 1 gives exactly **4 X.509 certificate slots and 3
trust-anchor slots** — no room for a conventional, growing CRL stored on-part.

Decisions that follow directly from the object-model limits:

1. **Revocation cannot be a growing list stored on the servo.** With 4 cert
   slots and 3 trust-anchor slots, any revocation scheme must be
   short-list/allowlist shaped (e.g. "the fleet controller only continues to
   trust servos whose certificate serial appears in a controller-side
   allowlist synced at commissioning"), not a servo-side CRL that grows
   without bound. This pushes revocation state to the fleet
   controller/gateway, not the servo — consistent with the Trust M being "a
   key vault, not platform attestation"
   (`OPTIGA-Trust-M-secure-element.md` §1).
2. **The identity key itself cannot be revoked-and-reissued in the field**
   (already recorded, `TODO.md` 4.10 / [45] p.11 §2 Note: fab-provisioned,
   customer-CA-signed). Revocation therefore means "the fleet controller
   stops trusting this serial," never "the servo gets a new identity key."
3. **Retiring a compromised servo from a daisy chain** is a bus-topology
   question this document cannot resolve alone: RS-485/CAN-FD daisy chains
   (README.md) mean a compromised node can still physically pass traffic
   even if logically distrusted, unless the protocol design gives the
   controller (or neighboring servos) a way to refuse to relay its frames.
   **`OPEN`:** this needs the eventual bus-protocol design from `TODO.md` 7,
   which does not exist yet — recorded here so it isn't rediscovered
   independently of this constraint, per `AGENTS.md`'s intent for `TODO.md`.
4. **Provisioning-time process**: revocation-list distribution and the
   platform-binding-secret provisioning in 4.4 should happen in the same
   commissioning step, since both are "day-one, fleet-controller-side state
   about this specific servo" — recorded as a process linkage, not a
   protocol requirement.

---

## 4.9 — ECC curve selection

**Decision: NIST P-256 for device identity and ECDHE, minimum; P-384 where
performance allows. RSA and P-521/Brainpool are explicitly not selected.**

- [45] pp. 8–9, Table 4 — the V3 crypto set includes ECC P-256/384/521,
  Brainpool P256/384/512 r1, and **RSA 1024/2048**. `TODO.md` 4.9 already
  correctly flags RSA-1024 as unacceptable (56-bit-class security margin by
  modern standards) — **this document reaffirms that RSA is not selected for
  this design at all** (ECDSA/ECDHE only), which is a stronger constraint
  than just excluding the 1024-bit RSA option, and removes RSA's much higher
  latency (Table 18: RSA-2048 sign ~310 ms vs. ECDSA-P256 ~65 ms, both with
  Shielded Connection off) from consideration entirely.
- **P-256 is the floor** (matches [46] p.88 §8.18's own CSC secure-boot
  choice of "software ECDSA P-256," so the MCU side is already committed to
  P-256 tooling — using the same curve on the Trust M side avoids a
  mismatched-curve integration burden between the two parts).
- **P-384 preferred where the 4.4/4.7 latency budget allows** — `TODO.md`
  4.9 states this preference; this document does not have per-curve Trust M
  latency figures beyond P-256 (Table 18 only tabulates P-256 numbers) to
  quantify the P-384 cost, so the "preferred" framing is carried forward
  as-is rather than sharpened into a firm requirement. `UNVERIFIED — needs
  primary source`: P-384-specific Trust M performance figures are not in
  [45]'s published tables; would need Infineon's own P-384 benchmark data or
  bench measurement before firmly committing to it over P-256 everywhere.
- Brainpool curves are supported by the part ([45] Table 4) but are not
  selected here — `JUDGMENT CALL`: no requirement in this repo calls for
  Brainpool, and standardizing on NIST P-256/P-384 keeps the MCU-side ECDSA
  P-256 tooling ([46] §8.18) and the Trust M side on the same curve family.

---

## 4.10 — No PQC path (status check only, no new decision)

Already fully recorded in `TODO.md` 4.10 itself, with a correct citation
([45] p.11 §2 Note on fab-provisioned identity keys). Re-verified in this
pass: neither [46] (MCU) nor [45] (Trust M) datasheet lists any ML-KEM/
ML-DSA/SLH-DSA or other PQC primitive anywhere in their feature lists (§8.18/
Table 4 respectively — checked directly, not from memory). No new finding;
marking `TODO.md` 4.10 closed as a recorded, accepted limitation rather than
an open action item, per its own text ("Accepted limitation of the device
class; recorded so it is not rediscovered").

---

## New follow-up items opened by this pass

- **1.4.f** (`REFERENCES.md`/`TODO.md`) — intake the OPTIGA™ Trust M
  *Solution Reference Manual* (referenced by [45] p.10 as shipped "as part of
  the package," not currently in `PCB/datasheets/`) before implementing 4.4's
  Shielded Connection pairing procedure.
- Bus noise-coupling risk flagged in 4.6 (shared `Vmot`/`GND` on the
  daisy-chain connector) is already tracked in
  `PCB/MSPM0G3518-MCU-swap.md` §6.4 — cross-referenced here, not duplicated
  as a new item.

---

*Written by Claude Sonnet 5 (`claude-sonnet-5`) under human direction,
2026-08-22, from the local Trust M ([45]) and MSPM0G351x-Q1 ([46], [49])
datasheets. Every citation above was read from the local PDF copies in this
session; nothing here is reproduced from model memory. Items marked
`JUDGMENT CALL` are explicitly not derived from a cited standard, per
`AGENTS.md` §4.*
