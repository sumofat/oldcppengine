
#if ENGINEIMPL
//NOTE(Ray):We could thinly wrap FMOD but not going to do it since
//probably wont build own sound system for awhile if ever.  Which means we will hold an internal pointer
//to the system.  Treated similar to a mix between Renderer and nativeap on graphics side.
//This is just a convience layer.
namespace SoundCode
{
    FMOD::Studio::System* sound_system = NULL;
    void Init()
    {
        FMOD_RESULT result;
        result = FMOD::Studio::System::create(&sound_system); // Create the Studio System object.
        if (result != FMOD_OK)
        {
            printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
            Assert(false);
        }
        // Initialize FMOD Studio, which will also initialize FMOD Low Level
        result = sound_system->initialize(512, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, 0);
        if (result != FMOD_OK)
        {
            printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
            Assert(false);
        }
        result = sound_system->setNumListeners(1);
        if (result != FMOD_OK)
        {
            printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
            Assert(false);        
        }
    }

    //GOOD for 2d sound effects
    void SetDefaultListener()
    {
     	FMOD_3D_ATTRIBUTES attr = { { 0 } };
        attr.forward.z = 1.0f;
        attr.up.y = 1.0f;
        attr.position.z = 0.0f;
        attr.position.x = 0.0f;
        attr.position.y = 0.0f;
        sound_system->setListenerAttributes(0, &attr);   
    }
    
    void ContinousPlay(SoundClip* sample)
    {
        FMOD_STUDIO_PLAYBACK_STATE state;
        FMOD_RESULT r = sample->state->getPlaybackState(&state);
        Assert(r == FMOD_OK);
        if(state != FMOD_STUDIO_PLAYBACK_PLAYING)
        {
            sample->state->start();
        }
    }

    void Stop(SoundClip* sample)
    {
        //sample->state->stop();
    }

    void Update()
    {
		FMOD_RESULT r = sound_system->update();
        Assert(r == FMOD_OK);
    }
}

namespace SoundAssetCode
{

    //NOTE(Ray):This is here for convience.
    SoundBankBuffer bank_buffer;
    SoundBusBuffer bus_buffer;     
    bool Init()
    {
        //NOTE(Ray):Was thinking of adding a convinence soundbankbuffer here for now leaving up to a layer up.
        bank_buffer.sound_banks = YoyoInitVector(100,SoundBank,false);
        bus_buffer.sound_buses = YoyoInitVector(100,SoundBus,false);
        return true;
    };
    
    void CreateSoundBank(SoundBankBuffer* buffer,char* bank_name)
    {
        SoundBank new_bank = {};
        FMOD::Studio::Bank* result = NULL;
        FMOD_RESULT r =   SoundCode::sound_system->loadBankFile(bank_name, FMOD_STUDIO_LOAD_BANK_NORMAL, &result);
        new_bank.state = result;
        Assert(r == FMOD_OK);
        YoyoPushBack(&buffer->sound_banks,new_bank);
    }

    void CreateSoundBank(char* bank_name)
    {
        CreateSoundBank(&bank_buffer,bank_name);
    }

    void LoadBankSampleData(SoundBank* bank)
    {
        bank->state->loadSampleData();
    }

    void LoadBankSampleData()
    {
        SoundBank* bank = {};
        while(bank = YoyoIterateVector(&bank_buffer.sound_banks,SoundBank))
        {
            bank->state->loadSampleData();
        }
    }
    
    void GetBus(SoundBusBuffer* buffer,char* bus_name)
    {
        SoundBus new_bus = {};
        FMOD::Studio::Bus* result;
        FMOD_RESULT r = SoundCode::sound_system->getBus(bus_name, &result);
        new_bus.state = result;
        YoyoPushBack(&buffer->sound_buses,new_bus);
        Assert(r == FMOD_OK);
    }

    void GetBus(char* bus_name)
    {
        GetBus(&bus_buffer,bus_name);
    }

    bool GetEvent(char* event_name,SoundClip* clip)
    {
        //SoundEvent new_event = {};
        FMOD::Studio::EventDescription* result;
        FMOD_RESULT r = SoundCode::sound_system->getEvent(event_name, &result);
        Assert(r == FMOD_OK);
        r = result->createInstance(&clip->state);
        clip->desc = result;
        Assert(r == FMOD_OK);
        return true;
    }
}
#endif

