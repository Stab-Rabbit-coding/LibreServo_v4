/**
 * @file    ls_optiga_lib_config.h
 * @brief   LibreServo v4.0.0 build configuration for the Infineon OPTIGA(TM)
 *          Trust M Host Library for C.
 *
 * Selected through the upstream hook, not by shadowing a header: build with
 *     -DOPTIGA_LIB_EXTERNAL="\"ls_optiga_lib_config.h\""
 * which [57] `include/optiga_lib_config.h` honours in preference to its own
 * `optiga_lib_config_m_v1.h` / `optiga_lib_config_m_v3.h` defaults.  Using the
 * documented hook means an upstream bump cannot silently reinstate the default
 * profile the way include-path ordering could.
 *
 * This file is a NARROWING of the upstream V3 profile
 * (`include/optiga_lib_config_m_v3.h` in [57]): every macro here either appears
 * upstream with the same meaning, or is deliberately *not* defined so the
 * corresponding feature is compiled out.  Nothing is invented.
 *
 * Attribution chain (AGENTS.md section 1; MIT license retention clause):
 *   Derived from `include/optiga_lib_config_m_v3.h` of the
 *   OPTIGA(TM) Trust M Host Library for C,
 *   Copyright (c) 2018-2024 Infineon Technologies AG, SPDX-License-Identifier: MIT,
 *   at commit 67cfd0e589fc09f936d4e4ce4fa35eacc43af72f ([57]).
 *   LibreServo-specific narrowing authored for this project; see the per-macro
 *   rationale comments and the TODO.md items each traces to.
 *
 * References (see REFERENCES.md):
 *   [45] Infineon, OPTIGA(TM) Trust M Datasheet, Rev 3.70.
 *   [46] TI, MSPM0G3519-Q1/MSPM0G3518-Q1 Datasheet, SLASFA6B.
 *   [53] Infineon, OPTIGA(TM) Trust M Solution Reference Manual, Rev 3.70.
 *   [56] Infineon, OPTIGA(TM) Trust M configurations, Configuration Guide, Rev 2.2.
 *   [57] Infineon, OPTIGA(TM) Trust M Host Library for C, Ver 5.8.1.
 *
 * Design decisions encoded here are recorded in
 * `PCB/servo-bus-security-protocol.md` sections 4.4, 4.7 and 4.9.
 *
 * Written by Claude Opus 5 (claude-opus-5) under human direction, 2026-08-23.
 */

#ifndef LS_OPTIGA_LIB_CONFIG_H_
#define LS_OPTIGA_LIB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------
 * Cryptographic feature selection
 *
 * The enabled set is exactly what the LibreServo trust layer calls, and no
 * more.  Compiling out the rest is not cosmetic: every enabled toolbox command
 * is reachable over the I2C interface, so a narrower build is a smaller
 * attack surface on a part whose bus is board-accessible.
 * ------------------------------------------------------------------------ */

/** TRNG.  Needed to generate the platform binding secret at pairing time
 *  ([53] section 2.3.4, step "Generate Random ... TRNG") and to generate the
 *  host nonce used for freshness ([53] section 6.6.3). */
#define OPTIGA_CRYPT_RANDOM_ENABLED

/** SHA-256.  Needed to digest the challenge before `optiga_crypt_ecdsa_sign`,
 *  which per [57] `optiga_crypt.h` takes a digest, not a message. */
#define OPTIGA_CRYPT_HASH_ENABLED

/** Ephemeral ECC key pair generation for the session handshake
 *  ([53] section 4.4.3 Table 64: ~55 ms, ECC NIST P-256, ephemeral key). */
#define OPTIGA_CRYPT_ECC_GENERATE_KEYPAIR_ENABLED

/** ECDSA sign with the fab-provisioned device identity key in 0xE0F0.  This is
 *  the servo proving its identity; [45] p.11 section 2 Note records that this key
 *  is fab-provisioned and cannot be replaced in the field (TODO.md 4.10). */
#define OPTIGA_CRYPT_ECDSA_SIGN_ENABLED

/** ECDSA verify.  Used to authenticate the fleet controller's signature and,
 *  during commissioning, to check a peer certificate against a trust anchor. */
#define OPTIGA_CRYPT_ECDSA_VERIFY_ENABLED

/** ECDH.  The session-key agreement half of the boot handshake
 *  ([53] section 4.4.3 Table 64: CalcSSec, ECDH SP 800-56A, ~60 ms, P-256). */
#define OPTIGA_CRYPT_ECDH_ENABLED

/** TLS v1.2 PRF SHA-256 / SHA-384 for deriving the per-session bus CMAC key
 *  from the ECDH shared secret held in a session context.
 *  [53] section 4.4.3 Table 64 gives ~50 ms for a 40-byte derived key from a
 *  32-byte session-context secret.  Deriving from a SESSION CONTEXT rather
 *  than a persistent data object also keeps the operation out of the
 *  "Key derivation" security event of [53] section 4.6.1 Table 65, which is
 *  scoped explicitly to DeriveKey "applied on a persistent data object". */
#define OPTIGA_CRYPT_TLS_PRF_SHA256_ENABLED
#define OPTIGA_CRYPT_TLS_PRF_SHA384_ENABLED
/* SHA-512 PRF is enabled only because upstream forces it, not because this
 * design uses it.  Second upstream guard bug found while building this port
 * 2026-08-23 against [57] at commit 67cfd0e58: `src/crypt/optiga_crypt.c`
 * wraps all three `optiga_crypt_tls_prf_shaXXX` bodies (lines 506-642) in one
 * combined `#if defined(SHA256) || defined(SHA384) || defined(SHA512) ||
 * defined(HKDF)` block, while the enum values those bodies reference
 * (`OPTIGA_TLS12_PRF_SHA_384`, `OPTIGA_TLS12_PRF_SHA_512` in
 * `include/common/optiga_lib_common.h` lines 241 and 245) are each guarded by
 * their OWN macro.  Enabling any subset therefore fails to compile with
 * "'OPTIGA_TLS12_PRF_SHA_512' undeclared".  The upstream V3 default profile
 * enables all three, which is why the coupling is not otherwise visible.
 * Unlike the CLEAR_AUTO_STATE case below, there is no way to avoid this one
 * without giving up TLS PRF entirely, which the session-key derivation of
 * `ls_trust_establish_session_key()` needs.  Enabling a stronger PRF variant
 * is not a weakening.  Tracked with the other finding as TODO.md 7.5. */
#define OPTIGA_CRYPT_TLS_PRF_SHA512_ENABLED

/* OPTIGA_CRYPT_CLEAR_AUTO_STATE_ENABLED is deliberately NOT defined, for two
 * independent reasons:
 *
 * 1. Design: `optiga_crypt_clear_auto_state` only means anything when an
 *    authorization-reference (AUTOREF) object is in use.  [56] section 2 Table 1
 *    shows 0xF1D0's object type as "Not configured" on an OPTIGA Trust M **V3**
 *    part — AUTOREF is provisioned only on the Express and MTR variants — so
 *    this design has no AUTO state to clear.  The guidance of [53] section 6.5
 *    ("the AUTO state ... need[s] to be cleared manually") applies to designs
 *    that establish one; this is not one.
 *
 * 2. Upstream header bug, found while building this port 2026-08-23 against
 *    [57] at commit 67cfd0e589fc09f936d4e4ce4fa35eacc43af72f:
 *    `include/cmd/optiga_cmd.h` line 596 guards `optiga_cmd_decrypt_sym` with
 *      `#if defined(OPTIGA_CRYPT_SYM_DECRYPT_ENABLED)
 *          || defined(OPTIGA_CRYPT_HMAC_VERIFY_ENABLED)
 *          || defined(OPTIGA_CRYPT_CLEAR_AUTO_STATE_ENABLED)`
 *    while its parameter type `optiga_decrypt_sym_params_t` is defined in
 *    `include/common/optiga_lib_common.h` line 524 under only
 *      `#if defined(OPTIGA_CRYPT_SYM_ENCRYPT_ENABLED)
 *          || defined(OPTIGA_CRYPT_SYM_DECRYPT_ENABLED)`.
 *    Enabling CLEAR_AUTO_STATE without also enabling a symmetric feature
 *    therefore fails to compile with "unknown type name
 *    'optiga_decrypt_sym_params_t'".  Working around it by enabling
 *    OPTIGA_CRYPT_SYM_DECRYPT_ENABLED would widen the command surface this
 *    configuration exists to narrow, so it is not done.  Tracked as
 *    TODO.md 7.5 (report upstream). */

/* ------------------------------------------------------------------------
 * Deliberately NOT enabled — each omission is a recorded design decision,
 * not an oversight.  Listed so a future reader does not "restore" them.
 * ------------------------------------------------------------------------ */

/* OPTIGA_CRYPT_RSA_* — RSA is excluded from this design entirely
 * (`PCB/servo-bus-security-protocol.md` section 4.9, TODO.md 4.9).  [45] pp. 8-9
 * Table 4 offers RSA 1024/2048; neither is selected.  [53] section 4.4.3 Table 64
 * also puts RSA-2048 key generation at "minimum 2900 ms", which is far outside
 * any plausible boot budget for this servo. */

/* OPTIGA_CRYPT_ECC_NIST_P_521_ENABLED, OPTIGA_CRYPT_ECC_BRAINPOOL_P_R1_ENABLED
 * — the curve set is pinned to NIST P-256 (floor) and P-384
 * (`PCB/servo-bus-security-protocol.md` section 4.9).  Brainpool and P-521 are
 * supported by the part ([45] Table 4) but are not selected. */

/* OPTIGA_CRYPT_SYM_ENCRYPT_ENABLED, OPTIGA_CRYPT_SYM_DECRYPT_ENABLED,
 * OPTIGA_CRYPT_SYM_GENERATE_KEY_ENABLED, OPTIGA_CRYPT_HMAC_ENABLED,
 * OPTIGA_CRYPT_HMAC_VERIFY_ENABLED, OPTIGA_CRYPT_HKDF_ENABLED,
 * OPTIGA_CRYPT_GENERATE_AUTH_CODE_ENABLED — per-frame symmetric work runs on
 * the MCU's own AESADV/CMAC engine, never on U7 ([46] sections 8.18/8.20;
 * `PCB/servo-bus-security-protocol.md` section 4.7).  Every one of these
 * commands would raise a "Secret key use" security event on U7
 * ([53] section 4.6.1 Table 65), which is exactly the budget this design
 * spends only at boot. */

/* EXAMPLE_OPTIGA_UTIL_PROTECTED_UPDATE_* — the upstream protected-update
 * example macros.  Not example code here; protected update of U7 objects is
 * not part of the current firmware scope (TODO.md 4.8 open). */

/* ------------------------------------------------------------------------
 * Shielded Connection — TODO.md 4.4
 * ------------------------------------------------------------------------ */

/** Compile in the Shielded Connection ([53] section 6.6.2 "Usage": "the Shielded
 *  Connection feature can be enabled/disabled using the macro
 *  (OPTIGA_COMMS_SHIELDED_CONNECTION in optiga_lib_config.h)").
 *
 *  This is not optional for this design.  [56] section 2 Table 1 records that on
 *  an OPTIGA(TM) Trust M **V3** part — the variant on this BOM, sales code
 *  SLS 32AIA010ML, [45] p.8 Table 2 — the platform binding secret object 0xE140
 *  ships with life-cycle state Creation, a **Default** value and read access
 *  **ALW**.  Without pairing and without the Shielded Connection, every byte on
 *  SE_I2C_SDA/SE_I2C_SCL is in clear on a board-accessible bus. */
#define OPTIGA_COMMS_SHIELDED_CONNECTION

/** Default protection level applied to every optiga_util / optiga_crypt call
 *  that does not override it.
 *
 *  Upstream defaults this to OPTIGA_COMMS_NO_PROTECTION.  This design raises it
 *  to full (command AND response) protection, per TODO.md 4.4's decision to
 *  "enable Shielded Connection for all I2C traffic to U7".  Choosing the
 *  fail-closed default matters because [53] section 6.6.2 states the per-call
 *  protection setting "reset[s] automatically to the default protection level
 *  ... once after the operation is invoked" — so anything that forgets to set a
 *  level inherits this one rather than silently dropping to clear.
 *
 *  The pairing sequence is the one deliberate exception and overrides this per
 *  call (see `firmware/trust/ls_trust_pairing.c`): before the secret exists
 *  there is nothing to key the protection with. */
#define OPTIGA_COMMS_DEFAULT_PROTECTION_LEVEL OPTIGA_COMMS_FULL_PROTECTION

/* ------------------------------------------------------------------------
 * Reset strategy
 * ------------------------------------------------------------------------ */

/** Reset type used by `optiga_comms_open`.  [57] `optiga_lib_config_m_v3.h`
 *  documents the encoding: 0 = cold (host has GPIO for both RST and VDD),
 *  1 = soft (host has neither), 2 = warm (host has RST but no VDD control).
 *
 *  This board is case 2.  U7's RST is driven from U1 pin 18 (PA14) via the
 *  SE_RST net, but VCC is a hard-wired +3V3 rail with only decoupling (C43) —
 *  there is no load switch, so VDD cannot be cycled in firmware
 *  (`PCB/OPTIGA-Trust-M-secure-element.md` section 2).
 *
 *  Independently, [53] section 4.6.4 p.77 caps VCC off/on cycling at 200 000 times
 *  over the part's lifetime and advises against removing power before the
 *  security event counter has returned to 0 — so a cold-reset strategy would be
 *  undesirable here even if the hardware allowed it. */
#define OPTIGA_COMMS_DEFAULT_RESET_TYPE (2U)

/* ------------------------------------------------------------------------
 * Library housekeeping — upstream values, retained deliberately
 * ------------------------------------------------------------------------ */

/** NULL parameter checking in the library.  Kept enabled: this is a
 *  safety-relevant actuator and the code-size cost is trivial against the
 *  256 KB flash of the MSPM0G3518-Q1 ([46] "Features"). */
#define OPTIGA_LIB_DEBUG_NULL_CHECK

/** Maximum number of concurrently registered optiga_util/optiga_crypt
 *  instances.  Upstream default 0x06; this design registers two (one util,
 *  one crypt) but the value is left at the upstream default rather than
 *  tightened, because `optiga_cmd` sizes internal state from it and a
 *  narrower value has not been validated against the library. */
#define OPTIGA_CMD_MAX_REGISTRATIONS (0x06)

/** Communication buffer.  Upstream default 0x615 (1557 bytes), which matches
 *  the part's own "Maximum Com Buffer Size" object 0xE0C6 default of 0x0615
 *  ([53] section 5.4 Table 68).  Retained: the MSPM0G3518-Q1 has 128 KB of SRAM
 *  ([46] "Features": "MSPM0G3518-Q1: 256KB flash, 128KB RAM"), so this buffer
 *  is about 1.2 % of RAM and there is no reason to shrink it below what the
 *  part will negotiate. */
#define OPTIGA_MAX_COMMS_BUFFER_SIZE (0x615)

/** Call `pal_init()` from the library's own init path. */
#define OPTIGA_PAL_INIT_ENABLED

/** Logging master switch.  The per-layer switches below are left commented out
 *  in production builds: [53] section 6.6 makes the whole point of the Shielded
 *  Connection that APDUs are not readable on the wire, and a UART log that
 *  prints those same APDUs in clear would undo it.  Enable individually, on a
 *  bench part only, never on a provisioned unit. */
#define OPTIGA_LIB_ENABLE_LOGGING
#ifdef OPTIGA_LIB_ENABLE_LOGGING
/* #define OPTIGA_LIB_ENABLE_UTIL_LOGGING */
/* #define OPTIGA_LIB_ENABLE_CRYPT_LOGGING */
/* #define OPTIGA_LIB_ENABLE_CMD_LOGGING */
/* #define OPTIGA_LIB_ENABLE_COMMS_LOGGING */
#endif

#ifdef __cplusplus
}
#endif

#endif /* LS_OPTIGA_LIB_CONFIG_H_ */
