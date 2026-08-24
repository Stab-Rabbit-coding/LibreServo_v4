/**
 * @file    test_pal_crypt_prf.c
 * @brief   Host test for the TLS 1.2 PRF (SHA-256) implementation in
 *          `firmware/pal/ls_pal_crypt.c`.
 *
 * Reads `secret label seed outputlength` vectors (hex; `-` for an empty seed)
 * on stdin and prints the derived key of each on stdout, for comparison
 * against an independent implementation of IETF RFC 5246 [58] section 5.
 *
 * Written by Claude Opus 5 (claude-opus-5) under human direction, 2026-08-23.
 */

#include <stdio.h>
#include <string.h>
#include "pal_crypt.h"
int main(void) {
    char sh[512], lh[512], dh[512]; int outlen;
    uint8_t secret[128], label[128], seed[128], out[256];
    while (scanf("%s %s %s %d", sh, lh, dh, &outlen) == 4) {
        int sl=0, ll=0, dl=0;
        for (int i=0;sh[2*i]&&sh[2*i+1];i++){ sscanf(sh+2*i,"%2hhx",&secret[i]); sl=i+1; }
        for (int i=0;lh[2*i]&&lh[2*i+1];i++){ sscanf(lh+2*i,"%2hhx",&label[i]); ll=i+1; }
        for (int i=0;dh[2*i]&&dh[2*i+1];i++){ sscanf(dh+2*i,"%2hhx",&seed[i]); dl=i+1; }
        if (pal_crypt_tls_prf_sha256(NULL, secret,(uint16_t)sl, label,(uint16_t)ll,
                                     seed,(uint16_t)dl, out,(uint16_t)outlen) != PAL_STATUS_SUCCESS) {
            printf("FAIL\n"); continue; }
        for (int i=0;i<outlen;i++) printf("%02x", out[i]); printf("\n");
    }
    return 0;
}
