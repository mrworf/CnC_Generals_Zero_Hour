# miniaudio provenance and build review

- Upstream: https://github.com/mackron/miniaudio
- Release: `0.11.25` (published 2026-03-03)
- Release page: https://github.com/mackron/miniaudio/releases/tag/0.11.25
- Pinned commit: `9634bedb5b5a2ca38c1ee7108a9358a4e233f14d`
- Header URL: https://raw.githubusercontent.com/mackron/miniaudio/9634bedb5b5a2ca38c1ee7108a9358a4e233f14d/miniaudio.h
- `miniaudio.h` SHA-256: `ac7af4de748b7e26b777f37e01cee313a308a7296a3eb080e2906b320cc55c89`
- License URL: https://raw.githubusercontent.com/mackron/miniaudio/9634bedb5b5a2ca38c1ee7108a9358a4e233f14d/LICENSE
- `LICENSE` SHA-256: `457f1b500e0adf6bc059edddfa78a2f62012e7c3bb43476c20e0bd23b25ba0eb`
- Selected upstream license alternative: MIT No Attribution (MIT-0).

The single-header implementation is compiled directly into `zh_audio_miniaudio`.
The project defines `MA_NO_FLAC`, `MA_NO_ENCODING`, and `MA_NO_GENERATION`;
PCM WAV, Microsoft/IMA ADPCM WAV, and MP3 decoding stay enabled. The normal
Linux device backends remain available for later hardware use. M12 tests do not
open a device and can select the deterministic null sink.

The upstream release notes identify this release as a WAV-decoder bug-fix
release. Review of the license and compile-time feature switches found no
license conflict with this repository. CMake consumes only the two files in
this directory; it contains no `FetchContent`, external project, URL download,
or package-manager fallback. `tests/audio/test_miniaudio_provenance.py` pins the
hashes and enforces that offline rule.
