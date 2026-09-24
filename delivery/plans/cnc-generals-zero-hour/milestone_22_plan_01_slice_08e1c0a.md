# M22 08E1C0A: generated multiplayer slot/template/settings data closure

This is split: C0A1 proves `GameInfo`/`GameSlot`; C0A2 proves parser-backed
templates/settings; C0B follows both. Prove one project-owned two-generation source transaction for `GameInfo`,
occupied/empty/AI `GameSlot` values, `PlayerTemplateStore` lookup/fallback and
`MultiplayerSettings` colour lookup. Reject missing slots/templates/colours,
duplicate or stale publication, and release all providers. No layout, map,
persona, network service, media or retail data is included.

## Aggregate transaction

This final C0A aggregate adds no production source. One generated executable
witness requires the accepted source GameInfo/GameSlot transaction before the
accepted parser-backed template/settings transaction, and requires both to
report two-generation zero-provider completion. Missing witnesses or changed
result markers fail deterministically; component slices retain malformed,
duplicate, stale-publication, removal, and teardown controls.
