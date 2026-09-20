# M4 retail VFS acceptance evidence

## Context

- Platform: Arch Linux x86-64.
- Compilers/configurations: GCC and Clang, Debug and Release.
- Corpus: user-owned combined Zero Hour/base Generals installation, selected locale `English`.
- Privacy boundary: validation used runtime-only local paths. No retail bytes, hashes, copied assets, or host paths are recorded here.
- Device boundary: `--verify-data` reported `devices skipped`; it initialized no window, GPU, audio, or video device.

## Portable and synthetic validation

Each canonical preset configured and built successfully, then passed both `ctest --preset <preset> -L data --output-on-failure` and its complete 20-test suite:

- `linux-gcc-debug`
- `linux-clang-debug`
- `linux-gcc-release`
- `linux-clang-release`

The data suite covers CLI/config precedence, locale selection and ambiguity, identical roots, arbitrary working directories, all five mount layers, stable archive order, ordinary cross-layer duplicates, case-only ambiguity, traversal, bad BIG identifiers/counts/NUL termination/table boundaries/ranges/size limits, missing resources, and positive/negative metadata fixtures for every supported format family.

## Retail verification

An isolated GCC Debug build enabled `ZH_ENABLE_RETAIL_TESTS` with the three local cache inputs. `ctest -L data` passed all four tests, including `retail_data_verification` from the build directory rather than the source working directory.

The stable logical inventory reported:

| Item | Observed |
|---|---:|
| Mounted BIG archives | 36 |
| Resolved logical resources | 25,513 |
| INI | 224 |
| CSF | 1 |
| W3D | 8,901 |
| DDS | 6,602 |
| TGA | 772 |
| WAV | 8,475 |
| MP3 | 56 |
| Bink | 70 |
| WWShade PSO/VSO | 16 |
| Embedded TrueType/OpenType fonts | 0 |
| NOX/LZH entries | 0 |
| Granny entries | 0 |

The selected English configuration names `Arial Unicode MS` as a system font and does not enable `LocalFontFile`. RefPack and zlib remain bounded supported compression paths, but no standalone resource with those synthetic test extensions exists in this corpus. Required representative INI, CSF, W3D, DDS, TGA, WAV, MP3, Bink, and WWShade logical assets all passed their metadata checks.

## Result

PRE-009 is satisfied for the supplied English corpus. Verification is deterministic, read-only, device-free, independent of the current working directory, and reports exact logical failures. No unexpected required asset format blocked M4.
