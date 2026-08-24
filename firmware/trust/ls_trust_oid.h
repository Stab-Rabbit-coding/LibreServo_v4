/**
 * @file    ls_trust_oid.h
 * @brief   OPTIGA(TM) Trust M object identifiers used by LibreServo v4, and the
 *          security-monitor facts that govern how often they may be touched.
 *
 * Every OID and access condition below was read from [53] section 5.4, pp. 85-87,
 * Table 68 (common data objects) and Table 69 (common key objects), and from
 * [56] section 2, Table 1, in this session.
 *
 * References (see REFERENCES.md):
 *   [45] Infineon, OPTIGA(TM) Trust M Datasheet, Rev 3.70.
 *   [53] Infineon, OPTIGA(TM) Trust M Solution Reference Manual, Rev 3.70.
 *   [56] Infineon, OPTIGA(TM) Trust M configurations, Configuration Guide, Rev 2.2.
 *
 * Written by Claude Opus 5 (claude-opus-5) under human direction, 2026-08-23.
 */

#ifndef LS_TRUST_OID_H_
#define LS_TRUST_OID_H_

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------
 * Data objects — [53] section 5.4, pp. 85-86, Table 68
 * ------------------------------------------------------------------------ */

/** Coprocessor UID.  Read AC = ALW, change AC = NEV.  The part's own immutable
 *  identifier; this design uses it as the servo's hardware serial number so the
 *  fleet allowlist of PCB/servo-bus-security-protocol.md section 4.8 keys on
 *  something that cannot be rewritten in the field. */
#define LS_OID_COPROCESSOR_UID          (0xE0C2U)

/** Security Event Counter (SEC).  Read AC = ALW, change AC = NEV.
 *  Reading this is how firmware observes its own remaining budget against the
 *  throttle of [53] section 4.6.4 — see #LS_TRUST_SEC_THROTTLE_THRESHOLD. */
#define LS_OID_SECURITY_EVENT_COUNTER   (0xE0C5U)

/** Security Monitor Configurations.  [53] section 5.6, p. 97, Table 77 gives the
 *  byte layout: offset 0 is t_max in ms/100 (default 50 = 5 s), offset 2 is
 *  SEC_CREDIT_MAX (default 5), offset 3 is the delayed SEC decrement sync count.
 *
 *  This design does NOT write this object.  [53] section 4.6.3 notes that setting
 *  t_max to 0 "disables the security monitor" outright, which would remove the
 *  anti-brute-force throttle that is a large part of why a certified secure
 *  element was chosen over a plain MCU key store.  Defined here so the OID is
 *  recognised in a dump, not so it gets configured. */
#define LS_OID_SECURITY_MONITOR_CONFIG  (0xE0C9U)

/** Device certificate 1, issued by Infineon.  Read AC = ALW.
 *  [56] section 1, p. 4: on V1/V3 the default PKI puts an ECC NIST P-256 end-device
 *  certificate here and its private key in 0xE0F0. */
#define LS_OID_DEVICE_CERTIFICATE       (0xE0E0U)

/** Root CA trust anchor slots 1-2.  Change AC = LcsO < operational.
 *  Where the fleet controller's CA lands at commissioning. */
#define LS_OID_TRUST_ANCHOR_1           (0xE0E8U)
#define LS_OID_TRUST_ANCHOR_2           (0xE0E9U)

/** Monotonic counters 1-4.  [53] section 5.1 caps each at 600 000 updates.
 *  PCB/servo-bus-security-protocol.md section 4.5 records the decision NOT to use
 *  these for per-frame freshness; they are listed for completeness. */
#define LS_OID_MONOTONIC_COUNTER_1      (0xE120U)

/** Shared platform binding secret.  The Shielded Connection key.
 *  [53] section 5.4 Table 68: change AC = "LcsO < op || Conf (0xE140)". */
#define LS_OID_PLATFORM_BINDING_SECRET  (0xE140U)

/** Arbitrary data object, type 3 (140 bytes, [53] section 5.7, p. 97, Table 79).
 *  On a V3 part [56] Table 1 shows 0xF1D0's object type as "Not configured"
 *  (it is AUTOREF only on Express/MTR), so it is free for this design's use. */
#define LS_OID_ARBITRARY_DATA_0         (0xF1D0U)

/* ------------------------------------------------------------------------
 * Key objects and session contexts — [53] section 5.4, p. 87, Table 69
 * ------------------------------------------------------------------------ */

/** Device private ECC key 1.  Read AC = NEV, change AC = NEV — it can never
 *  leave the part and can never be replaced ([45] p. 11 section 2 Note: fab
 *  provisioned).  This is the servo's identity key; TODO.md 4.10 records the
 *  accepted consequence that it cannot be rotated in the field. */
#define LS_OID_DEVICE_PRIVATE_KEY       (0xE0F0U)

/** Session contexts 1-4.  Not addressable by GetDataObject/SetDataObject; they
 *  hold ephemeral private keys and shared secrets for toolbox commands.
 *
 *  Using a session context rather than a persistent object for the ECDH result
 *  is what keeps the handshake out of the "Key derivation" and "Private key
 *  use" security events of [53] section 4.6.1, Table 65 — every one of those
 *  entries carves out "temporary keys from the session context". */
#define LS_OID_SESSION_CONTEXT_1        (0xE100U)

/* ------------------------------------------------------------------------
 * Security-monitor budget — [53] section 4.6, pp. 74-77
 * ------------------------------------------------------------------------ */

/**
 * SEC value at which the security monitor begins delaying protected operations.
 *
 * [53] section 4.6.4, p. 76: "the delay starts as soon as SEC reaches the value of
 * 128 and will be t_max in case the SEC reaches its maximum value of 255."
 *
 * This constant exists to correct a persistent misreading of the part, recorded
 * here so it does not recur: the budget is NOT a flat "one protected operation
 * per 5 seconds, always."  Per [53] section 4.6.2 that is the *permitted sustained
 * usage profile*, enforced through the SEC/SEC_CREDIT mechanism — a short burst
 * of protected operations at boot is entirely normal and incurs no delay at all
 * while SEC stays below 128.
 */
#define LS_TRUST_SEC_THROTTLE_THRESHOLD (128U)

/** SEC value at which the delay per protected operation equals t_max.
 *  [53] section 4.6.4, p. 76. */
#define LS_TRUST_SEC_MAX                (255U)

/** Default t_max, in milliseconds.  [53] section 4.6.2, p. 74: "t_max is
 *  configurable, and default value is 5 seconds (+/- 5%)"; [53] section 5.6
 *  Table 77 encodes it as ms/100 with default 50. */
#define LS_TRUST_TMAX_MS                (5000U)

/**
 * SEC reading at or above which this design stops issuing protected operations
 * and reports degraded trust to the application.
 *
 * JUDGMENT CALL — no cited source sets this number; [53] only defines where the
 * hardware throttle begins (128).  A margin below the hardware threshold is
 * chosen so that firmware notices and reports the condition before the part
 * starts silently adding multi-second delays to a servo's boot, which on a
 * safety-relevant actuator is a failure worth surfacing rather than absorbing.
 * The value is deliberately conservative and is a tuning parameter, not a
 * derived constant.
 */
#define LS_TRUST_SEC_WARNING_LEVEL      (64U)

#ifdef __cplusplus
}
#endif

#endif /* LS_TRUST_OID_H_ */
