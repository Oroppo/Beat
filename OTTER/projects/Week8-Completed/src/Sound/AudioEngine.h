#pragma once
#ifndef _AUDIO_ENGINE_H_
#define _AUDIO_ENGINE_H_

#include "fmod_studio.hpp"
#include "fmod.hpp"
#include <string>
#include <map>
#include <vector>
#include <math.h>
#include <iostream>
#include <GLM/glm.hpp>

#endif

using namespace std;

struct Implementation {
    Implementation();
    ~Implementation();

    void Update();

    FMOD::Studio::System* mpStudioSystem;
    FMOD::System* mpSystem;

    int mnNextChannelId;

    typedef map<string, FMOD::Sound*> SoundMap;
    typedef map<int, FMOD::Channel*> ChannelMap;
    typedef map<string, FMOD::Studio::EventInstance*> EventMap;
    typedef map<string, FMOD::Studio::Bank*> BankMap;

    BankMap mBanks;
    EventMap mEvents;
    SoundMap mSounds;
    ChannelMap mChannels;
};

class AudioEngine
{


public:
    static void Init();
    static void Update();
    static void Shutdown();
    static int ErrorCheck(FMOD_RESULT result);

    void LoadBank(const string& strBankName, FMOD_STUDIO_LOAD_BANK_FLAGS flags);
    void LoadEvent(const string& strEventName);
    void LoadSound(const string& strSoundName, bool b3d = true, bool bLooping = false, bool bStream = false);
    void UnLoadSound(const string& strSoundName);
    void Set3dListenerAndOrientation(const glm::vec3& vPos = glm::vec3{ 0, 0, 0 }, float fVolumedB = 0.0f);
    int PlaySound(const string& strSoundName, const glm::vec3& vPos = glm::vec3( 0, 0, 0 ), float fVolumedB = 0.0f);
    void PlayEvent(const string& strEventName);
    void StopChannel(int nChannelId);
    void StopEvent(const string& strEventName, bool bImmediate = false);
    void GetEventParameter(const string& strEventName, const string& strEventParameter, float* parameter);
    void SetEventParameter(const string& strEventName, const string& strParameterName, float fValue);
    void StopAllChannels();
    void SetChannel3dPosition(int nChannelId, const glm::vec3& vPosition);
    void SetChannelVolume(int nChannelId, float fVolumedB);
    bool IsPlaying(int nChannelId) const;
    bool IsEventPlaying(const string& strEventName) const;
    float dbToVolume(float db);
    float VolumeTodb(float volume);
    FMOD_VECTOR VectorToFmod(const glm::vec3& vPosition);

};

