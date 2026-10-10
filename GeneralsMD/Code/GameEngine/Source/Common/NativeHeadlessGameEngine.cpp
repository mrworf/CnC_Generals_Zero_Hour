// SPDX-License-Identifier: GPL-3.0-or-later
#include "PreRTS.h"
#include "Common/NativeHeadlessGameEngine.h"
#include "Common/FileSystem.h"
#include "Common/ModuleFactory.h"
#include "Common/ThingFactory.h"
#include "Common/FunctionLexicon.h"
#include "Common/Radar.h"
#include "Common/GameAudio.h"
#include "GameLogic/GameLogic.h"
#include "GameClient/NativeHeadlessGameClient.h"
#include "GameClient/ParticleSys.h"
#include <memory>
#include <utility>

namespace {
class NativeHeadlessRadar final : public Radar {
  void draw(Int,Int,Int,Int) override { throw ERROR_BAD_ARG; }
  void clearShroud() override { throw ERROR_BAD_ARG; }
  void setShroudLevel(Int,Int,CellShroudStatus) override { throw ERROR_BAD_ARG; }
};

class NativeHeadlessParticles final : public ParticleSystemManager {
  Int getOnScreenParticleCount() override { return 0; }
  void doParticles(RenderInfoClass&) override { throw ERROR_BAD_ARG; }
  void queueParticleRender() override { throw ERROR_BAD_ARG; }
};

// Startup parses original audio metadata. Physical output is absent for this
// bounded headless runner; requests outside that contract fail explicitly.
class NativeHeadlessAudio final : public AudioManager {
  AsciiString m_preferredProvider;
  AsciiString m_preferredSpeaker;
#define ZH_NO_OUTPUT(result,name,args) result name args override { throw ERROR_BAD_ARG; }
#if defined(_DEBUG) || defined(_INTERNAL)
  ZH_NO_OUTPUT(void,audioDebugDisplay,(DebugDisplayInterface*,void*,FILE*))
#endif
  ZH_NO_OUTPUT(void,stopAudio,(AudioAffect))
  ZH_NO_OUTPUT(void,pauseAudio,(AudioAffect))
  ZH_NO_OUTPUT(void,resumeAudio,(AudioAffect))
  ZH_NO_OUTPUT(void,pauseAmbient,(Bool))
  ZH_NO_OUTPUT(void,killAudioEventImmediately,(AudioHandle))
  ZH_NO_OUTPUT(void,nextMusicTrack,())
  ZH_NO_OUTPUT(void,prevMusicTrack,())
  ZH_NO_OUTPUT(Bool,isMusicPlaying,()const)
  ZH_NO_OUTPUT(Bool,hasMusicTrackCompleted,(const AsciiString&,Int)const)
  ZH_NO_OUTPUT(AsciiString,getMusicTrackName,()const)
  ZH_NO_OUTPUT(void,openDevice,())
  ZH_NO_OUTPUT(void,closeDevice,())
  ZH_NO_OUTPUT(void*,getDevice,())
  ZH_NO_OUTPUT(void,notifyOfAudioCompletion,(UnsignedInt,UnsignedInt))
  ZH_NO_OUTPUT(UnsignedInt,getProviderCount,()const)
  ZH_NO_OUTPUT(AsciiString,getProviderName,(UnsignedInt)const)
  ZH_NO_OUTPUT(UnsignedInt,getProviderIndex,(AsciiString)const)
  ZH_NO_OUTPUT(void,selectProvider,(UnsignedInt))
  ZH_NO_OUTPUT(void,unselectProvider,())
  ZH_NO_OUTPUT(UnsignedInt,getSelectedProvider,()const)
  ZH_NO_OUTPUT(void,setSpeakerType,(UnsignedInt))
  ZH_NO_OUTPUT(UnsignedInt,getSpeakerType,())
  ZH_NO_OUTPUT(UnsignedInt,getNum2DSamples,()const)
  ZH_NO_OUTPUT(UnsignedInt,getNum3DSamples,()const)
  ZH_NO_OUTPUT(UnsignedInt,getNumStreams,()const)
  ZH_NO_OUTPUT(Bool,doesViolateLimit,(AudioEventRTS*)const)
  ZH_NO_OUTPUT(Bool,isPlayingLowerPriority,(AudioEventRTS*)const)
  ZH_NO_OUTPUT(Bool,isPlayingAlready,(AudioEventRTS*)const)
  ZH_NO_OUTPUT(Bool,isObjectPlayingVoice,(UnsignedInt)const)
  ZH_NO_OUTPUT(void,adjustVolumeOfPlayingAudio,(AsciiString,Real))
  ZH_NO_OUTPUT(void,removePlayingAudio,(AsciiString))
  ZH_NO_OUTPUT(void,removeAllDisabledAudio,())
  ZH_NO_OUTPUT(Bool,has3DSensitiveStreamsPlaying,()const)
  ZH_NO_OUTPUT(void*,getHandleForBink,())
  ZH_NO_OUTPUT(void,releaseHandleForBink,())
  ZH_NO_OUTPUT(void,friend_forcePlayAudioEventRTS,(const AudioEventRTS*))
  void setPreferredProvider(AsciiString value) override { m_preferredProvider = std::move(value); }
  void setPreferredSpeaker(AsciiString value) override { m_preferredSpeaker = std::move(value); }
  ZH_NO_OUTPUT(Real,getFileLengthMS,(AsciiString)const)
  ZH_NO_OUTPUT(void,closeAnySamplesUsingFile,(const void*))
  ZH_NO_OUTPUT(void,setDeviceListenerPosition,())
#undef ZH_NO_OUTPUT
};
}

NativeHeadlessGameEngine::NativeHeadlessGameEngine(NativeHeadlessConfiguration configuration)
  : m_configuration(std::move(configuration)) {
  if (m_configuration.dataRoots.empty() || m_configuration.userPaths.data.empty() ||
      m_configuration.userPaths.cache.empty()) throw ERROR_BAD_ARG;
}
FileSystem* NativeHeadlessGameEngine::createFileSystem() {
  auto files=std::make_unique<FileSystem>();
  files->mountReadOnly(m_configuration.dataRoots);
  return files.release();
}
NativeUserStorage* NativeHeadlessGameEngine::createUserStorage() {
  auto storage=std::make_unique<NativeUserStorage>(m_configuration.userPaths,*TheFileSystem);
  TheFileSystem->attachUserStorage(storage.get());
  return storage.release();
}
GameLogic* NativeHeadlessGameEngine::createGameLogic() { return new GameLogic; }
GameClient* NativeHeadlessGameEngine::createGameClient() { return new NativeHeadlessGameClient; }
ModuleFactory* NativeHeadlessGameEngine::createModuleFactory() { return new ModuleFactory; }
ThingFactory* NativeHeadlessGameEngine::createThingFactory() { return new ThingFactory; }
FunctionLexicon* NativeHeadlessGameEngine::createFunctionLexicon() { return new FunctionLexicon(std::span<const NativeFunctionTable>{}); }
Radar* NativeHeadlessGameEngine::createRadar() { return new NativeHeadlessRadar; }
WebBrowser* NativeHeadlessGameEngine::createWebBrowser() { return nullptr; }
ParticleSystemManager* NativeHeadlessGameEngine::createParticleSystemManager() { return new NativeHeadlessParticles; }
AudioManager* NativeHeadlessGameEngine::createAudioManager() { return new NativeHeadlessAudio; }
