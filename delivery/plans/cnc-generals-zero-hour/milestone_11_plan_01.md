# M11 delivery plan: POSIX LAN transport and local peer harness

## Goal

Close M11 with a bounded fixed-width LAN protocol, deterministic virtual datagram coverage, and a real two-process POSIX UDP proof on distinct loopback identities. The proof must negotiate data/map identity, exchange a command, and disconnect cleanly without retail data, a display, or another host.

## Authority and constraints

- Milestone contract: `delivery/milestones/cnc-generals-zero-hour/M11-lan-transport.md`.
- Product authority: `docs/zero-hour-linux-port-plan.md`, especially serialization, LAN, M11, and loopback-discovery sections.
- Direct providers M3 and M4 are complete at transaction start `eb8af4ba2579614e0b2e2110bed2f143e37cbac5`.
- Retail files and the ignored original-game symlink remain read-only. Tests use project-owned synthetic identities and payloads only.
- Lobby port 8086 and gameplay base port 8088 remain named protocol identities. Local tests bind `127.0.0.2` and `127.0.0.3`.
- No second host, Windows peer, Internet service, GPU, audio device, display, or network fetch is required.

## Delivery slices

1. [Slice 01: bounded fixed-width LAN packet codec](milestone_11_plan_01_slice_01.md)
2. [Slice 02: deterministic virtual datagram sessions](milestone_11_plan_01_slice_02.md)
3. [Slice 03: POSIX UDP and two-process headless harness](milestone_11_plan_01_slice_03.md)

The slices are dependency ordered. The virtual transport carries slice 01 packets and proves the state machine under deterministic faults. The POSIX adapter and process harness carry the same packets and state transitions over real nonblocking UDP.

## Milestone validation

For each canonical preset:

```sh
cmake --preset <preset>
cmake --build --preset <preset>
ctest --preset <preset> -L 'lan|headless' --output-on-failure
```

Acceptance requires positive direct-connect and virtual-discovery paths plus actionable rejection of invalid type/version/length/count/string encodings, timeout, corruption, and data/map identity mismatch. The real test launches two headless processes on distinct loopback addresses, observes join/negotiation/command/disconnect, and verifies isolated writable state. A failed loopback broadcast attempt may be recorded as an environment limitation because virtual discovery and real direct-connect are the contract fallback.

## Non-scope

Full match completion (M16), map transfer content, physical-subnet broadcast assurance, mixed Windows/Linux compatibility, Internet services, and replacement of retained reliability/simulation layers are excluded.

## Commit record

- Slice 01: `62ebbf9b3255d29040910386a9aafd2425d534d8`
- Slice 02: `0ff7e62f77a71cf48296d68dc296eafa56a6ec36`
- Slice 03: `dec6f041aa98df9ac3e158668cce3cfd05d888d8`
