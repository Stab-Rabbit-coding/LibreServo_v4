# `firmware/` — OPTIGA™ Trust M integration for LibreServo v4.0.0

This directory is the deliverable of [`TODO.md`](../TODO.md) §7.2: the Trust M
driver layer — I²C, Shielded Connection, ECDSA device authentication and ECDHE
session-key agreement — for `U7` on `LibreServo-v4.0.0`.

Governed by [`AGENTS.md`](../AGENTS.md).  Every register offset, magic key, bit
field, object identifier and timing figure in this tree was read from a primary
source and is cited at the point of use against [`REFERENCES.md`](../REFERENCES.md).
Nothing is carried over from a similar part or reproduced from memory.

> **This is not the servo firmware.**  [`TODO.md`](../TODO.md) §7.1 records that
> the firmware is a full rewrite and that `Src/`, `Inc/` and
> `Test_LibreServo_v2.ioc` still target ST's HAL and an STM32 register set that
> this fork no longer uses.  Nothing here touches those; this tree is the
> security subsystem only, and it is designed to plug into whatever control
> firmware §7.1 eventually produces.

---

## Layout

| Path | What it is |
| --- | --- |
| `config/ls_optiga_lib_config.h` | Feature selection for the host library — a deliberate narrowing of the upstream V3 profile |
| `pal/` | The platform abstraction layer the host library requires, ported to the MSPM0G3518-Q1 |
| `pal/mspm0/ls_mspm0_i2c_regs.h` | I²C register map, from SLAU846E §25 |
| `trust/` | The LibreServo-facing API: init, device authentication, session-key agreement, pairing |
| `tests/` | Host-side verification of the specified constructions — see [`tests/README.md`](tests/README.md) |
| `tools/fetch_optiga_host_library.sh` | Fetches the pinned upstream host library |

---

## Upstream dependency and attribution

The **OPTIGA™ Trust M Host Library for C** is an upstream third-party
dependency, cataloged as **[57]**.  It is **not vendored** into this
repository — `tools/fetch_optiga_host_library.sh` fetches the pinned commit
into `firmware/external/optiga-trust-m/`:

```text
repository  https://github.com/Infineon/optiga-trust-m
commit      67cfd0e589fc09f936d4e4ce4fa35eacc43af72f   (2026-08-13)
version     "Ver 5.8.1"  (include/optiga_lib_version.h)
licence     MIT — Copyright (c) 2018-2024 Infineon Technologies AG
```

Pinning by commit gives the same reproducibility as vendoring while keeping the
boundary between upstream code and LibreServo code legible, as `AGENTS.md` §1
requires.  **Do not bump the pin casually**: a host-library bump changes the
security-relevant `COMMS` layer and must be re-reviewed against [53] §6.6 first.

Files in this tree that derive from upstream carry the attribution chain in
their own header comments.  The two that derive most directly are:

- `config/ls_optiga_lib_config.h` — narrowed from
  `include/optiga_lib_config_m_v3.h`.
- `trust/ls_trust_pairing.c` — the sequence of [53] §2.3.4, structured after
  `examples/optiga/usecases/example_pair_host_and_optiga_using_pre_shared_secret.c`,
  with this design's own access-condition choice and error ordering.

---

## Building

Select this project's configuration through the upstream hook — not by
shadowing a header on the include path, so an upstream bump cannot silently
reinstate the default profile:

```sh
-DOPTIGA_LIB_EXTERNAL='"ls_optiga_lib_config.h"'
```

Include paths: `firmware/config`, `firmware/pal`, `firmware/trust`, and the
upstream `include/`, `include/pal`, `include/common`, `include/comms`,
`include/ifx_i2c`, `include/cmd`.

---

## What the application must supply

The port stops at three explicitly declared boundaries.  They are not
oversights; each is a system-level decision that belongs to the servo firmware
rather than to a secure-element driver, and each is documented where it is
declared.  As of 2026-08-23 a link of this tree against the full host library
resolves **every** symbol except these eight:

| Contract | Declared in | Why it is not decided here |
| --- | --- | --- |
| `ls_board_time_us`, `ls_board_schedule_oneshot_us`, `ls_board_cancel_oneshot` | `pal/ls_board.h` | The control loop owns timer allocation.  A PAL that seized a `TIMG` instance now would be deciding a system resource on behalf of firmware that §7.1 has not written yet. |
| `ls_secure_store_read/write/has_platform_binding_secret` | `pal/ls_secure_store.h` | *Where* the platform binding secret lives is a one-way, per-unit manufacturing decision — see below.  Tracked as `TODO.md` 4.13. |
| `ls_crypto_aes128_encrypt_block`, `ls_crypto_hmac_sha256` | `pal/ls_crypto_backend.h` | AES and SHA-256 must come from a vetted implementation, not from this project.  Tracked as `TODO.md` 7.6. |

---

## Findings from this integration

Four things turned up while building this that were not visible from the
datasheets alone.  All four are recorded in `TODO.md`; they are listed together
here because each changes something a reader might otherwise assume.

### 1. `PA8`'s I²C function is `PF4`, not `PF3`

`TODO.md` §3.1 and `PCB/OPTIGA-Trust-M-secure-element.md` §3 both recorded
`SE_I2C_SDA` on `PA8` as "IOMUX `PF3` = `I2C0_SDA`".  Read with a
column-preserving extraction, [46] Table 6-2 (p. 18) gives `PA8` as
`PF1` = `PA8`, `PF2` = `UART1_TX`, `PF3` = **`SPI0_CS0`**, `PF4` = **`I2C0_SDA`**.
(`PA1`'s `PF3` = `I2C0_SCL`, p. 15, *is* correct; the likely origin of the error
is `PA0`, whose `PF3` genuinely is `I2C0_SDA`.)

The pin selection and the schematic are unaffected — `PA8` is still the right
pin — but firmware written from "PF3" would mux an SPI chip-select onto the
secure element's data line and the bus would never come up.  Corrected in
`pal/ls_board.h`; tracked as `TODO.md` 3.4.

### 2. The MSPM0 KEYSTORE cannot hold the platform binding secret

`PCB/servo-bus-security-protocol.md` §4.4 originally proposed depositing the
secret "into both `U7`'s platform-binding-secret slot and the MCU's KEYSTORE at
manufacture."  It cannot go in the KEYSTORE, for two independent reasons:

- [46] §8.21 (p. 89) describes the Keystore controller as a place to "securely
  deposit keys ... and have the AES engine access them subsequently in a secure
  manner without leaking any key data to observers", holding "128 and 256-bit
  keys".  Software does not read key material back out of it.
- The host library needs the plaintext.  [53] §6.6.1 states that during
  Shielded Connection establishment "the `optiga_comms_ifx_i2c` module invokes
  `pal_os_datastore_read`", and the secret is up to 64 bytes — not an AES key
  width.

The secret must live in MCU non-volatile memory that firmware can read,
protected by flash and debug protections.  The KEYSTORE remains exactly right
for the *derived per-session bus CMAC key* of §4.7 — an AES key, used only by
AESADV, never read back.  Two different secrets with two different lifetimes;
conflating them was the original error.  Tracked as `TODO.md` 4.13.

### 3. Enabling the Shielded Connection obliges the host to supply crypto

Defining `OPTIGA_COMMS_SHIELDED_CONNECTION` makes three host-side primitives a
link-time requirement — `pal_crypt_tls_prf_sha256`,
`pal_crypt_encrypt_aes128_ccm` and `pal_crypt_decrypt_aes128_ccm` — because the
presentation layer of the IFX I²C protocol [54] protects each APDU with
AES-128-CCM under a key derived from the platform binding secret.  This is a
real cost of the §4.4 decision and was not visible in [45].

`pal/ls_pal_crypt.c` implements both constructions from their specifications
([58] RFC 5246 §5 and [59] NIST SP 800-38C §§6.1–6.2 and Appendix A) and is
verified against independent implementations — see [`tests/README.md`](tests/README.md).
The AES block cipher and HMAC-SHA256 underneath are deliberately *not*
implemented here.

### 4. Two upstream header-guard bugs in the host library

Both found at commit `67cfd0e58` while narrowing the feature set, both
reported in `TODO.md` 7.5:

- `include/cmd/optiga_cmd.h:596` guards `optiga_cmd_decrypt_sym` on
  `SYM_DECRYPT || HMAC_VERIFY || CLEAR_AUTO_STATE`, but its parameter type
  `optiga_decrypt_sym_params_t` (`include/common/optiga_lib_common.h:524`) is
  guarded on `SYM_ENCRYPT || SYM_DECRYPT` alone.  Enabling
  `CLEAR_AUTO_STATE` without a symmetric feature fails to compile.
- `src/crypt/optiga_crypt.c` wraps all three `optiga_crypt_tls_prf_shaXXX`
  bodies (lines 506–642) in one combined guard, while the enum values they
  reference (`OPTIGA_TLS12_PRF_SHA_384`, `OPTIGA_TLS12_PRF_SHA_512`) are each
  guarded individually.  Any subset of the three fails to compile.

Neither is visible with the upstream default V3 profile, which enables
everything.  The workarounds and their reasoning are in
`config/ls_optiga_lib_config.h`.

---

## Design decisions this tree encodes

Recorded in full in
[`../PCB/servo-bus-security-protocol.md`](../PCB/servo-bus-security-protocol.md);
summarised here with where each one lands in code.

| Decision | Where |
| --- | --- |
| §4.4 — Shielded Connection on all `U7` traffic, full command+response protection by default | `config/ls_optiga_lib_config.h` |
| §4.4 — pairing at manufacture is mandatory, because a V3 part ships 0xE140 with a published default value and read AC `ALW` ([56] Table 1) | `trust/ls_trust_pairing.c` |
| §4.7 — per-frame authentication runs on the MCU's AESADV, never on `U7` | `trust/ls_trust.h`, and the omission of every symmetric command from the config |
| §4.9 — NIST P-256 floor, P-384 permitted, RSA and Brainpool excluded entirely | `config/ls_optiga_lib_config.h` |
| Warm reset, never cold — the board has `SE_RST` but no VDD switch, and [53] §4.6.4 caps VCC cycling at 200 000 over the part's life | `config/ls_optiga_lib_config.h`, `pal/ls_pal_ifx_i2c_config.c` |
| No heap — a servo control loop cannot tolerate unbounded allocation latency | `pal/ls_pal_os_memory.c` |
| Logging compiled to a no-op sink — a UART echoing APDUs would undo the Shielded Connection | `pal/ls_pal_logger.c` |

### On the security-monitor budget

This tree corrects a characterisation that had propagated through the repo's
documentation.  The Trust M's limit is **not** a flat "one protected operation
per 5 seconds, always."  Per [53] §4.6.2 that is the *permitted sustained usage
profile*, and per §4.6.4 the hardware throttle "starts as soon as SEC reaches
the value of 128" and reaches `t_max` only at SEC = 255.  A short burst of
protected operations at boot incurs no delay at all.

What the design still holds is the conclusion, and for a better reason:
`U7` must stay out of the servo's per-frame path.  A control loop authenticating
at tens of Hz to kHz would drive SEC to its ceiling and be throttled into
failure.  The boot-time sequence this tree performs — one identity signature,
one ephemeral key generation — spends two security events, and every subsequent
step operates on a *session context*, which [53] §4.6.1 Table 65 explicitly
carves out of all four key-use events.

`trust/ls_trust_oid.h` carries the constants; `trust/ls_trust.c` reads
OID 0xE0C5 (SEC) before spending budget and reports `LS_TRUST_ERR_THROTTLED`
rather than issuing an operation into a throttle.

---

## Binding the crypto backend

`pal/ls_crypto_backend.h` needs an AES-128 block encrypt and an HMAC-SHA256.
Two candidates, with the choice tracked as `TODO.md` 7.6:

1. **MSPM0 AESADV for AES, software SHA-256.**  [46] §8.20 gives the part an
   on-die AES engine and [49] Table 5-3 measures it at 76 cycles / 0.95 µs per
   128-bit block at 80 MHz.  The part has no SHA accelerator.
2. **Mbed TLS**, which is what the upstream reference PALs use
   (`examples/utilities/authenticate_chip/pal_crypt_mbedtls.c`).

Whichever is chosen must be constant-time with respect to key material.
`pal/ls_pal_crypt.c`'s own MAC comparison already is.

`tests/ls_crypto_backend_openssl.c` binds the same interface to OpenSSL **for
host tests only** and is not a candidate for the target build.

---

## Checking the port without hardware

Every file in `pal/` and `trust/` compiles clean on a development host under
`-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wcast-qual`, against the real
upstream headers.  The one architecture-specific construct outside
`pal/mspm0/` — the `PRIMASK` critical section in `pal/ls_pal_os_lock.c` — is
guarded on `__ARM_ARCH` with a host no-op fallback so the same sources stay
lintable off-target.  The host fallback is never linked into a target image.

---

*Written by Claude Opus 5 (`claude-opus-5`) under human direction, 2026-08-23,
from the Infineon `optiga-trust-m-overview` and `optiga-trust-m` repositories
[53]–[57] and the local TI datasheets and technical reference manual [46], [49],
[52].  Every citation was read from the source in that session; nothing is
reproduced from model memory.*
