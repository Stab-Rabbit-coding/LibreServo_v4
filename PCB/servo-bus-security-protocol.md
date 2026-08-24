# Servo bus security protocol — design decisions (TODO.md §4.4–4.10)

**Date:** 2026-08-22; §4.4 and §4.7 substantially revised 2026-08-23 on intake
of the OPTIGA™ Trust M Solution Reference Manual [53].
**Status:** §4.4, §4.5, §4.7 and §4.9 are decided **and implemented** in
[`firmware/`](../firmware/) (`TODO.md` 7.2). §4.6's threshold and §4.8's
daisy-chain question still wait on the control-loop and bus-protocol design of
`TODO.md` 7.1, which does not exist yet.
**Governing rules:** [`AGENTS.md`](../AGENTS.md). Every claim below traces to
[`REFERENCES.md`](../REFERENCES.md) or is marked `UNVERIFIED`/`OPEN`/
`JUDGMENT CALL` (the last per `AGENTS.md` §4: "if a decision is a judgment
call with no governing standard, say so explicitly").

The Trust M's security-monitor budget governs everything below. **Its
characterization in this repository was corrected 2026-08-23** — see §4.7, and
[`OPTIGA-Trust-M-secure-element.md`](OPTIGA-Trust-M-secure-element.md) §4.

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

**RESOLVED 2026-08-23.** The Solution Reference Manual is now intaken as
**[53]** (`PCB/datasheets/OPTIGA_Trust_M_Solution_Reference_Manual_v3.70.pdf`,
fetched from Infineon's own GitHub organization — see `TODO.md` 1.4.f), and the
provisioning procedure it defines is implemented in
[`firmware/trust/ls_trust_pairing.c`](../firmware/trust/ls_trust_pairing.c).
The paragraph that used to stand here said the procedure "is not in the
datasheet ... Do not implement firmware-side Shielded Connection pairing from
memory of similar protocols; intake that document first."  That instruction was
followed; what follows is read from [53] and [56], not reconstructed.

### 4.4.1 — Why pairing is mandatory, not a hardening option

[56] §2, pp. 5–7, Table 1, row "0xE140 – Platform binding secret" records that
on an **OPTIGA™ Trust M V3** part — the variant on this BOM, sales code
SLS 32AIA010ML ([45] p. 8, Table 2) — the object ships as:

| Field | As shipped (V3) |
| --- | --- |
| Life cycle state (LcsO) | Creation |
| Value | **Default** |
| Read AC | **ALW** |
| Change AC | `LcsO < operational \|\| Conf(0xE140)` |

A **default value with always-readable access is a published constant**. Until
pairing has run, the Shielded Connection is keyed by something anyone can look
up, and it protects nothing at all. This is the single most consequential fact
the SRM intake produced: it converts 4.4 from "enable a feature" into "execute a
manufacturing step, without which the part is decorative."

(Express and MTR variants ship 0xE140 already Operational with a chip-unique
value and read AC NEV — paired at Infineon, secret retrieved from CIRRENT™
Cloud ID. This design does not use those variants.)

### 4.4.2 — The procedure

[53] §2.3.4, p. 20, Figure 12, "Pair OPTIGA™ Trust M with host (pre-shared
secret based)". Pre-condition: 0xE140 is not locked, LcsO below operational.
Post-condition: the secret is present on both sides and locked.

1. Create `optiga_util` and `optiga_crypt` instances at
   `OPTIGA_COMMS_NO_PROTECTION` with protocol version
   `OPTIGA_COMMS_PROTOCOL_VERSION_PRE_SHARED_SECRET`. There is no secret yet, so
   there is nothing to protect the channel with.
2. Open the application.
3. Read 0xE140's metadata and check LcsO. If it is already operational, stop —
   the part was paired before, and whether that is benign depends entirely on
   whether the host still holds the matching secret.
4. Generate the secret with `optiga_crypt_random(OPTIGA_RNG_TYPE_TRNG, ...)`.
   **64 bytes**, the maximum the host library allows: [53] §6.5.8, p. 107
   recommends "32 bytes or more", and there is no reason to take the floor.
   Drawn from the OPTIGA rather than the MCU because [45] p. 1 makes the OPTIGA
   a Common Criteria EAL6+ (high) certified device and its RNG carries that
   evaluation; the MSPM0's entropy source carries no equivalent certification
   in [46].
5. Write it to 0xE140 (`OPTIGA_UTIL_ERASE_AND_WRITE`).
6. **Store it on the host, and confirm the store, *before* step 7.**
7. Write 0xE140's final metadata: raise LcsO to operational and set read AC to
   `LcsO < operational`, i.e. permanently unreadable.
8. Close the application.

**The step 6/7 ordering is this design's own, and it is not the ordering the
upstream reference example uses.** Step 7 is irreversible — [53] §5.3, p. 96,
Table 74 states the four life-cycle states "only progress in one direction from
a lower value to a higher value." If the host store had failed after locking,
the two parts would be unpaired with no way back and no diagnostic. Locking
last means a host-store failure leaves 0xE140 still writable, so the line can
retry or scrap cleanly.

**The final life-cycle state is operational, also a divergence.** The upstream
example ships with `FINAL_LCSO_STATE` set to *creation* and a comment that "at
the real time/customer side this needs to be LCSO_STATE_OPERATIONAL". Leaving a
shipped unit at creation would be unacceptable: the change AC permits
`LcsO < operational`, so anything that can reach the I²C bus could rewrite the
binding secret and re-pair the part to itself.

The change AC keeps its `Conf(0xE140)` branch — [56] §2, p. 7 defines `Conf(X)`
as "the action is only possible in case the data involved ... are
confidentiality protected with key given by X. This enforces the shielded
connection" — which is what leaves runtime secret rotation ([53] §2.3.6, p. 21,
Figure 14) possible later. [53] §6.5.8 recommends that rotation; [53] §5.1's NVM
budget (2 million tearing-safe programming cycles across all objects, and
data retention declining toward ½ year beyond about 40 000 cycles of an object)
is why it must be scheduled rather than done casually. Rotation is not
implemented — `TODO.md` 4.14.

### 4.4.3 — Where the host keeps the secret: a correction

The paragraph below this section used to say the secret would be deposited
"into both `U7`'s platform-binding-secret slot and the MCU's KEYSTORE at
manufacture." **The KEYSTORE cannot hold it**, for two independent reasons read
2026-08-23:

1. [46] §8.21, p. 89 describes the Keystore controller's use model as depositing
   keys "and have the AES engine access them subsequently in a secure manner
   without leaking any key data to observers", holding "128 and 256-bit keys".
   It is a write-then-use-by-AESADV store; software does not read key material
   back out of it.
2. The host library requires plaintext readback. [53] §6.6.1, p. 108 states that
   during Shielded Connection establishment "the `optiga_comms_ifx_i2c` module
   invokes `pal_os_datastore_read`", and the secret is up to 64 bytes — not an
   AES key width.

The secret must therefore live in MCU non-volatile memory that firmware can
read, protected by the MSPM0's flash and debug protections rather than by a key
store. Selecting that protection — static write protection, flash read-out
protection, and the one-way NONMAIN debug lockdown of [49] §§2.6/3.2 — is
`TODO.md` 4.13 and must be settled before any unit is provisioned.

The KEYSTORE remains exactly the right home for the **derived per-session bus
CMAC key** of §4.7: that *is* an AES key, it is used only by AESADV, and it is
never read back. Two different secrets with two different lifetimes; conflating
them was the original error.

### 4.4.4 — The cost of this decision, which was not visible in [45]

Defining `OPTIGA_COMMS_SHIELDED_CONNECTION` makes three **host-side**
cryptographic primitives a link-time requirement:
`pal_crypt_tls_prf_sha256`, `pal_crypt_encrypt_aes128_ccm` and
`pal_crypt_decrypt_aes128_ccm`. The presentation layer of the IFX I²C protocol
[54] protects each APDU with AES-128-CCM under a key derived from the platform
binding secret with the TLS 1.2 PRF, and that work happens on the MCU.

This is a real cost of enabling the Shielded Connection and it appears nowhere
in [45]. It is implemented in
[`firmware/pal/ls_pal_crypt.c`](../firmware/pal/ls_pal_crypt.c) from [58]
RFC 5246 §5 and [59] NIST SP 800-38C §§6.1–6.2 and Appendix A, and verified
against independent implementations — see
[`firmware/tests/README.md`](../firmware/tests/README.md). The AES block cipher
and HMAC-SHA256 beneath it are deliberately not implemented in this project;
`TODO.md` 7.6 tracks binding them to a vetted source.

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

- **Corrected and strengthened 2026-08-23 from [53] §4.6, which the datasheet
  only summarises.** This document previously wrote the constraint as "one
  protected operation per 5 s `t_max`, hard security-monitor limit." That
  overstates it, and the correction matters because the overstatement would rule
  out things this design actually needs to do:

  - [53] §4.6.2, p. 74 defines *one protected operation per `t_max`* as the
    **permitted sustained usage profile**, not an instantaneous gate.
  - [53] §4.6.4, pp. 76–77 gives the enforcement: a delay "starts as soon as SEC
    reaches the value of 128" and grows to `t_max` only at SEC = 255. Below
    SEC = 128 there is **no delay at all**.
  - The SEC/SEC_CREDIT mechanism of [53] §4.6.2 lets credit accumulate while the
    part is idle (SEC_CREDIT_MAX default 5), and an event consumes credit before
    it ever increments SEC.
  - **[53] §4.6.1, p. 74, Table 65 is the load-bearing detail**: each of the
    "Private key use", "Secret key use" and "Key derivation" events carries an
    explicit carve-out for *temporary keys held in a session context*. Work done
    against a session context spends no budget.

  So a short burst of protected operations at boot is entirely normal.
  **The conclusion of this section is unchanged and better supported**: a
  control loop authenticating every frame at tens of Hz to kHz would drive SEC
  to its ceiling and be throttled into failure, so the Trust M cannot sit in the
  per-frame path. What changes is that the boot-time trust sequence — one
  identity signature plus one ephemeral key generation, two security events
  total, everything after that on a session context — is comfortably affordable,
  which the old framing implied it was not.

  Implemented accordingly: [`firmware/trust/ls_trust.c`](../firmware/trust/ls_trust.c)
  reads OID 0xE0C5 (SEC) before spending budget and reports
  `LS_TRUST_ERR_THROTTLED` rather than issuing an operation into a throttle;
  [`firmware/trust/ls_trust_oid.h`](../firmware/trust/ls_trust_oid.h) carries the
  thresholds with their citations.
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

- ~~**1.4.f** — intake the OPTIGA™ Trust M *Solution Reference Manual*~~
  **CLOSED 2026-08-23.** Intaken as [53], along with [54]–[56]; see
  `TODO.md` 1.4.f and §4.4 above.
- **4.13** — decide and configure where the MCU stores the platform binding
  secret (§4.4.3). Blocks provisioning of any unit, because MCU-side NONMAIN
  lockdown is a one-way per-unit door ([49] §§2.6/3.2).
- **4.14** — implement runtime platform-binding-secret rotation ([53] §2.3.6,
  Figure 14), which §4.4.2's access-condition choice deliberately leaves open.
- **7.6** — bind `firmware/pal/ls_crypto_backend.h` to a vetted AES and
  HMAC-SHA256 implementation (§4.4.4).
- Bus noise-coupling risk flagged in 4.6 (shared `Vmot`/`GND` on the
  daisy-chain connector) is already tracked in
  `PCB/MSPM0G3518-MCU-swap.md` §6.4 — cross-referenced here, not duplicated
  as a new item.

---

*Written by Claude Sonnet 5 (`claude-sonnet-5`) under human direction,
2026-08-22, from the local Trust M ([45]) and MSPM0G351x-Q1 ([46], [49])
datasheets; §4.4 and §4.7 revised and the follow-up list updated by Claude
Opus 5 (`claude-opus-5`) under human direction, 2026-08-23, from the newly
intaken [53]–[57]. Every citation above was read from the local PDF copies in this
session; nothing here is reproduced from model memory. Items marked
`JUDGMENT CALL` are explicitly not derived from a cited standard, per
`AGENTS.md` §4.*
