#!/usr/bin/env bash
set -euo pipefail

# Network access is confined to --acquire. Configuration, builds and tests
# never fetch dependencies. --offline-sources accepts three sibling public
# checkouts (bgfx, bx, bimg) at the exact pinned revisions.
repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
source "${repo_root}/third_party/bgfx_shaderc.lock"
cache_root="${repo_root}/build/bgfx-toolchain"
source_root="${cache_root}/source"
mode="${1:---offline-sources}"
if [[ "${mode}" == "--offline-sources" ]]; then
    if [[ $# -ne 2 ]]; then
        echo "usage: $0 --offline-sources <directory-containing-bgfx-bx-bimg> | --acquire" >&2
        exit 2
    fi
    supplied_root="$(realpath "$2")"
    for project in bgfx bx bimg; do
        case "${project}" in
            bgfx) revision="${BGFX_REV}";;
            bx) revision="${BX_REV}";;
            bimg) revision="${BIMG_REV}";;
        esac
        supplied_dir="${supplied_root}/${project}"
        if [[ "$(git -C "${supplied_dir}" rev-parse HEAD 2>/dev/null || true)" != "${revision}" ]]; then
            echo "wrong supplied ${project} revision (required ${revision})" >&2; exit 1
        fi
        if [[ "$(sha256sum "${supplied_dir}/LICENSE" | cut -d ' ' -f 1)" != "${LICENSE_SHA256}" ]]; then
            echo "supplied ${project} license differs from reviewed BSD-2-Clause text" >&2; exit 1
        fi
    done
    mkdir -p "${source_root}"
    for project in bgfx bx bimg; do
        if [[ ! -e "${source_root}/${project}" ]]; then
            git clone --quiet --local --no-hardlinks "${supplied_root}/${project}" "${source_root}/${project}"
        fi
    done
elif [[ "${mode}" == "--acquire" ]]; then
    if [[ $# -ne 1 ]]; then echo "--acquire takes no directory" >&2; exit 2; fi
    mkdir -p "${source_root}"
    for project in bgfx bx bimg; do
        case "${project}" in
            bgfx) revision="${BGFX_REV}";;
            bx) revision="${BX_REV}";;
            bimg) revision="${BIMG_REV}";;
        esac
        project_dir="${source_root}/${project}"
        if [[ ! -e "${project_dir}" ]]; then
            git init -q "${project_dir}"
            git -C "${project_dir}" remote add origin "https://github.com/bkaradzic/${project}.git"
        fi
        if [[ ! -d "${project_dir}/.git" ]]; then
            echo "refusing non-Git source path: ${project_dir}" >&2; exit 1
        fi
        current_revision="$(git -C "${project_dir}" rev-parse HEAD 2>/dev/null || true)"
        if [[ -n "${current_revision}" && "${current_revision}" != "${revision}" ]]; then
            echo "refusing to replace an existing ${project} checkout at ${current_revision}" >&2; exit 1
        fi
        if [[ -z "${current_revision}" ]]; then
            git -C "${project_dir}" fetch --depth 1 origin "${revision}"
            git -C "${project_dir}" checkout --detach --quiet FETCH_HEAD
        fi
    done
else
    echo "unknown mode: ${mode}" >&2
    exit 2
fi

for project in bgfx bx bimg; do
    case "${project}" in
        bgfx) revision="${BGFX_REV}";;
        bx) revision="${BX_REV}";;
        bimg) revision="${BIMG_REV}";;
    esac
    project_dir="${source_root}/${project}"
    if [[ "$(git -C "${project_dir}" rev-parse HEAD 2>/dev/null || true)" != "${revision}" ]]; then
        echo "wrong ${project} source revision (required ${revision})" >&2; exit 1
    fi
    if [[ "$(sha256sum "${project_dir}/LICENSE" | cut -d ' ' -f 1)" != "${LICENSE_SHA256}" ]]; then
        echo "${project} license differs from reviewed BSD-2-Clause text" >&2; exit 1
    fi
    if [[ -n "$(git -C "${project_dir}" ls-files --others --exclude-standard)" ]]; then
        echo "${project} cache contains unreviewed untracked files" >&2; exit 1
    fi
done

patch_file="${repo_root}/third_party/bgfx_shaderc_glsl.patch"
bgfx_dir="${source_root}/bgfx"
if git -C "${bgfx_dir}" apply --reverse --check "${patch_file}" 2>/dev/null; then
    if [[ "$(git -C "${bgfx_dir}" diff --name-only)" != $'tools/shaderc/shaderc.cpp\ntools/shaderc/shaderc_spirv.cpp' ]]; then
        echo "bgfx checkout has changes outside the reviewed shaderc patch" >&2; exit 1
    fi
    if ! git -C "${bgfx_dir}" diff --binary -- tools/shaderc/shaderc.cpp tools/shaderc/shaderc_spirv.cpp | cmp -s - "${patch_file}"; then
        echo "bgfx shaderc changes differ from the reviewed patch" >&2; exit 1
    fi
else
    if ! git -C "${bgfx_dir}" diff --quiet; then
        echo "bgfx checkout has unreviewed tracked changes" >&2; exit 1
    fi
    git -C "${bgfx_dir}" apply --check "${patch_file}"
    git -C "${bgfx_dir}" apply "${patch_file}"
fi
for project in bx bimg; do
    if ! git -C "${source_root}/${project}" diff --quiet; then
        echo "${project} checkout has unreviewed tracked changes" >&2; exit 1
    fi
done

make -C "${bgfx_dir}" shaderc -j "${ZH_BGFX_JOBS:-4}"
mkdir -p "${cache_root}/bin"
cp "${bgfx_dir}/tools/bin/linux/shaderc" "${cache_root}/bin/shaderc"
"${cache_root}/bin/shaderc" --version
echo "Pinned bgfx shader compiler ready; all later CMake operations are offline."
