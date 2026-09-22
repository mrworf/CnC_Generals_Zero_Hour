#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
source "${repo_root}/third_party/bgfx_shaderc.lock"
source_root="${repo_root}/build/bgfx-toolchain/source"
for project in bgfx bx bimg; do
    case "${project}" in
        bgfx) revision="${BGFX_REV}";;
        bx) revision="${BX_REV}";;
        bimg) revision="${BIMG_REV}";;
    esac
    project_dir="${source_root}/${project}"
    if [[ "$(git -C "${project_dir}" rev-parse HEAD 2>/dev/null || true)" != "${revision}" ]]; then
        echo "missing or wrong pinned ${project} source; run tools/renderer/bootstrap_bgfx_shaderc.sh --acquire first" >&2
        exit 1
    fi
    if [[ "$(sha256sum "${project_dir}/LICENSE" | cut -d ' ' -f 1)" != "${LICENSE_SHA256}" ]]; then
        echo "${project} license differs from reviewed BSD-2-Clause text" >&2
        exit 1
    fi
done

if ! git -C "${source_root}/bgfx" diff --binary | \
    cmp -s - "${repo_root}/third_party/bgfx_shaderc_glsl.patch"; then
    echo "bgfx source differs from the reviewed shaderc-only patch" >&2
    exit 1
fi
for project in bx bimg; do
    if ! git -C "${source_root}/${project}" diff --quiet; then
        echo "${project} source differs from reviewed pin" >&2
        exit 1
    fi
done

# The shaderc-only patch is reviewed by its bootstrap script and does not
# alter bgfx runtime sources. This invocation never accesses the network.
make -C "${source_root}/bgfx" .build/projects/gmake-linux-gcc
make -C "${source_root}/bgfx/.build/projects/gmake-linux-gcc" \
    bgfx-shared-lib config=release64 -j "${ZH_BGFX_JOBS:-4}"
library="${source_root}/bgfx/.build/linux64_gcc/bin/libbgfx-shared-libRelease.so"
test -f "${library}"
echo "Pinned public bgfx runtime ready: ${BGFX_REV}"
