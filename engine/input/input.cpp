#include "../metalizer/RendererInclude.h"
#ifdef ENGINEIMPL
namespace EngineInput
{
    bool log = true;
    Input last_known_input_state;

#define MAX_DIGITAL_BUTTON_QUEUE 100
    int button_queue_count;
    DigitalButton button_queue[MAX_DIGITAL_BUTTON_QUEUE];
    
#ifdef OSX || WIN
    void UpdateDigitalButton(DigitalButton* button,u32 state)
    {
        b32 was_down = button->down;
        b32 down = state;
        button->pressed = !was_down && down;
        button->released = was_down && !down;
        button->down = down;
    }

    void PullMouseState(PlatformState* ps)
    {
        Input* input = &ps->input;
        if(input)
        {
            float2 mp = PlatformGraphicsAPI_Metal::PullMouseState();
            float2 window_dim = float2(ps->window.dim.x(),ps->window.dim.y());
            input->mouse.p = float2(mp.x(),(window_dim.y)() - mp.y());
            input->mouse.delta_p = input->mouse.prev_p - input->mouse.p;
            input->mouse.prev_uv = input->mouse.uv;
            input->mouse.uv = float2(input->mouse.p.x() / ps->window.dim.x(), input->mouse.p.y() / ps->window.dim.y());
            input->mouse.delta_uv = input->mouse.prev_uv - input->mouse.uv;
            input->mouse.prev_p = input->mouse.p;
            PlatformOutput(log,"mousep: %f:%f\n",mp.x(),mp.y());
            last_known_input_state = *input;
            
            uint32_t mbstate = PlatformInputAPI_Metal::GetMouseButtonState();
            u32 lmbstate = (mbstate >> 0) & 0x00000001;
            UpdateDigitalButton(&input->mouse.lmb,lmbstate);
            u32 rmbstate = (mbstate >> 1) & 0x00000001;
            UpdateDigitalButton(&input->mouse.rmb,rmbstate);
        }
    }

    //TODO(Ray):Dont like return a pointer here going with this for now.
    DigitalButton* GetLastKeyPress()
    {
        if(button_queue_count <= 0)return nullptr;
        return &button_queue[button_queue_count - 1];
    }
    
    void PushDigitalButtonInput(DigitalButton b)
    {
        button_queue[button_queue_count] = b;
        button_queue_count++;        
    }

    void ClearButtonQueue()
    {
        for(int i = 0;i < MAX_DIGITAL_BUTTON_QUEUE;++i)
        {
            button_queue[i] = {};
        }
        button_queue_count = 0;
    }

    void ResetKeys(PlatformState* ps)
    {
#if OSX
        Input* input = &ps->input;
        for(int i = 0;i < MAX_KEYS;++i)
        {
            input->keyboard.keys[i].released = false;
            input->keyboard.keys[i].pressed = false;
        }
#endif
    }
    
#if 0
    //NOTE(Ray):for now we are only concerned with the last character pressed before this frame
    //TODO(Ray):Get all the keys pressed in between frames this would be more for key typing etc...
    void PullKeys(PlatformState* ps)
    {
        Input* input = &ps->input;
        if(input)
        {
            char* last_key_press = PlatformGraphicsAPI_Metal::PullKeyState();
            uint i = (u32)*last_key_press;

            DigitalButton* button = &input->keyboard.keys[i];
            UpdateDigitalButton(&button,1)
            b32 was_down = button->down;
            b32 down = downorup;
            button->pressed = !was_down && down;
            button->released = was_down && !down;
            button->down = down;
        }
    }
#endif   
#endif
    
    Input GetInput()
    {
        return last_known_input_state;
    }
}

#endif


