# M20 original-lifecycle dependency blocker

Status: `UPSTREAM_REQUIRED` — delivery architecture/dependency replanning

No retail root, retail file, private path, GPU or display session was accessed. No production implementation from the rejected approach is retained.

## Finding

M20 requires preserved `GameMain -> CreateGameEngine -> GameEngine::init/execute`, including original INI/CSF and object/module factory initialization, while explicitly excluding actual simulation and presentation assigned to M21–M23. The original source does not contain such a dependency boundary.

In `GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp`, `GameEngine::init` creates file/data and CSF systems, then audio and function-lexicon providers before `TheModuleFactory` (line 542) and many simulation-data stores before `TheThingFactory` (line 577). It immediately proceeds to `TheGameClient` (line 588), `TheAI` and `TheGameLogic` (lines 599–600). `GameEngine::execute` assumes the resulting `TheGameLogic`, `TheGameClient`, network, radar and audio globals.

The device factory confirms the cross-milestone ownership: `Win32GameEngine.h` returns `W3DModuleFactory`, `W3DThingFactory`, `W3DGameClient`, `W3DGameLogic`, `W3DFunctionLexicon`, `W3DRadar`, `W3DParticleSystemManager` and `MilesAudioManager`. The common `ModuleFactory.cpp` directly includes 202 GameLogic module headers and three GameClient module headers; `W3DModuleFactory.cpp` adds 20 W3D draw-module headers. These are not incidental link helpers: they are the concrete module registry M20 requires to initialize. The active source classification assigns common GameEngine consumers across M20–M25 and assigns W3D factory consumers to M22–M23.

Therefore M20 cannot initialize the original object/module factory state without pulling in the M21 original simulation and M22/M23 presentation/media provider closures. Replacing those providers with empty/success-returning registries would violate SE-003. A wholesale compile-time alternate implementation inside `GameEngine.cpp` would preserve a filename but replace semantics and is equally invalid.

## Bounded checks

- Original initialization ordering inspected at `GameEngine.cpp` lines 437–637.
- Concrete factory ownership inspected in `Win32GameEngine.h` and `Win32GameEngine.cpp`.
- Registry dependency surfaces counted from direct includes: 202 GameLogic, three GameClient and 20 W3D draw-module headers.
- `PreRTS.h` also retains unconditional ATL/Win32 precompiled dependencies; these are locally portable under M19/SE-002, but fixing them does not resolve the semantic provider cycle.
- All rejected production edits were removed; only plans and this evidence remain in the transaction diff.

## Required upstream resolution

Reconcile the M20–M23 contracts and graph using actual source initialization order. A valid packet must choose one of these behavior-preserving directions:

1. Narrow M20 to the last executable original-source boundary before module/object factory construction, move those factory obligations and full `GameEngine::init/execute` runtime acceptance to the milestones that compile their real M21–M23 providers, and make the later integrated milestone depend on all of them; or
2. Reorder/combine the original simulation, W3D/GameClient and media provider closure ahead of the milestone that claims full `GameEngine::init/execute` acceptance.

The new contract must state which original initialization stages may be deliberately deferred and must forbid acceptance from reduced classes, parallel globals, empty registries or source-file identity alone.

## Resume point

Resume M20 only from a readiness-approved replacement packet and a new governing plan. The first implementation slice should port the original headers/PreRTS and factory interfaces needed by the newly defined executable boundary, then compile actual provider implementations in source dependency order. Do not resume the discarded synthetic lifecycle design.
