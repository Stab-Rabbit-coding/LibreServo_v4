/**
 * @file    test_pal_crypt_ccm.c
 * @brief   Host test for the AES-128-CCM implementation in
 *          `firmware/pal/ls_pal_crypt.c`.
 *
 * Reads whitespace-separated `key nonce aad plaintext maclen` vectors (hex,
 * `-` for an empty field) on stdin, prints the ciphertext-plus-tag of each on
 * stdout for comparison against a reference, and checks round-trip decryption
 * and single-bit tamper rejection in-process.
 *
 * See `firmware/tests/README.md` for the reference used, the 2026-08-23
 * results, and the one spec-mandated difference around empty payloads.
 *
 * Written by Claude Opus 5 (claude-opus-5) under human direction, 2026-08-23.
 */

#include <stdio.h>
#include <string.h>
#include "pal_crypt.h"

static void hex(const char *tag, const uint8_t *b, int n) {
    printf("%s ", tag); for (int i=0;i<n;i++) printf("%02x", b[i]); printf("\n");
}

int main(void) {
    /* Read vectors from stdin: key(hex) nonce(hex) aad(hex or "-") pt(hex or "-") maclen */
    char kh[512], nh[512], ah[4096], ph[4096]; int maclen;
    uint8_t key[16], nonce[16], aad[1024], pt[1024], ct[2048], back[2048];
    int fails = 0, cases = 0;
    while (scanf("%s %s %s %s %d", kh, nh, ah, ph, &maclen) == 5) {
        int klen=0,nlen=0,alen=0,plen=0;
        for (int i=0;kh[2*i] && kh[2*i+1];i++){ sscanf(kh+2*i,"%2hhx",&key[i]); klen=i+1; }
        for (int i=0;nh[2*i] && nh[2*i+1];i++){ sscanf(nh+2*i,"%2hhx",&nonce[i]); nlen=i+1; }
        if (strcmp(ah,"-")) for (int i=0;ah[2*i] && ah[2*i+1];i++){ sscanf(ah+2*i,"%2hhx",&aad[i]); alen=i+1; }
        if (strcmp(ph,"-")) for (int i=0;ph[2*i] && ph[2*i+1];i++){ sscanf(ph+2*i,"%2hhx",&pt[i]); plen=i+1; }
        (void)klen;
        cases++;
        if (pal_crypt_encrypt_aes128_ccm(NULL, pt, (uint16_t)plen, key, nonce, (uint16_t)nlen,
                                         alen?aad:NULL, (uint16_t)alen, (uint8_t)maclen, ct)
            != PAL_STATUS_SUCCESS) { printf("ENCRYPT FAIL case %d\n", cases); fails++; continue; }
        hex("CT", ct, plen+maclen);
        if (pal_crypt_decrypt_aes128_ccm(NULL, ct, (uint16_t)(plen+maclen), key, nonce,
                                         (uint16_t)nlen, alen?aad:NULL, (uint16_t)alen,
                                         (uint8_t)maclen, back) != PAL_STATUS_SUCCESS
            || memcmp(back, pt, plen)) { printf("ROUNDTRIP FAIL case %d\n", cases); fails++; continue; }
        /* tamper check */
        ct[0] ^= 0x01;
        if (pal_crypt_decrypt_aes128_ccm(NULL, ct, (uint16_t)(plen+maclen), key, nonce,
                                         (uint16_t)nlen, alen?aad:NULL, (uint16_t)alen,
                                         (uint8_t)maclen, back) == PAL_STATUS_SUCCESS) {
            printf("TAMPER ACCEPTED case %d\n", cases); fails++; }
    }
    fprintf(stderr, "cases=%d fails=%d\n", cases, fails);
    return fails ? 1 : 0;
}
