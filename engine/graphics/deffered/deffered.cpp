
#include "deffered.h"

#ifdef ENGINEIMPL
namespace DefferedRenderer
{
    RenderPassBuffer shadow_gen_pass_buffer;//Creates shadow maps
    RenderPassBuffer gbuffer_pass_buffer;//creates diffuse normal etc.. buffers
    RenderPassBuffer stencil_lighting_pass_buffer;//writes the lit areas to the stencil buffer
    RenderPassBuffer lighting_accumulation_pass_buffer;//does lighting 
    RenderPassBuffer composite_pass_buffer;//composites all gbuffers and lighting together
    RenderPassBuffer transparent_pass_buffer;
    RenderPassBuffer ui_pass_buffer;
    RenderPassBuffer overlay_pass_buffer;

    RenderMaterial gbuffer_material;
    
    void Init(RenderCamera* cam)
    {
        RenderPassCode::InitRenderPass(&gbuffer_pass_buffer,1,RenderPassCode::ExecuteRenderPasses);
        RenderCommandBuffer test_c_buffer;
        RenderCommandCode::InitRenderCommand(&test_c_buffer,cam,sizeof(RenderWithMaterialCommand),0,RenderCommandCode::ExecuteCommands,(void*)&gbuffer_material);
    }
        
    void ExecutePasses()
    {
        void* c_buffer = RenderEncoderCode::CommandBuffer();

        if(gbuffer_pass_buffer.ExecutePasses)
            gbuffer_pass_buffer.ExecutePasses(&gbuffer_pass_buffer,c_buffer,0);
    }
};
#endif

