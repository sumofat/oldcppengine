
#ifdef ENGINEIMPL
namespace EngineInput
{
    bool log = true;
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
        }
    }
}

#endif


