#if !defined(INPUT_H)

#define MAX_KEYS 256
#define MAX_CONTROLLER_SUPPORT 8
struct DigitalButton
{
    b32 down;
    bool pressed;
    bool released;
};

struct Keyboard
{
    DigitalButton keys[MAX_KEYS];    
};

struct AnalogButton
{
    f32 threshold;
    f32 value;
    b32 down;
    b32 pressed;
    b32 released;
};

struct Axis
{
    f32 value;
    f32 threshold;
};

//TODO(Ray):Handle other types of axis like ruddder/throttle etc...
struct Stick
{
    Axis X;
    Axis Y;
};

struct GamePad
{
    Stick left_stick;
    Stick right_stick;
    
    Axis left_shoulder;
    Axis right_shoulder;

    DigitalButton up;
    DigitalButton down;
    DigitalButton left;
    DigitalButton right;

    DigitalButton a;
    DigitalButton b;
    DigitalButton x;
    DigitalButton y;

    DigitalButton l;
    DigitalButton r;

    DigitalButton select;
    DigitalButton start;

    void* state;    
};

struct Mouse
{
    float2 p;
    float2 prev_p;
    float2 delta_p;
    float2 uv;
    float2 prev_uv;
    float2 delta_uv;

    DigitalButton lmb;//left_mouse_button
    DigitalButton rmb;
	bool wrap_mode;
};

struct Input
{
    Keyboard keyboard;
    Mouse mouse;
    GamePad game_pads[MAX_CONTROLLER_SUPPORT];
};

struct PlatformState;
namespace EngineInput
{
    extern bool log;
#ifdef OSX || WINDOWS
    void PullMouseState(PlatformState* ps);
#endif
    Input GetInput();
}

#define INPUT_H
#endif
