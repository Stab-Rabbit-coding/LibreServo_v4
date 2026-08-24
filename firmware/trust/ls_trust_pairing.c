/**
 * @file    ls_trust_pairing.c
 * @brief   Platform binding secret provisioning — the manufacturing pairing
 *          step that TODO.md 4.4 was blocked on.
 *
 * Implements [53] section 2.3.4, p. 20, Figure 12, "Pair OPTIGA(TM) Trust M with
 * host (pre-shared secret based)".  Structure follows the upstream reference
 * implementation [57]
 * `examples/optiga/usecases/example_pair_host_and_optiga_using_pre_shared_secret.c`
 * (Copyright (c) 2018-2024 Infineon Technologies AG, SPDX-License-Identifier:
 * MIT), with the LibreServo-specific access-condition choice and error handling
 * documented below.
 *
 * ------------------------------------------------------------------------
 * WHY THIS RUNS AT ALL
 * ------------------------------------------------------------------------
 * [56] section 2, pp. 5-7, Table 1, row "0xE140 - Platform binding secret":
 * on an **OPTIGA(TM) Trust M V3** part — the variant on this BOM, sales code
 * SLS 32AIA010ML ([45] p. 8, Table 2) — the object ships with
 *
 *     Life cycle state (LcsO) : Creation
 *     Value                   : Default
 *     Read AC                 : ALW
 *     Change AC               : LcsO < operational || Conf(0xE140)
 *
 * A **Default** value with **ALW** read access is a published constant.  Until
 * this function has run, the Shielded Connection of [53] section 6.6 is keyed by
 * something anyone can look up, and it protects nothing.  Pairing is not a
 * hardening option on this design; it is the step that makes U7 worth fitting.
 *
 * (Express and MTR parts ship 0xE140 Operational with a chip-unique value and
 * read AC NEV — already paired at Infineon, secret retrieved from CIRRENT(TM)
 * Cloud ID.  This design does not use those variants.)
 *
 * ------------------------------------------------------------------------
 * IT IS A ONE-WAY DOOR
 * ------------------------------------------------------------------------
 * The final metadata write raises 0xE140's life-cycle state and closes its read
 * access.  After it, the secret cannot be read back out of the part.  If the
 * host side of the write did not land durably, the two are unpaired and there
 * is no recovery — hence the ordering below: **the host stores the secret and
 * confirms the store BEFORE the OPTIGA-side metadata is locked.**
 *
 * References (see REFERENCES.md):
 *   [45] Infineon, OPTIGA(TM) Trust M Datasheet, Rev 3.70.
 *   [49] TI, MSPM0 application note SLAAE29A.
 *   [53] Infineon, OPTIGA(TM) Trust M Solution Reference Manual, Rev 3.70.
 *   [56] Infineon, OPTIGA(TM) Trust M configurations, Configuration Guide, Rev 2.2.
 *   [57] Infineon, OPTIGA(TM) Trust M Host Library for C, Ver 5.8.1.
 *
 * Written by Claude Opus 5 (claude-opus-5) under human direction, 2026-08-23.
 */

#include "ls_secure_store.h"
#include "ls_trust.h"
#include "ls_trust_internal.h"
#include "ls_trust_oid.h"
#include "optiga_lib_common.h"
#include "pal_os_datastore.h"

/* Life-cycle status coding — [53] section 5.3, p. 96, Table 74.  The four primary
 * states progress in ONE direction only: "Initialization (in) => operational
 * (op) state, but not vice versa." */
#define LS_LCSO_CREATION       (0x01U)  /**< creation (cr) */
#define LS_LCSO_INITIALIZATION (0x03U)  /**< initialization (in) */
#define LS_LCSO_OPERATIONAL    (0x07U)  /**< operational (op) */
#define LS_LCSO_TERMINATION    (0x0FU)  /**< termination (te) */

/**
 * Life-cycle state this design leaves 0xE140 in after pairing: **operational**.
 *
 * This is a deliberate divergence from the upstream reference example [57],
 * which sets CREATION and carries the comment "At the real time/customer side
 * this needs to be LCSO_STATE_OPERATIONAL".  Shipping a unit at CREATION would
 * be unacceptable: the change access condition written below permits
 * `LcsO < operational`, so any party that can reach the I2C bus could rewrite
 * the platform binding secret and re-pair U7 to itself.
 *
 * Raising LcsO to operational is irreversible ([53] section 5.3).  That is the
 * intent — a unit that leaves the manufacturing line must not be re-pairable.
 */
#define LS_PAIRING_FINAL_LCSO (LS_LCSO_OPERATIONAL)

/**
 * Final metadata for 0xE140, in the TLV form [53] section 5.5 specifies.
 *
 * Byte-for-byte rationale (tags from [53] section 5.5 "Metadata expression"):
 *
 *   20 17        Metadata tag, 0x17 = 23 bytes of metadata follow.
 *   C0 01 07     LcsO := operational.  Irreversible ([53] section 5.3).
 *   D0 07        Change access condition, 7 bytes:
 *      E1 FC 07     LcsO < operational  — i.e. never again, now that C0 sets it
 *                   to operational in this same write.
 *      FE           Logical OR.
 *      20 E1 40     Conf(0xE140) — the object may still be changed, but ONLY
 *                   through a confidentiality-protected write under an
 *                   established Shielded Connection keyed by the very secret
 *                   being replaced.  [56] section 2, p. 7 defines Conf(X): "the
 *                   action is only possible in case the data involved ... are
 *                   confidentiality protected with key given by X.  This
 *                   enforces the shielded connection".
 *   D1 03 E1 FC 07  Read access condition := LcsO < operational.  Since LcsO is
 *                   operational after this write, the secret becomes
 *                   permanently unreadable over the interface.
 *   D3 01 00     Execute access condition := ALW.
 *   E8 01 22     Data object type := 0x22, "platform binding secret".
 *
 * Keeping the `Conf(0xE140)` branch is what makes the runtime secret rotation
 * of [53] section 2.3.6, p. 21, Figure 14 possible later.  [53] section 6.5.8
 * recommends that rotation; [53] section 5.1's NVM endurance budget (2 million
 * tearing-safe programming cycles across all objects) is the reason it must be
 * scheduled rather than done casually.  Rotation itself is not implemented yet
 * — TODO.md 4.14.
 */
static const uint8_t ls_pairing_final_metadata[] = {
    0x20, 0x17,
    /* LcsO */
    0xC0, 0x01, LS_PAIRING_FINAL_LCSO,
    /* Change AC: (LcsO < operational) OR Conf(0xE140) */
    0xD0, 0x07,
    0xE1, 0xFC, LS_LCSO_OPERATIONAL,
    0xFE,
    0x20, 0xE1, 0x40,
    /* Read AC: LcsO < operational */
    0xD1, 0x03,
    0xE1, 0xFC, LS_LCSO_OPERATIONAL,
    /* Execute AC: always */
    0xD3, 0x01, 0x00,
    /* Data object type: platform binding secret */
    0xE8, 0x01, 0x22,
};

ls_trust_status_t ls_trust_pair_with_host(void) {
    /* The full 64 bytes the library permits.  [53] section 6.5.8, p. 107
     * recommends "32 bytes or more"; there is no reason to take the floor when
     * #OPTIGA_SHARED_SECRET_MAX_LENGTH allows twice it. */
    uint8_t secret[OPTIGA_SHARED_SECRET_MAX_LENGTH];
    uint8_t metadata[64];
    uint16_t metadata_length = (uint16_t)sizeof(metadata);
    optiga_lib_status_t submitted;
    ls_trust_status_t status;
    pal_status_t pal_status;

    /* 1. Instances.  Both are created with NO protection and the pre-shared
     *    secret protocol version, per the upstream reference sequence: there is
     *    no secret yet, so there is nothing to protect the channel with. */
    if (ls_trust_util == NULL) {
        ls_trust_util = optiga_util_create(0, ls_trust_callback, NULL);
    }
    if (ls_trust_crypt == NULL) {
        ls_trust_crypt = optiga_crypt_create(0, ls_trust_callback, NULL);
    }
    if ((ls_trust_util == NULL) || (ls_trust_crypt == NULL)) {
        return LS_TRUST_ERR_OPTIGA;
    }

    OPTIGA_UTIL_SET_COMMS_PROTECTION_LEVEL(ls_trust_util, OPTIGA_COMMS_NO_PROTECTION);
    OPTIGA_UTIL_SET_COMMS_PROTOCOL_VERSION(ls_trust_util,
                                           OPTIGA_COMMS_PROTOCOL_VERSION_PRE_SHARED_SECRET);
    OPTIGA_CRYPT_SET_COMMS_PROTECTION_LEVEL(ls_trust_crypt, OPTIGA_COMMS_NO_PROTECTION);
    OPTIGA_CRYPT_SET_COMMS_PROTOCOL_VERSION(ls_trust_crypt,
                                            OPTIGA_COMMS_PROTOCOL_VERSION_PRE_SHARED_SECRET);

    /* 2. Open the application. */
    ls_trust_lib_status = OPTIGA_LIB_BUSY;
    submitted = optiga_util_open_application(ls_trust_util, 0);
    status = ls_trust_await(submitted);
    if (status != LS_TRUST_OK) {
        return status;
    }

    /* 3. Read 0xE140's metadata and check its life-cycle state.
     *
     *    [53] section 2.3.4 pre-condition: "The Platform Binding Secret data
     *    object is not locked.  The LcsO ... must be less than operational."
     *    An already-operational object means this unit was paired before; that
     *    is not an error to power through, it is a reason to stop — the host
     *    secret store is the only place the matching secret could be, and if it
     *    is empty the part is already unusable by this host. */
    ls_trust_lib_status = OPTIGA_LIB_BUSY;
    OPTIGA_UTIL_SET_COMMS_PROTECTION_LEVEL(ls_trust_util, OPTIGA_COMMS_NO_PROTECTION);
    submitted = optiga_util_read_metadata(ls_trust_util,
                                          LS_OID_PLATFORM_BINDING_SECRET,
                                          metadata,
                                          &metadata_length);
    status = ls_trust_await(submitted);
    if (status != LS_TRUST_OK) {
        return status;
    }

    /* Metadata layout: 0x20, length, then TLVs.  The upstream reference reads
     * LcsO at offset 4, which corresponds to `20 LL C0 01 <lcso>`.  Verify the
     * two tag bytes before trusting that offset rather than indexing blind — a
     * misread here would decide whether an irreversible write goes ahead. */
    if ((metadata_length < 5U) || (metadata[0] != 0x20U) || (metadata[2] != 0xC0U)
        || (metadata[3] != 0x01U)) {
        return LS_TRUST_ERR_OPTIGA;
    }
    if (metadata[4] >= LS_LCSO_OPERATIONAL) {
        /* Already paired (or locked by someone else).  If the host has the
         * matching secret this is simply a re-run and benign; if not, the unit
         * needs escalation, not another pairing attempt. */
        return (ls_secure_store_has_platform_binding_secret() != 0U) ? LS_TRUST_OK
                                                                    : LS_TRUST_ERR_NOT_PAIRED;
    }

    /* 4. Generate the secret on the part's certified TRNG.
     *
     *    Taken from U7 rather than from the MCU deliberately: [45] p. 1 makes
     *    the OPTIGA a Common Criteria EAL6+ (high) certified device, and its
     *    RNG carries that evaluation.  The MSPM0's entropy source carries no
     *    equivalent certification in [46].  For the one secret that keys every
     *    subsequent protected exchange, the certified source is the right one. */
    ls_trust_lib_status = OPTIGA_LIB_BUSY;
    OPTIGA_CRYPT_SET_COMMS_PROTECTION_LEVEL(ls_trust_crypt, OPTIGA_COMMS_NO_PROTECTION);
    submitted = optiga_crypt_random(ls_trust_crypt,
                                    OPTIGA_RNG_TYPE_TRNG,
                                    secret,
                                    (uint16_t)sizeof(secret));
    status = ls_trust_await(submitted);
    if (status != LS_TRUST_OK) {
        ls_trust_secure_wipe(secret, (uint16_t)sizeof(secret));
        return status;
    }

    /* 5. Write it into 0xE140. */
    ls_trust_lib_status = OPTIGA_LIB_BUSY;
    OPTIGA_UTIL_SET_COMMS_PROTECTION_LEVEL(ls_trust_util, OPTIGA_COMMS_NO_PROTECTION);
    submitted = optiga_util_write_data(ls_trust_util,
                                       LS_OID_PLATFORM_BINDING_SECRET,
                                       OPTIGA_UTIL_ERASE_AND_WRITE,
                                       0U,
                                       secret,
                                       (uint16_t)sizeof(secret));
    status = ls_trust_await(submitted);
    if (status != LS_TRUST_OK) {
        ls_trust_secure_wipe(secret, (uint16_t)sizeof(secret));
        return status;
    }

    /* 6. Store it on the host — BEFORE locking the metadata.
     *
     *    Ordering matters and is the whole reason this is not a straight
     *    transcription of the upstream example.  Step 7 makes the secret
     *    permanently unreadable from the part.  If the host store failed and we
     *    had already locked, the unit would be bricked for this purpose with no
     *    diagnostic.  Locking last means a host-store failure leaves 0xE140
     *    still writable, so the line can retry or scrap cleanly. */
    pal_status = pal_os_datastore_write(OPTIGA_PLATFORM_BINDING_SHARED_SECRET_ID,
                                        secret,
                                        (uint16_t)sizeof(secret));
    ls_trust_secure_wipe(secret, (uint16_t)sizeof(secret));
    if (pal_status != PAL_STATUS_SUCCESS) {
        return LS_TRUST_ERR_OPTIGA;
    }

    /* Read-back check.  A write that reported success but did not persist is
     * exactly the failure this ordering exists to catch, and it costs one flash
     * read to rule out. */
    if (ls_secure_store_has_platform_binding_secret() == 0U) {
        return LS_TRUST_ERR_OPTIGA;
    }

    /* 7. Lock 0xE140: raise LcsO to operational and close read access.
     *    Irreversible from here. */
    ls_trust_lib_status = OPTIGA_LIB_BUSY;
    OPTIGA_UTIL_SET_COMMS_PROTECTION_LEVEL(ls_trust_util, OPTIGA_COMMS_NO_PROTECTION);
    submitted = optiga_util_write_metadata(ls_trust_util,
                                           LS_OID_PLATFORM_BINDING_SECRET,
                                           ls_pairing_final_metadata,
                                           (uint8_t)sizeof(ls_pairing_final_metadata));
    status = ls_trust_await(submitted);
    if (status != LS_TRUST_OK) {
        /* The secret matches on both sides but the object is not locked.  The
         * unit is functional and MUST NOT ship: 0xE140's read AC is still ALW.
         * Reported as a failure so the line stops. */
        return status;
    }

    /* 8. Close the application.  The caller re-opens through ls_trust_init(),
     *    which will now find a paired host and run at full protection. */
    ls_trust_lib_status = OPTIGA_LIB_BUSY;
    submitted = optiga_util_close_application(ls_trust_util, 0);
    (void)ls_trust_await(submitted);

    return LS_TRUST_OK;
}
