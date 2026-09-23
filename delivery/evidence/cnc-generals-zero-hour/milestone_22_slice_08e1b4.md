# M22 08E1B4: generated SinglePlayerLoadScreen owner evidence

The actual source `SinglePlayerLoadScreen` composes only generated project-owned
WND, Campaign/Mission, image, video-buffer, text, mouse and audio providers.
Two generations inject ambient registration failure, retry, update progress,
and drain source window destruction to zero audio/windows/display strings.
`LoadScreen.cpp` identity and negative provider removal are required.

- Focused GCC and Clang source/identity/removal: 3/3 each.
- Fresh non-GPU/non-LAN suites: 228/228 in GCC Debug, GCC Release, Clang
  Debug, Clang Release, GCC sanitizer and Clang sanitizer.  Sanitizer broad
  runs use `ASAN_OPTIONS=detect_leaks=0` only for the established ptrace limit.
- Strict host `detect_leaks=1` focused GCC and Clang checks: 3/3 each.
  A separate strict-UB GCC exploration is diagnostic-only: it exposes existing
  original-source enum/null diagnostics outside this slice and is not used for
  acceptance.
- Physical Vulkan: 9/9. Serial LAN: 4/4 in all six configurations. `git diff
  --check` passes. The unrelated renderer diagnostic remains unstaged.
