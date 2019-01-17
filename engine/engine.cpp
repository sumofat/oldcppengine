
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
        static bool show_demo_window = true;
        static bool show_another_window = false;
        static float clear_color[4] = { 0.28f, 0.36f, 0.5f, 1.0f };
        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);
        
        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;
            
            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
            
            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);
            
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
            
            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);
            
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }
        
        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }
        
        // Rendering
        ImGui::Render();
        
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
