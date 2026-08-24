#!/bin/bash
#
# fetch_optiga_host_library.sh — fetch the pinned Infineon OPTIGA(TM) Trust M
# Host Library for C into firmware/external/optiga-trust-m/.
#
# The host library is an upstream third-party dependency and is deliberately
# NOT vendored into this repository:
#
#   * it is ~2 MB of Infineon-maintained C that this project does not modify,
#     and vendoring it would obscure the boundary between upstream code and
#     LibreServo code that AGENTS.md section 1 requires be legible;
#   * pinning by commit gives the same reproducibility as vendoring while
#     leaving the upstream history intact and auditable.
#
# Attribution (AGENTS.md, and the MIT license's own retention clause):
#   OPTIGA(TM) Trust M Host Library for C
#   Copyright (c) 2018-2024 Infineon Technologies AG
#   SPDX-License-Identifier: MIT
#   Cataloged in REFERENCES.md as [57].
#
# Governing references:
#   [57] Infineon, OPTIGA(TM) Trust M Host Library for C, Ver 5.8.1.
#   [53] Infineon, OPTIGA(TM) Trust M Solution Reference Manual, Rev 3.70.
#
# Written by Claude Opus 5 (claude-opus-5) under human direction, 2026-08-23.

set -euo pipefail

# Pinned upstream revision.  Update this deliberately, never automatically:
# a host-library bump changes the security-relevant COMMS layer and must be
# re-reviewed against [53] section 6.6 before it is accepted.
readonly OPTIGA_REPO="https://github.com/Infineon/optiga-trust-m.git"
readonly OPTIGA_COMMIT="67cfd0e589fc09f936d4e4ce4fa35eacc43af72f"
readonly OPTIGA_VERSION="Ver 5.8.1"

# Resolve the firmware/ directory from this script's own location, so the
# script works regardless of the caller's working directory.
readonly HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly FIRMWARE_DIR="$(dirname "${HERE}")"
readonly DEST_DIR="${FIRMWARE_DIR}/external/optiga-trust-m"

main() {
    if [ -d "${DEST_DIR}/.git" ]; then
        printf 'Host library already present at %s\n' "${DEST_DIR}"
        local current
        current="$(git -C "${DEST_DIR}" rev-parse HEAD)"
        if [ "${current}" = "${OPTIGA_COMMIT}" ]; then
            printf 'Already at the pinned commit %s — nothing to do.\n' \
                "${OPTIGA_COMMIT}"
            return 0
        fi
        printf 'WARNING: checked out %s, pinned commit is %s.\n' \
            "${current}" "${OPTIGA_COMMIT}" >&2
        printf 'Refusing to move it automatically. Resolve by hand.\n' >&2
        return 1
    fi

    mkdir -p "$(dirname "${DEST_DIR}")"
    printf 'Cloning %s\n' "${OPTIGA_REPO}"
    git clone --no-checkout "${OPTIGA_REPO}" "${DEST_DIR}"
    git -C "${DEST_DIR}" checkout --detach "${OPTIGA_COMMIT}"

    # Verify what we actually got, rather than trusting the checkout silently.
    local version_line
    version_line="$(grep -m1 'OPTIGA_LIB_VERSION' \
        "${DEST_DIR}/include/optiga_lib_version.h")"
    printf 'Checked out %s\n' "${OPTIGA_COMMIT}"
    printf 'Reported version: %s\n' "${version_line}"
    if ! printf '%s' "${version_line}" | grep -q "${OPTIGA_VERSION}"; then
        printf 'ERROR: expected %s, header says otherwise.\n' \
            "${OPTIGA_VERSION}" >&2
        return 1
    fi
    printf 'OK — host library matches REFERENCES.md [57].\n'
}

main "$@"
