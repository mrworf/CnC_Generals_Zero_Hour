// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/GameEngine.h"
#include "Common/NativeUserStorage.h"
#include <string>
#include <vector>

// Owns the explicit roots for one original-engine lifetime. These paths are
// never published in shared diagnostics or written into the supplied mounts.
struct NativeHeadlessConfiguration {
  std::vector<std::string> dataRoots;
  NativeUserPaths userPaths;
};

class NativeHeadlessGameEngine final : public GameEngine {
public:
  explicit NativeHeadlessGameEngine(NativeHeadlessConfiguration configuration);
private:
  FileSystem* createFileSystem() override;
  NativeUserStorage* createUserStorage() override;
  GameLogic* createGameLogic() override;
  GameClient* createGameClient() override;
  ModuleFactory* createModuleFactory() override;
  ThingFactory* createThingFactory() override;
  FunctionLexicon* createFunctionLexicon() override;
  Radar* createRadar() override;
  WebBrowser* createWebBrowser() override;
  ParticleSystemManager* createParticleSystemManager() override;
  AudioManager* createAudioManager() override;
  NativeHeadlessConfiguration m_configuration;
};
