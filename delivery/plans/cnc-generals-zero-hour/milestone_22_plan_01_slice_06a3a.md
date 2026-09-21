# M22 plan 01 slice 06A3A: original source light expansion

## Dependency and outcome

Requires accepted 06A2. Restore the **original** `DX8Wrapper::Set_Light_Environment` body as the single CPU/native source of equivalent ambient and up to four source directional/point lights. `Set_Light` publishes a bounded physical record (type, diffuse/ambient/specular, direction/center, range, authored point attenuation) to the device edge while preserving original slot-disable behavior and source command order. Original mesh and `RenderInfoClass` own scene light selection; adapter only copies already selected values. Original material source/opacity, normal transform and view/world selection remain source-owned. Retain the typed lit draw edge until B; A may witness source `LightEnvironmentClass`/wrapper direct methods and source category light selection, not claim a lit physical pixel or full frame. Missing required environment, >4 lights, invalid radius/attenuation/nonfinite light and stale-owner/generation cases fail closed without partial publication. Test original zero/directional/point/four-light and slot clearing plus source ambient differences; do not fabricate an ambient default or force all retail geometry to be lit.

## Validation and commit

Exact original light-source expansion and immutable snapshot/Recording witness with negatives and source/category order; GCC/Clang cumulative full suites, ABI/link/provider removal, focused sanitizers and ledger before one commit. B is required for lit physical shader acceptance and C for category-issued lit draw.
