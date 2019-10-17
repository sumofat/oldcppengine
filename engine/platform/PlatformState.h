    

struct Window
{
	float2 dim;
    float2 p;
    bool is_full_screen_mode;
};

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
    Window window;

    bool is_running = false;
    bool is_init = false;
};

