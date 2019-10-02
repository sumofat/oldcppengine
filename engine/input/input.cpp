#include "../metalizer/RendererInclude.h"
#ifdef ENGINEIMPL
namespace EngineInput
{
    bool log = true;
    Input last_known_input_state;
#ifdef OSX || WIN
    void UpdateDigitalButton(DigitalButton* button,u32 state)
    {
        b32 was_down = button->down;
        //b32 down = state >> 7;
        b32 down = state;
        if(down)
        {
            int a= 0;
        }
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
           if(mbstate)
           {
               int a =0;
           }
            u32 lmbstate = (mbstate >> 0) & 0xEFFFFFFF;
            UpdateDigitalButton(&input->mouse.lmb,lmbstate);
            u32 rmbstate = (mbstate >> 1) & 0xEFFFFFFE;
            UpdateDigitalButton(&input->mouse.rmb,rmbstate);

        }
    }
    
   
#endif
    
    Input GetInput()
    {
        return last_known_input_state;
    }
}

#endif


