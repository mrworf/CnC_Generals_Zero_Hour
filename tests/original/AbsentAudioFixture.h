// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/GameAudio.h"
// Only the original base addAudioEvent's empty/NoSound path is exercised.
// Any request for physical audio, discovery or output explicitly rejects.
struct AbsentAudioFixture final:AudioManager {
#define ZH_ABSENT_AUDIO(result,name,args) result name args override {throw ERROR_BAD_ARG;}
#if defined(_DEBUG) || defined(_INTERNAL)
  ZH_ABSENT_AUDIO(void,audioDebugDisplay,(DebugDisplayInterface*,void*,FILE*))
#endif
  ZH_ABSENT_AUDIO(void,stopAudio,(AudioAffect))
  ZH_ABSENT_AUDIO(void,pauseAudio,(AudioAffect))
  ZH_ABSENT_AUDIO(void,resumeAudio,(AudioAffect))
  ZH_ABSENT_AUDIO(void,pauseAmbient,(Bool))
  ZH_ABSENT_AUDIO(void,killAudioEventImmediately,(AudioHandle))
  ZH_ABSENT_AUDIO(void,nextMusicTrack,())
  ZH_ABSENT_AUDIO(void,prevMusicTrack,())
  ZH_ABSENT_AUDIO(Bool,isMusicPlaying,()const)
  ZH_ABSENT_AUDIO(Bool,hasMusicTrackCompleted,(const AsciiString&,Int)const)
  ZH_ABSENT_AUDIO(AsciiString,getMusicTrackName,()const)
  ZH_ABSENT_AUDIO(void,openDevice,())
  ZH_ABSENT_AUDIO(void,closeDevice,())
  ZH_ABSENT_AUDIO(void*,getDevice,())
  ZH_ABSENT_AUDIO(void,notifyOfAudioCompletion,(UnsignedInt,UnsignedInt))
  ZH_ABSENT_AUDIO(UnsignedInt,getProviderCount,()const)
  ZH_ABSENT_AUDIO(AsciiString,getProviderName,(UnsignedInt)const)
  ZH_ABSENT_AUDIO(UnsignedInt,getProviderIndex,(AsciiString)const)
  ZH_ABSENT_AUDIO(void,selectProvider,(UnsignedInt))
  ZH_ABSENT_AUDIO(void,unselectProvider,())
  ZH_ABSENT_AUDIO(UnsignedInt,getSelectedProvider,()const)
  ZH_ABSENT_AUDIO(void,setSpeakerType,(UnsignedInt))
  ZH_ABSENT_AUDIO(UnsignedInt,getSpeakerType,())
  ZH_ABSENT_AUDIO(UnsignedInt,getNum2DSamples,()const)
  ZH_ABSENT_AUDIO(UnsignedInt,getNum3DSamples,()const)
  ZH_ABSENT_AUDIO(UnsignedInt,getNumStreams,()const)
  ZH_ABSENT_AUDIO(Bool,doesViolateLimit,(AudioEventRTS*)const)
  ZH_ABSENT_AUDIO(Bool,isPlayingLowerPriority,(AudioEventRTS*)const)
  ZH_ABSENT_AUDIO(Bool,isPlayingAlready,(AudioEventRTS*)const)
  ZH_ABSENT_AUDIO(Bool,isObjectPlayingVoice,(UnsignedInt)const)
  ZH_ABSENT_AUDIO(void,adjustVolumeOfPlayingAudio,(AsciiString,Real))
  ZH_ABSENT_AUDIO(void,removePlayingAudio,(AsciiString))
  ZH_ABSENT_AUDIO(void,removeAllDisabledAudio,())
  ZH_ABSENT_AUDIO(Bool,has3DSensitiveStreamsPlaying,()const)
  ZH_ABSENT_AUDIO(void*,getHandleForBink,())
  ZH_ABSENT_AUDIO(void,releaseHandleForBink,())
  ZH_ABSENT_AUDIO(void,friend_forcePlayAudioEventRTS,(const AudioEventRTS*))
  ZH_ABSENT_AUDIO(void,setPreferredProvider,(AsciiString))
  ZH_ABSENT_AUDIO(void,setPreferredSpeaker,(AsciiString))
  ZH_ABSENT_AUDIO(Real,getFileLengthMS,(AsciiString)const)
  ZH_ABSENT_AUDIO(void,closeAnySamplesUsingFile,(const void*))
  ZH_ABSENT_AUDIO(void,setDeviceListenerPosition,())
#undef ZH_ABSENT_AUDIO
};
