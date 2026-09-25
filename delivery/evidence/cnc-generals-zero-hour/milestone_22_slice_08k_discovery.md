# M22 slice 08K discovery

The post-08J redacted continuation crossed all three initialization categories
and both pre-logic route categories, retained the accepted setup/reachability
and zero-owner teardown aggregates, then stopped before the first game-logic
category. The fixed failure class is **pre-map display over reset-detached
active water unavailable**. No sanitizer category was present.

Source audit shows one adjacent lifecycle graph. `GameEngine::update` invokes
the client before game logic. The client accepts the 08J detached-water terrain
update, updates the display, and immediately calls display draw. That draw has
an otherwise complete detached pre-map owner check, but admission still
depends on a generated-route selector. The later logic phase loads terrain and
08J reattaches water before the ordinary mapped draw path.

08K therefore replaces only the selector dependency with the complete live
owner-state predicate and proves no command mutation, rejection, load failure
rollback, retry and teardown. Slice-08 trial admission, registration and owner-
guard changes were removed before this checkpoint. No private identifier,
path, filename, content, hash, image or raw output is retained.
