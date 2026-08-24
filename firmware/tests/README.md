# `firmware/tests/` — host-side verification of the LibreServo trust firmware

These tests run on a development host, not on the MSPM0.  They exist to check
the parts of `firmware/` that are *specified constructions* — where a published
document says exactly what bytes must come out — against independent
implementations of those same specifications.

Governed by [`AGENTS.md`](../../AGENTS.md).  Every expected value here comes
from a cited primary source or from an independent implementation of one;
nothing is a golden file captured from this project's own output.

## What is covered

| Test | Covers | Checked against |
| --- | --- | --- |
| `test_pal_crypt_ccm.c` | `pal_crypt_encrypt_aes128_ccm` / `pal_crypt_decrypt_aes128_ccm` in [`../pal/ls_pal_crypt.c`](../pal/ls_pal_crypt.c) | NIST SP 800-38C \[59\] Appendix C example vectors, plus `cryptography.hazmat.primitives.ciphers.aead.AESCCM` |
| `test_pal_crypt_prf.c` | `pal_crypt_tls_prf_sha256` | An independent transcription of IETF RFC 5246 \[58\] §5 `P_hash` |

`ls_crypto_backend_openssl.c` binds [`../pal/ls_crypto_backend.h`](../pal/ls_crypto_backend.h)
to OpenSSL **for these tests only**.  It is not a candidate for the target
build — see "Binding the crypto backend" in [`../README.md`](../README.md).

## Results, 2026-08-23

- **AES-128-CCM: 52/52 encryption outputs byte-identical** to
  `python-cryptography`'s `AESCCM`, spanning every legal nonce length
  (7–13 bytes) and every legal tag length (4, 6, 8, 10, 12, 14, 16 bytes)
  per \[59\] Appendix A.1, with empty and non-empty associated data and
  payloads of 0–100 bytes.  The three \[59\] Appendix C vectors reproduce
  exactly.  Round-trip and single-bit-tamper rejection pass on every
  non-degenerate case.
- **TLS 1.2 PRF (SHA-256): 24/24 outputs identical** to the independent
  RFC 5246 §5 implementation, across secret lengths 16–64 bytes, label
  lengths 4–20 bytes, seeds of 0–48 bytes and outputs of 16–128 bytes
  (i.e. across the `P_hash` iteration boundary).

### One deliberate, spec-mandated difference

`pal_crypt_decrypt_aes128_ccm` **rejects a ciphertext with an empty payload**
(`Clen == Tlen`).  That is \[59\] §6.2 step 1 verbatim: "If Clen ≤ Tlen, then
return INVALID."  Some CCM implementations accept the degenerate MAC-only case;
this one follows the cited specification.  The Shielded Connection always
carries an APDU payload, so the case does not arise in use.  The five
"ROUNDTRIP FAIL" lines the harness prints for zero-length-payload vectors are
this behaviour, not a defect — the encrypt side of those same vectors still
matches the reference exactly.

## Running them

```sh
LIB=firmware/external/optiga-trust-m     # see ../tools/fetch_optiga_host_library.sh

gcc -std=c99 -O1 -o /tmp/test_ccm \
    -DOPTIGA_LIB_EXTERNAL='"ls_optiga_lib_config.h"' \
    -Ifirmware/pal -Ifirmware/config \
    -I$LIB/include -I$LIB/include/pal -I$LIB/include/common \
    firmware/tests/test_pal_crypt_ccm.c \
    firmware/tests/ls_crypto_backend_openssl.c \
    firmware/pal/ls_pal_crypt.c -lcrypto

/tmp/test_ccm < vectors.txt
```

`vectors.txt` is whitespace-separated `key nonce aad plaintext maclen`, hex,
one case per line, with `-` for an empty field.  The generator used for the
2026-08-23 run is in `generate_vectors.py`.
