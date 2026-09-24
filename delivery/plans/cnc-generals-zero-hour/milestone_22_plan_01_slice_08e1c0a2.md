# M22 08E1C0A2: parser-backed template/settings closure

After C0A1, use only source INI parser/provider paths to populate generated
`PlayerTemplateStore` and `MultiplayerSettings` values required by multiplayer
UI. The generated fixture must use the original top-level `PlayerTemplate`,
`MultiplayerSettings`, `MultiplayerColor`, and
`MultiplayerStartingMoneyChoice` grammar, loaded through `INI::load`.

The slice publishes a caller-owned PlayerTemplateStore before parsing; the
source MultiplayerSettings parser owns its singleton allocation and the probe
releases it before each generation ends. Cover a valid template/settings/color/
default-money bundle, same-name source update, missing template/out-of-range
color controls, malformed field rejection, two generations, and link-removal
controls for both PlayerTemplate and multiplayer parser providers. No map cache,
persona, layout owner, retail data, media, pixels, or network service is in
scope.

Acceptance: focused GCC/Clang and sanitizer checks, fresh six-configuration
portable suite, strict host LSan in both sanitizers, physical Vulkan, serial
LAN, source identity/removal, evidence, and one isolated commit.
