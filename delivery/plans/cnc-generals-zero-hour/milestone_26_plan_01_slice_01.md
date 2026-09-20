# M26 plan 01 slice 01: Allocator bootstrap and ABI

## Goal and outcome

An asset-free x86-64 Linux consumer uses the actual original `GameMemory`/`MemoryPool` implementation before `main`, allocates and frees across target boundaries, and applies an optional bounded `MemoryPools.ini` override without VFS, engine strings, engine logging, or CWD dependence.

## Scope

Included: original allocator and pool-default translation units, required Linux PCH/CRT portability, synchronization needed by allocation, complete C++17 allocation/deallocation overload pairing, test-only bounded-count selection, configuration parser, diagnostics/result API, and allocator/config tests. Excluded: original strings/Version, general INI/VFS, gameplay, retail data, and live retuning.

## Dependencies and ordering

M19 is complete. This is the first M26 slice. Slice 02 depends on its allocator and synchronization behavior.

## Entry point and behavior

A static test consumer allocates before `main`; the original global allocation operator initializes the original factory/DMA exactly once. Bootstrap reads at most one explicit low-level override source, validates its bounded contents into fixed storage, applies recognized names once, and exposes a non-allocating diagnostic result. Normal initialization runs the original linkage check. Cross-target consumers allocate in one translation unit and free in another. Shutdown keeps the manager alive when pre-main/static consumers require it.

## Data and state transitions

`uninitialized -> bootstrapping -> active` is one-way for the manager. Pool override state transitions from compiled defaults to a fully validated override snapshot before any configurable named pool is created; malformed/reentrant/late input leaves defaults unchanged and records an error. Live allocation and raw-resource counters return to the expected static-lifetime baseline.

## Authorization and permissions

Not applicable. The only external input is an explicitly named local fixture path. It is read-only, bounded, and never resolves through retail or CWD state.

## Validation and error handling

Positive: absent file defaults; valid recognized override and rounding; original-compatible ignored unknown names; executable-relative and explicit precedence; pre-main allocation; cross-target new/delete; alignment and sized delete; linkage counter; clean ordinary shutdown. Negative/boundary: malformed token, duplicate recognized names, invalid/negative/excessive count, oversized file/line, arbitrary CWD, re-entry, and post-allocation retune. Invalid input must produce a stable diagnostic, keep defaults atomically, and avoid recursive allocation.

Focused commands: configure/build the M26 allocator targets, `ctest --test-dir <preset-dir> -L original-process --output-on-failure`, and the dedicated bootstrap parser unit tests. Repeat the focused allocator/config tests in all four presets before slice completion.

## Implementation surfaces

- `CMakeLists.txt` and M26 test targets.
- Original `GameMemory`/`MemoryInit`/`CriticalSection` sources and directly required headers.
- Narrow Linux compatibility include/source under `src/original_runtime/`.
- Cross-target and configuration tests under `tests/original_runtime/`.

## Acceptance criteria

- Actual original allocator symbols compile/link/run in all four presets without `NDEBUG` forced on the target.
- Pre-main, cross-target, aligned/sized, and static-lifetime paths are exercised with explicit counts.
- All required pool-file cases are bounded, deterministic, CWD-independent, M27-independent, and cannot retune live pools.
- Removing the original allocator provider produces a real link failure in the negative control.

## Commit boundary

Commit the governing/slice plans, allocator/config implementation, focused tests, and passing four-preset evidence as one coherent slice.

## Result

Complete. The actual original allocator and pool-default units build without a target-wide `NDEBUG`, and the allocator/config matrix passes in GCC/Clang Debug/Release. The matrix covers defaults, valid rounded override with compatible unknown-name ignore behavior, malformed/invalid/duplicate/oversized/missing inputs, arbitrary CWD, pre-main allocation, re-entry detection, late-retune rejection, cross-target raw allocation/free counts, alignment, and the original linkage check.
