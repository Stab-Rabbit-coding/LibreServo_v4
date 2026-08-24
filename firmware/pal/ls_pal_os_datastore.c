/**
 * @file    ls_pal_os_datastore.c
 * @brief   OPTIGA(TM) Trust M PAL datastore implementation for LibreServo v4.
 *
 * Implements [57] `include/pal/pal_os_datastore.h`.  This is the most
 * security-critical file in the PAL: datastore ID
 * #OPTIGA_PLATFORM_BINDING_SHARED_SECRET_ID holds the **platform binding
 * secret**, the symmetric secret that keys the entire I2C Shielded Connection
 * to U7 (TODO.md 4.4, PCB/servo-bus-security-protocol.md section 4.4).
 *
 * ------------------------------------------------------------------------
 * WHERE THE SECRET LIVES — a corrected design decision
 * ------------------------------------------------------------------------
 * `PCB/servo-bus-security-protocol.md` section 4.4 (2026-08-22) stated the secret
 * would be "deposited into both U7's platform-binding-secret slot and the MCU's
 * KEYSTORE at manufacture."  **The KEYSTORE cannot serve this purpose**, for two
 * independent reasons, both read from primary sources 2026-08-23:
 *
 *   1. [46] section 8.21, p. 89 describes the Keystore controller's use model as
 *      depositing keys "and have the AES engine access them subsequently in a
 *      secure manner without leaking any key data to observers", holding
 *      "128 and 256-bit keys".  It is a write-then-use-by-AESADV store; the CPU
 *      does not read key material back out of it.
 *   2. The host library requires plaintext readback.  [53] section 6.6.1 states
 *      that during Shielded Connection establishment "the optiga_comms_ifx_i2c
 *      module invokes pal_os_datastore_read", and
 *      #OPTIGA_SHARED_SECRET_MAX_LENGTH is 64 bytes — not an AES key width.
 *
 * The platform binding secret must therefore live in MCU non-volatile memory
 * that firmware can read, protected by the MSPM0's own flash and debug
 * protections rather than by a key store.  Selecting and configuring that
 * protection (static write protection, flash read-out protection, and the
 * one-way NONMAIN debug lockdown of [49] sections 2.6/3.2) is a real,
 * consequential decision that this file deliberately does NOT make on its own:
 * it is tracked as TODO.md 4.13 and must be settled before any unit is
 * provisioned, because NONMAIN lockdown is a one-way, per-unit door.
 *
 * The KEYSTORE remains the right home for the *derived per-session bus CMAC
 * key* of section 4.7 — which is an AES key, is used only by AESADV, and is never
 * read back.  The two are different secrets with different lifetimes; conflating
 * them was the original error.
 *
 * ------------------------------------------------------------------------
 * The secure-store contract
 * ------------------------------------------------------------------------
 * Until TODO.md 4.13 is settled, this file talks to the four functions declared
 * in `ls_secure_store.h`.  That keeps the storage-medium decision in one place
 * and out of the OPTIGA porting layer.
 *
 * References (see REFERENCES.md):
 *   [46] TI, MSPM0G3519-Q1/MSPM0G3518-Q1 Datasheet, SLASFA6B.
 *   [49] TI, MSPM0 application note SLAAE29A.
 *   [53] Infineon, OPTIGA(TM) Trust M Solution Reference Manual, Rev 3.70.
 *   [56] Infineon, OPTIGA(TM) Trust M configurations, Configuration Guide, Rev 2.2.
 *   [57] Infineon, OPTIGA(TM) Trust M Host Library for C, Ver 5.8.1.
 *
 * Written by Claude Opus 5 (claude-opus-5) under human direction, 2026-08-23.
 */

#include "pal_os_datastore.h"

#include <string.h>

#include "ls_secure_store.h"

/**
 * Shielded-connection manage context.
 *
 * [57] `pal_os_datastore.h` allows this to live in volatile memory only (by
 * setting #OPTIGA_COMMS_MANAGE_CONTEXT_ID to
 * #OPTIGA_LIB_PAL_DATA_STORE_NOT_CONFIGURED).  This port keeps it in RAM
 * deliberately: it is per-session state, and persisting it would mean an NVM
 * write on every session — against the endurance budget of [53] section 5.1
 * (2 million tearing-safe programming cycles across all objects) for no gain,
 * since a reset ends the session anyway.
 */
static uint8_t ls_manage_context[LS_MANAGE_CONTEXT_SIZE];
static uint16_t ls_manage_context_length;

/**
 * Hibernate/application context handle.  Also RAM-resident, and for the same
 * reason: hibernate state does not survive the reset that would need it.
 * [53] section 5.1 additionally notes "One hibernate cycle causes five
 * tearing-safe-programming-cycles" on the OPTIGA side, which is its own reason
 * not to build a design around frequent hibernation.
 */
static uint8_t ls_hibernate_context[APP_CONTEXT_SIZE];
static uint16_t ls_hibernate_context_length;

pal_status_t
pal_os_datastore_read(uint16_t datastore_id, uint8_t *p_buffer, uint16_t *p_buffer_length) {
    pal_status_t status = PAL_STATUS_FAILURE;

    if ((p_buffer == NULL) || (p_buffer_length == NULL)) {
        return PAL_STATUS_INVALID_INPUT;
    }

    switch (datastore_id) {
        case OPTIGA_PLATFORM_BINDING_SHARED_SECRET_ID:
            /* Delegated to the secure store.  A failure here is NOT recoverable
             * by falling back to a default or a zero buffer: [56] section 2 Table 1
             * records that an unpaired OPTIGA Trust M V3 ships 0xE140 with a
             * published Default value and read access ALW, so a "sensible
             * fallback" would silently key the Shielded Connection with a value
             * an attacker already knows.  Fail closed. */
            status = ls_secure_store_read_platform_binding_secret(p_buffer, p_buffer_length);
            break;

        case OPTIGA_COMMS_MANAGE_CONTEXT_ID:
            if (*p_buffer_length < ls_manage_context_length) {
                status = PAL_STATUS_FAILURE;
                break;
            }
            (void)memcpy(p_buffer, ls_manage_context, ls_manage_context_length);
            *p_buffer_length = ls_manage_context_length;
            status = PAL_STATUS_SUCCESS;
            break;

        case OPTIGA_HIBERNATE_CONTEXT_ID:
            if (*p_buffer_length < ls_hibernate_context_length) {
                status = PAL_STATUS_FAILURE;
                break;
            }
            (void)memcpy(p_buffer, ls_hibernate_context, ls_hibernate_context_length);
            *p_buffer_length = ls_hibernate_context_length;
            status = PAL_STATUS_SUCCESS;
            break;

        default:
            status = PAL_STATUS_INVALID_INPUT;
            break;
    }

    return status;
}

pal_status_t
pal_os_datastore_write(uint16_t datastore_id, const uint8_t *p_buffer, uint16_t length) {
    pal_status_t status = PAL_STATUS_FAILURE;

    if ((p_buffer == NULL) || (length == 0U)) {
        return PAL_STATUS_INVALID_INPUT;
    }

    switch (datastore_id) {
        case OPTIGA_PLATFORM_BINDING_SHARED_SECRET_ID:
            /* [53] section 6.5.8, p. 107: "The recommended length of platform
             * binding shared secret is 32 bytes or more."  Enforced here rather
             * than trusted, because this write happens once per unit at
             * manufacture and a short secret would not be noticed afterwards. */
            if ((length < LS_PLATFORM_BINDING_SECRET_MIN_LENGTH)
                || (length > OPTIGA_SHARED_SECRET_MAX_LENGTH)) {
                status = PAL_STATUS_INVALID_INPUT;
                break;
            }
            status = ls_secure_store_write_platform_binding_secret(p_buffer, length);
            break;

        case OPTIGA_COMMS_MANAGE_CONTEXT_ID:
            if (length > (uint16_t)sizeof(ls_manage_context)) {
                status = PAL_STATUS_FAILURE;
                break;
            }
            (void)memcpy(ls_manage_context, p_buffer, length);
            ls_manage_context_length = length;
            status = PAL_STATUS_SUCCESS;
            break;

        case OPTIGA_HIBERNATE_CONTEXT_ID:
            if (length > (uint16_t)sizeof(ls_hibernate_context)) {
                status = PAL_STATUS_FAILURE;
                break;
            }
            (void)memcpy(ls_hibernate_context, p_buffer, length);
            ls_hibernate_context_length = length;
            status = PAL_STATUS_SUCCESS;
            break;

        default:
            status = PAL_STATUS_INVALID_INPUT;
            break;
    }

    return status;
}
