#!/usr/bin/env python3
"""Generate AES-128-CCM test vectors for ``test_pal_crypt_ccm``.

Emits ``key nonce aad plaintext maclen`` per line, hex, ``-`` for empty.
The first three cases are the worked examples of NIST SP 800-38C [59]
Appendix C; the rest sweep every legal nonce length (7-13) and tag length
(4, 6, 8, 10, 12, 14, 16) from [59] Appendix A.1 against assorted associated
data and payload lengths, including the multi-block and empty cases.

The seed is fixed so a re-run reproduces the 2026-08-23 result recorded in
``README.md``.

Written by Claude Opus 5 (``claude-opus-5``) under human direction, 2026-08-23.
"""

import os
import random

# NIST SP 800-38C [59] Appendix C, examples C.1 to C.3.
SP800_38C_APPENDIX_C = [
    (
        "404142434445464748494a4b4c4d4e4f",
        "10111213141516",
        "0001020304050607",
        "20212223",
        4,
    ),
    (
        "404142434445464748494a4b4c4d4e4f",
        "1011121314151617",
        "000102030405060708090a0b0c0d0e0f",
        "202122232425262728292a2b2c2d2e2f",
        6,
    ),
    (
        "404142434445464748494a4b4c4d4e4f",
        "101112131415161718191a1b",
        "000102030405060708090a0b0c0d0e0f10111213",
        "202122232425262728292a2b2c2d2e2f3031323334353637",
        8,
    ),
]

# [59] Appendix A.1: n is an element of {7, ..., 13}, t of {4, 6, ..., 16}.
LEGAL_NONCE_LENGTHS = range(7, 14)
LEGAL_TAG_LENGTHS = (4, 6, 8, 10, 12, 14, 16)


def main():
    """Print the vector set to stdout."""
    random.seed(20260823)

    for case in SP800_38C_APPENDIX_C:
        print(*case)

    for nonce_length in LEGAL_NONCE_LENGTHS:
        for tag_length in LEGAL_TAG_LENGTHS:
            aad_length = random.choice([0, 1, 13, 16, 17, 40])
            payload_length = random.choice([0, 1, 15, 16, 17, 31, 64, 100])
            print(
                os.urandom(16).hex(),
                os.urandom(nonce_length).hex(),
                os.urandom(aad_length).hex() if aad_length else "-",
                os.urandom(payload_length).hex() if payload_length else "-",
                tag_length,
            )


if __name__ == "__main__":
    main()
