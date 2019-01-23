
//NOTE(Ray):Following this model we will maintain a seperate implementation.
//We want the game to link to the engine as a dll or static lib.

#include "engine.h"
#include "input/input.cpp"

#ifdef ENGINEIMPL

#include "graphics/camera.cpp"
#include "graphics/deffered/deffered.cpp"
#include "external/imgui/imgui.cpp"
#include "external/imgui/imgui_draw.cpp"
#include "external/imgui/imgui_widgets.cpp"

//TODO(Ray):NOTE(Ray):Due to the use of static in the function calls here we need to add a
//a-fno_threadsafe_static call to the engine for reasons unknown revisit later.

#include "external/imgui/imgui_demo.cpp"
#include "graphics/deffered/imguirender.cpp"

#include "graphics/deffered/imgui_impl.cpp"

#include "editor/editor.h"



#include "asset/AssetSystem.cpp"

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
        //order of importantce
        //TODO(Ray):FBXSDK inclusion .. set up precompiled headers
        AssetSystem::Init();
        //TODO(Ray):Go ahead and set up physx and fmod see how compile times will fair    
        //TODO(Ray):AssetSystem:loading serializing meta prefabs shaders loading etc etc..

        //TODO(Ray):Graphics:Deffered needs some assets first.
        //TODO(Ray):Physx and Audio integration
        //TODO(Ray):Multi threading facilities
        //TODO(Ray):Memory tracking system
        //TODO(Ray):Profiling System
        //TODO(Ray):Networking 
        
        APIFileOptions::data_dir = "/data/";
        //TODO(Ray):Set Asset or file system to hold this 
//        es.base_path_to_data = BuildPathToAssets(&ps->string_state.string_memory, Directory_None);
        StringsHandler::Init();

        //NOTE(Ray):Probably move this to a more renderer specific area
        RenderCache::Init(3000);
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
        StringsHandler::ResetTransientStrings();
    }

    //TODO(Ray):Fixed update.
    
    //NOTE(Ray):Here we do the updating of everything engine related.
    void Update()
    {
        PlatformOutput(engine_log,"Engine Update Begin\n");
        
        EngineInput::PullMouseState(&ps);

        gameUpdate();
#if TARGET_OS_OSX
        ImGui_ImplOSX_NewFrame();
#endif
        ImGui::NewFrame();
        
        Editor::Update();

        // Pre render data preparation.
        ImGui::Render();
        
        //TODO(ray):Find a place to give this a more rigourous test.
        //PlatformOutput(engine_log,"Test vec size %d \n",test_vector.max_size);
        //float2 newf2 = float2(100,100);
        //YoyoStretchPushBack(&test_vector,newf2);
        
        //Update Graphics
        DefferedRenderer::ExecutePasses();
        //Update Audio
        //Update Physics
        PlatformOutput(engine_log,"Engine Update Complete\n");

        StringsHandler::ResetTransientStrings();
    }
};

#endif
