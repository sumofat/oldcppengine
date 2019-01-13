
//NOTE(Ray):Following this model we will maintain a seperate implementation.
//We want the game to link to the engine as a dll or static lib.

#include "engine.h"


#include "input/input.cpp"


#ifdef ENGINEIMPL

#include "graphics/camera.cpp"
#include "graphics/deffered/deffered.cpp"

extern "C" void gameInit();
extern "C" void gameUpdate();


 
namespace Engine
{
    YoyoVector test_vector;
    
    bool engine_log = true;
    PlatformState ps = {};
    //NOTE(Ray):Here we init all the engine memory and ints "Subsystems"
    void Init(float2 window_dim)
    {
        PlatformOutput(engine_log,"Engine Init Begin\n");
        ps.window.dim = window_dim;
        ps.window.is_full_screen_mode = false;
        //Init Input
        //TODO(Ray):Mouse buttons gamepads keyboard
        EngineInput::PullMouseState(&ps);
        
        //Init Renderer
        Camera::Init();
        DefferedRenderer::Init(&Camera::main, &ps);
        //Init Audio
        //Init Physics

        //Init TIme
        
        //Init Multithreading

        
        test_vector = YoyoInitVector(1,float2,false);
        test_vector.resize_ratio = 0.5f;
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
        PlatformOutput(engine_log,"Engine Update Begin\n");
        
        EngineInput::PullMouseState(&ps);

        gameUpdate();

        
        PlatformOutput(engine_log,"Test vec size %d \n",test_vector.max_size);
        float2 newf2 = float2(100,100);
        YoyoStretchPushBack(&test_vector,newf2);
        //Update Graphics
        DefferedRenderer::ExecutePasses();
        //Update Audio
        //Update Physics
        PlatformOutput(engine_log,"Engine Update Complete\n");
    }
};

#endif
