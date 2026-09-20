# Legacy manifest inventory

This M0 inventory classifies the VC6 Zero Hour manifests. Candidate entries are migration inputs assigned to a native target; they are not claimed to compile until their owning port milestones complete. `cmake/LegacySourceInventory.cmake` contains every explicit source record.

## Source summary

| Classification | Records |
|---|---:|
| candidate | 1889 |
| excluded | 1085 |
| unavailable | 381 |

| Native target | Candidate records |
|---|---:|
| `zh_compression` | 18 |
| `zh_game_device` | 168 |
| `zh_game_engine` | 1074 |
| `zh_main` | 3 |
| `zh_w3d` | 243 |
| `zh_wwshade` | 67 |
| `zh_wwsupport` | 316 |

## Project classification

| Manifest/project | Classification | Native target or reason |
|---|---|---|
| `GeneralsMD/Code/GameEngine/GameEngine.dsp` | candidate | zh_game_engine |
| `GeneralsMD/Code/GameEngineDevice/GameEngineDevice.dsp` | candidate | zh_game_device |
| `GeneralsMD/Code/Libraries/Source/Benchmark/Benchmark.dsp` | excluded | unused legacy benchmark target |
| `GeneralsMD/Code/Libraries/Source/Compression/Compression.dsp` | candidate | zh_compression |
| `GeneralsMD/Code/Libraries/Source/EABrowserDispatch/EABrowserDispatch.dsp` | excluded | obsolete embedded browser dependency |
| `GeneralsMD/Code/Libraries/Source/WPAudio/WPAudio.dsp` | excluded | obsolete Windows audio implementation replaced at M12 |
| `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/ww3d2.dsp` | candidate | zh_w3d |
| `GeneralsMD/Code/Libraries/Source/WWVegas/WWAudio/WWAudio.dsp` | excluded | obsolete Windows audio implementation replaced at M12 |
| `GeneralsMD/Code/Libraries/Source/WWVegas/WWDebug/wwdebug.dsp` | candidate | zh_wwsupport |
| `GeneralsMD/Code/Libraries/Source/WWVegas/WWDownload/WWDownload.dsp` | excluded | retired download service |
| `GeneralsMD/Code/Libraries/Source/WWVegas/WWLib/wwlib.dsp` | candidate | zh_wwsupport |
| `GeneralsMD/Code/Libraries/Source/WWVegas/WWMath/wwmath.dsp` | candidate | zh_wwsupport |
| `GeneralsMD/Code/Libraries/Source/WWVegas/WWSaveLoad/wwsaveload.dsp` | candidate | zh_wwsupport |
| `GeneralsMD/Code/Libraries/Source/WWVegas/Wwutil/wwutil.dsp` | candidate | zh_wwsupport |
| `GeneralsMD/Code/Libraries/Source/WWVegas/wwshade/wwshade.dsp` | candidate | zh_wwshade |
| `GeneralsMD/Code/Libraries/Source/debug/debug.dsp` | excluded | legacy developer-only diagnostic harness |
| `GeneralsMD/Code/Libraries/Source/debug/debug_dlg/debug_dlg.dsp` | excluded | legacy developer-only diagnostic harness |
| `GeneralsMD/Code/Libraries/Source/debug/netserv/netserv.dsp` | excluded | legacy developer-only diagnostic harness |
| `GeneralsMD/Code/Libraries/Source/debug/test1/test1.dsp` | excluded | legacy developer-only diagnostic harness |
| `GeneralsMD/Code/Libraries/Source/debug/test2/test2.dsp` | excluded | legacy developer-only diagnostic harness |
| `GeneralsMD/Code/Libraries/Source/debug/test3/test3.dsp` | excluded | legacy developer-only diagnostic harness |
| `GeneralsMD/Code/Libraries/Source/debug/test4/test4.dsp` | excluded | legacy developer-only diagnostic harness |
| `GeneralsMD/Code/Libraries/Source/debug/test5/test5.dsp` | excluded | legacy developer-only diagnostic harness |
| `GeneralsMD/Code/Libraries/Source/debug/test6/test6.dsp` | excluded | legacy developer-only diagnostic harness |
| `GeneralsMD/Code/Libraries/Source/profile/profile.dsp` | excluded | legacy developer-only profiling harness |
| `GeneralsMD/Code/Libraries/Source/profile/test1/test1.dsp` | excluded | legacy developer-only profiling harness |
| `GeneralsMD/Code/RTS.dsp` | candidate | zh_main |
| `GeneralsMD/Code/Tools/Autorun/Autorun.dsp` | excluded | authoring or maintenance tool outside the game runtime |
| `GeneralsMD/Code/Tools/Babylon/Babylon.dsp` | excluded | authoring or maintenance tool outside the game runtime |
| `GeneralsMD/Code/Tools/CRCDiff/CRCDiff.dsp` | excluded | authoring or maintenance tool outside the game runtime |
| `GeneralsMD/Code/Tools/Compress/Compress.dsp` | excluded | authoring or maintenance tool outside the game runtime |
| `GeneralsMD/Code/Tools/DebugWindow/DebugWindow.dsp` | excluded | authoring or maintenance tool outside the game runtime |
| `GeneralsMD/Code/Tools/GUIEdit/GUIEdit.dsp` | excluded | authoring or maintenance tool outside the game runtime |
| `GeneralsMD/Code/Tools/ImagePacker/ImagePacker.dsp` | excluded | authoring or maintenance tool outside the game runtime |
| `GeneralsMD/Code/Tools/Launcher/DatGen/DatGen.dsp` | excluded | authoring or maintenance tool outside the game runtime |
| `GeneralsMD/Code/Tools/Launcher/launcher.dsp` | excluded | authoring or maintenance tool outside the game runtime |
| `GeneralsMD/Code/Tools/MapCacheBuilder/MapCacheBuilder.dsp` | excluded | authoring or maintenance tool outside the game runtime |
| `GeneralsMD/Code/Tools/PATCHGET/patchgrabber.dsp` | excluded | authoring or maintenance tool outside the game runtime |
| `GeneralsMD/Code/Tools/ParticleEditor/ParticleEditor.dsp` | excluded | authoring or maintenance tool outside the game runtime |
| `GeneralsMD/Code/Tools/WW3D/max2w3d/max2w3d.dsp` | excluded | authoring or maintenance tool outside the game runtime |
| `GeneralsMD/Code/Tools/WW3D/pluglib/pluglib.dsp` | excluded | authoring or maintenance tool outside the game runtime |
| `GeneralsMD/Code/Tools/WorldBuilder/WorldBuilder.dsp` | excluded | authoring or maintenance tool outside the game runtime |
| `GeneralsMD/Code/Tools/assetcull/assetcull.dsp` | excluded | authoring or maintenance tool outside the game runtime |
| `GeneralsMD/Code/Tools/buildVersionUpdate/buildVersionUpdate.dsp` | excluded | authoring or maintenance tool outside the game runtime |
| `GeneralsMD/Code/Tools/mangler/mangler.dsp` | excluded | authoring or maintenance tool outside the game runtime |
| `GeneralsMD/Code/Tools/mangler/manglertest.dsp` | excluded | authoring or maintenance tool outside the game runtime |
| `GeneralsMD/Code/Tools/textureCompress/textureCompress.dsp` | excluded | authoring or maintenance tool outside the game runtime |
| `GeneralsMD/Code/Tools/timingTest/timingTest.dsp` | excluded | authoring or maintenance tool outside the game runtime |
| `GeneralsMD/Code/Tools/versionUpdate/versionUpdate.dsp` | excluded | authoring or maintenance tool outside the game runtime |
| `GeneralsMD/Code/Tools/wolSetup/wolSetup.dsp` | excluded | authoring or maintenance tool outside the game runtime |
| `RTS.dsw::Benchmark` | solution-present | GeneralsMD/Code/Libraries/Source/Benchmark/Benchmark.dsp |
| `RTS.dsw::CRCDiff` | solution-present | GeneralsMD/Code/Tools/CRCDiff/CRCDiff.dsp |
| `RTS.dsw::Compression` | solution-present | GeneralsMD/Code/Libraries/Source/Compression/Compression.dsp |
| `RTS.dsw::DatGen` | solution-present | GeneralsMD/Code/Tools/Launcher/DatGen/DatGen.dsp |
| `RTS.dsw::DebugWindow` | solution-present | GeneralsMD/Code/Tools/DebugWindow/DebugWindow.dsp |
| `RTS.dsw::EABrowserDispatch` | solution-present | GeneralsMD/Code/Libraries/Source/EABrowserDispatch/EABrowserDispatch.dsp |
| `RTS.dsw::GUIEdit` | solution-present | GeneralsMD/Code/Tools/GUIEdit/GUIEdit.dsp |
| `RTS.dsw::GameEngine` | solution-present | GeneralsMD/Code/GameEngine/GameEngine.dsp |
| `RTS.dsw::GameEngineDevice` | solution-present | GeneralsMD/Code/GameEngineDevice/GameEngineDevice.dsp |
| `RTS.dsw::GameSpyHTTP` | solution-unavailable | GeneralsMD/Code/Libraries/Source/GameSpy/GameSpy/ghttp/GameSpyHTTP.dsp is absent; project is unavailable/out of scope |
| `RTS.dsw::GameSpyPatching` | solution-unavailable | GeneralsMD/Code/Libraries/Source/GameSpy/GameSpy/pt/GameSpyPatching.dsp is absent; project is unavailable/out of scope |
| `RTS.dsw::GameSpyPeer` | solution-unavailable | GeneralsMD/Code/Libraries/Source/GameSpy/GameSpy/peer/GameSpyPeer.dsp is absent; project is unavailable/out of scope |
| `RTS.dsw::GameSpyPresence` | solution-unavailable | GeneralsMD/Code/Libraries/Source/GameSpy/GameSpy/gp/GameSpyPresence.dsp is absent; project is unavailable/out of scope |
| `RTS.dsw::GameSpyStats` | solution-unavailable | GeneralsMD/Code/Libraries/Source/GameSpy/GameSpy/gstats/GameSpyStats.dsp is absent; project is unavailable/out of scope |
| `RTS.dsw::ImagePacker` | solution-present | GeneralsMD/Code/Tools/ImagePacker/ImagePacker.dsp |
| `RTS.dsw::MapCacheBuilder` | solution-present | GeneralsMD/Code/Tools/MapCacheBuilder/MapCacheBuilder.dsp |
| `RTS.dsw::ParticleEditor` | solution-present | GeneralsMD/Code/Tools/ParticleEditor/ParticleEditor.dsp |
| `RTS.dsw::RTS` | solution-present | GeneralsMD/Code/RTS.dsp |
| `RTS.dsw::WWDownload` | solution-present | GeneralsMD/Code/Libraries/Source/WWVegas/WWDownload/WWDownload.dsp |
| `RTS.dsw::WorldBuilder` | solution-present | GeneralsMD/Code/Tools/WorldBuilder/WorldBuilder.dsp |
| `RTS.dsw::assetcull` | solution-present | GeneralsMD/Code/Tools/assetcull/assetcull.dsp |
| `RTS.dsw::buildVersionUpdate` | solution-present | GeneralsMD/Code/Tools/buildVersionUpdate/buildVersionUpdate.dsp |
| `RTS.dsw::debug` | solution-present | GeneralsMD/Code/Libraries/Source/debug/debug.dsp |
| `RTS.dsw::launcher` | solution-present | GeneralsMD/Code/Tools/Launcher/launcher.dsp |
| `RTS.dsw::patchgrabber` | solution-present | GeneralsMD/Code/Tools/PATCHGET/patchgrabber.dsp |
| `RTS.dsw::profile` | solution-present | GeneralsMD/Code/Libraries/Source/profile/profile.dsp |
| `RTS.dsw::versionUpdate` | solution-present | GeneralsMD/Code/Tools/versionUpdate/versionUpdate.dsp |
| `RTS.dsw::ww3d2` | solution-present | GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/ww3d2.dsp |
| `RTS.dsw::wwdebug` | solution-present | GeneralsMD/Code/Libraries/Source/WWVegas/WWDebug/wwdebug.dsp |
| `RTS.dsw::wwlib` | solution-present | GeneralsMD/Code/Libraries/Source/WWVegas/WWLib/wwlib.dsp |
| `RTS.dsw::wwmath` | solution-present | GeneralsMD/Code/Libraries/Source/WWVegas/WWMath/wwmath.dsp |
| `RTS.dsw::wwsaveload` | solution-present | GeneralsMD/Code/Libraries/Source/WWVegas/WWSaveLoad/wwsaveload.dsp |
| `RTS.dsw::wwshade` | solution-present | GeneralsMD/Code/Libraries/Source/WWVegas/wwshade/wwshade.dsp |
| `RTS.dsw::wwutil` | solution-present | GeneralsMD/Code/Libraries/Source/WWVegas/Wwutil/wwutil.dsp |

## Audit

Run `python3 tools/legacy_manifest_inventory.py --check`. Regenerate after an intentional manifest change with `python3 tools/legacy_manifest_inventory.py --generate`.
