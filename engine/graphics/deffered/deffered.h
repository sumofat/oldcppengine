#if !defined(DEFFEREDRENDERER_H)

struct DefferedRenderPass
{
    RenderPassBuffer pass_buffer;
    RenderCommandBuffer* pass_command_buffer;//s
    RenderMaterial material;
};

struct DefferedRenderPassBuffer
{
    YoyoVector buffer;
};

namespace DefferedRenderer
{
    extern DefferedRenderPassBuffer passes;
    
    extern DefferedRenderPass gbufferpass;
    
    extern RenderPassBuffer shadow_gen_pass_buffer;//Creates shadow maps
    extern RenderPassBuffer gbuffer_pass_buffer;//creates diffuse normal etc.. buffers
    
    
    
    extern RenderPassBuffer stencil_lighting_pass_buffer;//writes the lit areas to the stencil buffer
    extern RenderPassBuffer lighting_accumulation_pass_buffer;//does lighting 
    extern RenderPassBuffer composite_pass_buffer;//composites all gbuffers and lighting together
    extern RenderPassBuffer transparent_pass_buffer;
    extern RenderPassBuffer ui_pass_buffer;
    extern RenderPassBuffer overlay_pass_buffer;

    void Init(RenderCamera* cam,PlatformState* ps);
    //void InitPerProjPass(RenderCamera* cam,PlatformState* ps,DefferedRenderPass* pass);
    
    void ExecutePasses();
};

#define DEFFEREDRENDERER_H
#endif
