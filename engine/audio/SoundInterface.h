#if !defined(SOUNDINTERFACE_H)

#include "fmod_studio.h"
#include "fmod_studio.hpp"
#include "fmod_errors.h"

//NOTE(Ray):Depends on FMOD

struct SoundClip
{
    FMOD_STUDIO_PLAYBACK_STATE playback_state;
    FMOD::Studio::EventDescription* desc;
    FMOD::Studio::EventInstance* state;    
};

struct SoundBuffer
{
    
};

struct SoundDevice
{
    
};

struct SoundListener
{
    FMOD_3D_ATTRIBUTES state;
};

struct SoundBus
{
    FMOD::Studio::Bus* state;
};

struct SoundBank
{
    FMOD::Studio::Bank* state;
};

struct SoundEventBuffer
{
    YoyoVector sound_events;
};

struct SoundBankBuffer
{
    YoyoVector sound_banks;
};

struct SoundBusBuffer
{
    YoyoVector sound_buses;
};

namespace SoundCode
{
//    static FMOD::Studio::System* sound_system = NULL;
    static void Init();
    //GOOD for 2d sound effects
    static void SetDefaultListener();
    static void ContinousPlay(SoundClip* sample);
    static void Stop(SoundClip* sample);
    static void Update();
}

namespace SoundAssetCode
{
    //NOTE(Ray):This is here for convience.
//    static SoundBankBuffer bank_buffer;
    //static SoundBusBuffer bus_buffer;     
    static bool Init();
    static void CreateSoundBank(SoundBankBuffer* buffer,char* bank_name);
    static void CreateSoundBank(char* bank_name);
    static void LoadBankSampleData(SoundBank* bank);
    static void LoadBankSampleData();
    static void GetBus(SoundBusBuffer* buffer,char* bus_name);
    static void GetBus(char* bus_name);
    static bool GetEvent(char* event_name,SoundClip* clip);
}
#define SOUNDINTERFACE_H
#endif
