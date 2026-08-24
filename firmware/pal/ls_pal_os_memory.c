/**
 * @file    ls_pal_os_memory.c
 * @brief   OPTIGA(TM) Trust M PAL memory implementation for LibreServo v4.
 *
 * Implements [57] `include/pal/pal_os_memory.h`.
 *
 * DESIGN DECISION, recorded because it is not the obvious one: `pal_os_malloc`
 * and `pal_os_calloc` return NULL and `pal_os_free` is a no-op — this port has
 * no heap.
 *
 * Rationale: this is a safety-relevant actuator with a hard real-time control
 * loop (TODO.md 7.1).  A heap introduces unbounded allocation latency and
 * fragmentation-dependent failure, both of which are exactly what a servo must
 * not have.  The host library's own service layer allocates its instances from
 * a fixed pool sized by OPTIGA_CMD_MAX_REGISTRATIONS
 * (firmware/config/optiga_lib_config.h) rather than from this allocator, so a
 * heapless build is supported by construction.
 *
 * If a future host-library revision starts requiring dynamic allocation, the
 * NULL return will surface it immediately and loudly at bring-up, which is the
 * intent — silently linking in `malloc` would hide a real change in the
 * library's memory model.
 *
 * Written by Claude Opus 5 (claude-opus-5) under human direction, 2026-08-23.
 */

#include "pal_os_memory.h"

#include <string.h>

void *pal_os_malloc(uint32_t block_size) {
    (void)block_size;
    return NULL;
}

void *pal_os_calloc(uint32_t number_of_blocks, uint32_t block_size) {
    (void)number_of_blocks;
    (void)block_size;
    return NULL;
}

void pal_os_free(void *block) {
    (void)block;
}

void pal_os_memcpy(void *p_destination, const void *p_source, uint32_t size) {
    if ((p_destination == NULL) || (p_source == NULL) || (size == 0U)) {
        return;
    }
    (void)memcpy(p_destination, p_source, (size_t)size);
}

void pal_os_memset(void *p_buffer, uint32_t value, uint32_t size) {
    if ((p_buffer == NULL) || (size == 0U)) {
        return;
    }
    (void)memset(p_buffer, (int)value, (size_t)size);
}
