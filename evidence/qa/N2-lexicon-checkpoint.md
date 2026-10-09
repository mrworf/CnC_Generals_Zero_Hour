# N2 actual FunctionLexicon checkpoint — not milestone acceptance

Parent45b61c12; same plan01/slice03, N2 remains in progress. No agents/vendor/
retail/recovery edits. Default construction captures all seven original GUI
tables; no callback is deleted or stubbed. Explicit native/headless metadata is
a caller-selected owner contract. Actual init/reset/update/typed lookup executes,
not just a registry mock.

## Evidence and identities

All configured targets build normal GCC/GCCsan/Clangsan. Related67/67 PASS
16.23s/65.25s/62.24s on the normal host, sanitizer/leak checks intact. Related
source canonical563 `6c230a22b654fac1d5e7c10191abc4b820c3ef3984ae74795c1849111cdcec0c`;
toolchain570 `58e6989018e94873c65c4671bd022a706b086482b4a2fe6237e5e0c9412ab7a7`.
The only subsequent executable change strengthens the negative fixture: malformed
candidate entries differ from prior accepted table backing. It does not assert
rollback of caller mutations to already-borrowed backing.

Final six focused families PASS all3 0.23s/1.33s/0.96s. Final canonical563 SHA256
`6944a8345a61aa61246ce149590bfdc8dcbe84807f4bf2a221261d98ffbb6871`;
toolchain-inclusive570 SHA256
`f2be068e779cf985d6eaad2cf3b7c75edab676b1d896aec0f385bbe159a2446b`.
All configured targets rebuild successfully with the final fixture. Existing
cumulative path/content identity convention since9e5c0e4f is unchanged.

Five executable owner families plus source-table lock cover captured descriptors,
borrowed backing, typed dispatch, device-first lookup, subset reset, empty
headless metadata, missing name service, malformed sentinel/prototype/name/index/
duplicate/oversized and unpaired-device rejection. Three complete owner/name/
publication lifetimes and all fault/retry pairs prove warm terminal1, cold
terminal9, device terminal9 all3. Cold residuals equal accepted monotonic name
ownership; complete owner retirement is exact. Original table/declaration SHA256
`c5070a715576d4bd607502f9bbf987efd98bd8c0f81b6a08e683db5af53b4d3d`.

```sh
cmake --build build/original-core-<variant> -j4 -- -k0
ctest --test-dir build/original-core-<variant> --output-on-failure -R 'original_(lexicon|function_registry|startup|service|configuration|message|runtime|core)'
ctest --test-dir build/original-core-<variant> --output-on-failure -R '^original_lexicon_'
```

Temporary logs `/tmp/zh-lexicon-<variant>-{build,reviewed-build,final-build,focused-tests,related-tests,final-tests,root-link,default-link}.log`.
Initial compile failed two newly written enum conversions (array cardinality and
fixture name-key input), corrected before execution. Oversized generated patch
output and later invalid documentation patch contexts were rejected without
mutation, then corrected. These attempts are not acceptance.

## Diagnostic scopes and remaining work

Startup root still fails6 normal GCC,8 GCCsan,15 Clangsan; never executed. Default
GUI still fails212/214/222; its separate diagnostic is EXCLUDE_FROM_ALL and never
registered/run as CTest. Genuine uncompiled GUI providers are not passed features.

Normal remaining: HideDiplomacy, PopulateInGameDiplomacyPopup, ResetDiplomacy,
positionStartSpots, testMinimumRequirements, updateMapStartSpots. GCCsan also needs
getQR2HostingStatus and isThreadHosting. Clangsan also needs CreateIMEManagerInterface,
GetBriefingTextList, OSDisplayWarningBox, ToggleDiplomacy, UpdateDiplomacyBriefingText,
doSkyBoxSet, oversizeTheTerrain and both hosting functions. Their full lifecycle/
semantic admission remains pending; no no-ops or fabricated benchmark claims.

Configured suite215 full matrix is not run. Original process/scenario/default
statistics sampler and physical GUI remain N2/later-milestone gates.
