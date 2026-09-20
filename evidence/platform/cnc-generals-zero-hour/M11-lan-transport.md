# M11 POSIX LAN transport evidence

## Result

PRE-014 is complete for the source-defined Linux-local acceptance boundary. Two real headless processes bind distinct `127.0.0.2` and `127.0.0.3` identities on lobby port 8086, direct-connect, negotiate matching data and map identities, exchange a deterministic command, disconnect cleanly, and keep writable state isolated. The POSIX adapter also proves direct datagrams on gameplay base port 8088.

## Protocol and virtual transport coverage

- The fixed-width little-endian codec validates magic, version, message type, declared length, bounded UTF-8 strings, bounded command count, command lengths, and trailing bytes before returning a packet.
- Session timers use modulo-32-bit elapsed clocks and are tested across wrap-around.
- Deterministic virtual tests prove broadcast discovery and direct join; dropped-join retry; duplicate suppression; reordered command buffering; corruption/version rejection without state mutation; delayed delivery; timeout; explicit disconnect; and data/map identity rejection.
- Virtual queues, receive batches, reorder windows, packet sizes, strings, and collections are bounded.

## POSIX and process coverage

- The real endpoint uses IPv4 `getaddrinfo`, `socket`, `bind`, `SO_BROADCAST`, `fcntl(O_NONBLOCK)`, `poll`, `sendto`, `recvfrom`, `close`, and contextual `errno` diagnostics.
- Headless CLI accepts the developer overrides `--lan-bind-address`, `--lan-discovery-address`, and `--lan-direct-connect`; invalid, duplicate, or role-incompatible values fail with usage diagnostics.
- The real-process test also rejects a data-identity mismatch and times out an unreachable direct peer.
- Tests require only local user-owned loopback sockets. The restricted execution sandbox denied socket creation; the unchanged tests passed with local-network permission.

## Loopback discovery observation

The `127.255.255.255` loopback broadcast probe was not delivered on this Arch Linux host. Physical/subnet broadcast is therefore explicitly **unverified**, not inferred. Per the M11 contract, discovery acceptance comes from deterministic virtual broadcast while the real two-process acceptance uses direct connect.

## Validation

All four canonical presets configured and built, then passed `ctest --preset <preset> -L 'lan|headless' --output-on-failure` (7/7 each):

- `linux-gcc-debug`
- `linux-clang-debug`
- `linux-gcc-release`
- `linux-clang-release`

The canonical `linux-gcc-debug` full suite also passed (41/41), preserving M0-M10 behavior. All tests are asset-free and contain no retail media or private host paths.
