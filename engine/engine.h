//NOTE(Ray):Here will be all the declarations for the engine.

#include "yoyo/RendererInclude.h"

#include "input.h"

struct PlatformState
{
    //NOTE(Ray):Move these.
//    StringState string_state;
//    Renderer renderer;
//    Memory memory;
//    Audio audio;    
//    Info info;

    //NOTE(Ray):Keep these here
//    Time time;
    Input input;
//    Thread thread;
//    Window window;

    bool is_running = false;
    bool is_init = false;
};

namespace Engine
{
    void Init(float2 window_dim);
    void Update();
}


    
