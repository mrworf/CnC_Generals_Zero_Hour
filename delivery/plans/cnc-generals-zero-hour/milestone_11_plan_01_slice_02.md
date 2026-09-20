# M11 slice 02: deterministic virtual datagram sessions

## Goal and observable outcome

Two in-process peers discover, join, negotiate matching data/map identities, exchange ordered commands, and disconnect through a deterministic virtual datagram network. Tests can inject loss, duplication, reordering, corruption, delay, and timeout without sleeps or real sockets.

## Scope

- Define a transport-neutral datagram endpoint and virtual network with explicit addresses and monotonic 32-bit ticks.
- Implement advertisement/direct-join session transitions, bounded retry/timeout behavior, identity negotiation, duplicate/old command suppression, and diagnostics.
- Support deterministic next-datagram fault actions: drop, duplicate, reorder, corrupt, and delay.
- Test discovery, direct connection, all fault modes, timeout, clean disconnect, version rejection, and data/map mismatch.

## Non-scope

No OS sockets or process launch is included. Reliability is a minimal M11 harness proving retained ordering/retry boundaries, not the full M16 match/synchronization layer.

## Dependencies and ordering

Depends on slice 01 packet validation. Slice 03 adapts the same session behavior to POSIX UDP.

## Entry point and end-to-end behavior

A test creates named virtual endpoints, starts one host and one joining peer, advances the virtual clock, pumps sessions, and observes state/events. Discovery or direct-connect leads to identity negotiation; matching peers exchange a command and disconnect, while mismatches reject with a diagnostic.

## Data and state transitions

Sessions transition through idle, advertising/discovering, joining, connected, rejected/timed-out, and disconnected. Receive processing decodes into temporary packets before applying valid transitions. Sequence tracking suppresses duplicates and stale reordered commands.

## Authorization and permissions

Not applicable: all datagrams and clocks are in-memory test-owned state.

## Validation, errors, and recovery

Corrupt/version-invalid packets are logged and ignored without mutation. Join retries are bounded and wrap-safe. Identity mismatch transitions to rejected with expected/received values; timeout and explicit disconnect are distinct observable outcomes. A fresh session can reconnect after teardown.

## Expected implementation surfaces

`include/zh/lan/transport.h`, `include/zh/lan/session.h`, `src/lan/virtual_transport.cpp`, `src/lan/session.cpp`, `tests/lan/test_virtual_transport.cpp`, and CMake declarations.

## Positive tests

- Discovery and direct-connect both establish matching peers.
- Command exchange and clean disconnect are observable.
- Delayed/reordered/duplicated traffic preserves state and command order; a dropped join is retried.

## Negative tests

- Corruption and wrong version do not mutate state.
- Data/map mismatch rejects.
- Silent peer times out across clock wrap.

## Required commands

```sh
cmake --build --preset linux-gcc-debug
ctest --preset linux-gcc-debug -R lan_virtual --output-on-failure
```

## Acceptance criteria

All deterministic scenarios pass without real time, real sockets, or unbounded queues, and failures expose actionable diagnostics.

## Commit boundary

Commit transport/session APIs, virtual implementation, tests, CMake wiring, and updated plan evidence as `delivery: M11 slice 02 simulate LAN sessions`.
