# M22 slice 08N0: modeled volume-shadow aggregate

Accepted 08N0A (`2c529c1`) and 08N0B (`b3e0abc`) now compose the generated
modeled Drawable-to-volume-shadow lifecycle. The constructor publishes its
module, scene-linked render object and `DrawableInfo` before exact bounded
caster admission. A map-loaded, map-owned source-buffer provider then readies
only that caster, and Recording draws its nonzero source geometry in the
terrain–tracks–volume-stencil–water order. Replacement releases the old caster
before scene removal, readies the new one after publication, and rolls back
new model/shadow/geometry/device state on every injected boundary. Removal and
two-generation re-entry leave zero caster and Recording-device residual.

The focused owner, standalone-volume, generated constructor/replacement and
late-modeled-frame tests passed 4/4 in both GCC and Clang Debug. The frozen
08N0B source also passed six complete GCC/Clang Debug, Release and ASan+UBSan
builds and canonical nonretail suites (267/267 each), strict host LSan focused
groups (11/11 each), physical Vulkan controls (2/2), serial host LAN in all
six configurations (4/4 each), dependency ledger and diff checks.

This is a generated-owner aggregate only. The active-bib producer remains
guarded, and retail Recording continuation remains slice 08. No retail
source, selector, path, logical name, bytes, hash or raw output is retained.
The original symlink/content is untouched and the unrelated renderer
diagnostic remains unstaged.
