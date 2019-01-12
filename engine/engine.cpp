//NOTE(Ray):Following this model we will maintain a seperate implementation.
//We want the game to link to the engine as a dll or static lib.

#include "engine.h"

#ifdef ENGINEIMPL
bool engine_log = true;

extern "C" void gameInit();
extern "C" void gameUpdate();

namespace Engine
{
    //NOTE(Ray):Here we init all the engine memory and ints "Subsystems"
    void Init(float2 window_dim)
    {
        PlatformOutput(engine_log,"Engine Init Begin\n");
        //Init Graphics Init Audio Init Physics Init Input Init TIme
        //Init Multithreading

//TODO(Ray):
        //Init Network
        //Init etc..
        PlatformOutput(engine_log,"Engine Init Complete\n");

        gameInit();
    }

    //TODO(Ray):Fixed update.
    
    //NOTE(Ray):Here we do the updating of everything engine related.
    void Update()
    {
        gameUpdate();
        
        PlatformOutput(engine_log,"Engine Update Begine\n");
        //Update Graphics
        //Update Audio
        //Update Physics
        PlatformOutput(engine_log,"Engine Update Complete\n");
    }
};

#endif
