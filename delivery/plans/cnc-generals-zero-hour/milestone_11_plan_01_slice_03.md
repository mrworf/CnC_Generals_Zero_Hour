# M11 slice 03: POSIX UDP and two-process headless harness

## Goal and observable outcome

Expose developer LAN overrides and prove two real headless processes on `127.0.0.2` and `127.0.0.3` can direct-connect over nonblocking POSIX UDP, negotiate data/map identity, exchange commands, and disconnect cleanly with isolated state directories.

## Scope

- Add a POSIX IPv4 UDP endpoint using `getaddrinfo`, `socket`, `bind`, broadcast option, `fcntl(O_NONBLOCK)`, `poll`, `sendto`, `recvfrom`, `close`, and `errno` diagnostics.
- Add `--lan-bind-address`, `--lan-discovery-address`, and `--lan-direct-connect` headless CLI options, plus harness role/identity/time-budget options needed by automated proof.
- Retain lobby/game identities 8086/8088 and permit narrow test port override only to avoid ambient conflicts.
- Launch two real processes on distinct loopback addresses and verify the complete direct-connect handshake, command, and disconnect transcript.
- Attempt loopback broadcast discovery separately and record whether the host delivers it; virtual discovery plus real direct-connect remains accepted fallback.

## Non-scope

No second host, network namespace, Windows peer, physical-subnet guarantee, Internet service, retail payload, or full LAN match is required.

## Dependencies and ordering

Depends on slices 01 and 02. Reuses their packet and session semantics over a real endpoint.

## Entry point and end-to-end behavior

`zh_main --headless` parses the three developer overrides. When a LAN role is selected it binds the requested address, uses discovery or direct-connect, pumps nonblocking datagrams until success/failure/deadline, writes normal isolated logs, and exits with actionable status. The Python test starts host then joiner with distinct loopback identities and checks both transcripts.

## Data and state transitions

Socket ownership is RAII and closes on all exits. Receive buffers are fixed-size and packet validation precedes session mutation. Poll deadlines derive from wrap-safe monotonic ticks. Connected peers exchange one deterministic command, the joiner disconnects, and both exit successfully.

## Authorization and permissions

Tests require only user-owned IPv4 loopback sockets and subprocesses. If sandbox policy blocks loopback, rerun the same validation with the required permission; do not weaken the test.

## Validation, errors, and recovery

Reject invalid/unresolvable addresses, duplicate options, invalid role/port/identity values, bind/send/receive/poll errors, identity mismatch, timeout, and peer disconnect with contextual diagnostics. Clean socket/process teardown permits immediate retry.

## Expected implementation surfaces

`include/zh/lan/posix_udp.h`, `src/lan/posix_udp.cpp`, LAN-aware headless runtime/main code, `tests/lan/test_posix_udp.cpp`, `tests/lan/test_two_processes.py`, platform evidence, CMake wiring, and plan evidence.

## Positive tests

- POSIX endpoint sends/receives between `127.0.0.2` and `127.0.0.3` without blocking.
- CLI accepts all three developer overrides and preserves default 8086/8088 identities.
- Two headless processes direct-connect, join, negotiate, exchange a command, disconnect, and isolate writable state.
- Broadcast probe reports delivered or explicitly unverified.

## Negative tests

- Invalid/duplicate addresses and role options fail with usage diagnostics.
- Data/map mismatch returns rejection rather than connection.
- Unreachable peer times out within the bounded test deadline.

## Required commands

```sh
cmake --build --preset linux-gcc-debug
ctest --preset linux-gcc-debug -L 'lan|headless' --output-on-failure
```

Then run the same configure/build/labeled test commands for all four canonical presets.

## Acceptance criteria

The real process proof passes on distinct loopback identities and all socket paths are nonblocking, bounded, diagnostic, and cleanly released. Broadcast status is recorded without overstating physical discovery.

## Commit boundary

Commit the POSIX endpoint, CLI/runtime integration, real-process tests, evidence, CMake wiring, and completed plan records as `delivery: M11 slice 03 prove POSIX peers`.
