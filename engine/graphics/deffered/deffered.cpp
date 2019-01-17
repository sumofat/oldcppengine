
#include "deffered.h"
#include "imguirender.h"

#ifdef ENGINEIMPL
namespace DefferedRenderer
{
    DefferedRenderPassBuffer passes;
    DefferedRenderPass gbufferpass;
    DefferedRenderPass imguipass;
    
    RenderPassBuffer shadow_gen_pass_buffer;//Creates shadow maps
    //RenderPassBuffer gbuffer_pass_buffer;//creates diffuse normal etc.. buffers
    RenderPassBuffer stencil_lighting_pass_buffer;//writes the lit areas to the stencil buffer
    RenderPassBuffer lighting_accumulation_pass_buffer;//does lighting 
    RenderPassBuffer composite_pass_buffer;//composites all gbuffers and lighting together
    RenderPassBuffer transparent_pass_buffer;
    RenderPassBuffer ui_pass_buffer;
    RenderPassBuffer overlay_pass_buffer;
    RenderPassBuffer imgui_pass_buffer;//editor ui 

    void InitPerProjPass(RenderCamera* cam,PlatformState* ps,DefferedRenderPass* pass)
    {
        RenderPassCode::InitRenderPass(&pass->pass_buffer,1,RenderPassCode::ExecuteRenderPasses);
        RenderCommandBuffer command_buffer;
        RenderCommandCode::InitRenderCommand(&command_buffer,cam,sizeof(RenderWithMaterialCommand),0,RenderCommandCode::ExecuteCommands,(void*)&pass->material);
        
        //2. Add a Pass to RenderPass
        //first create a depth state
        TextureDescriptor depth_texture_desc = RendererCode::Texture2DDescriptorWithPixelFormat(PixelFormatDepth32Float_Stencil8,ps->window.dim.x(),ps->window.dim.y(),false);
        depth_texture_desc.usage       = TextureUsageRenderTarget;
        depth_texture_desc.storageMode = StorageModePrivate;
        depth_texture_desc.pixelFormat = PixelFormatDepth32Float_Stencil8;
        Texture depth_texture = RendererCode::NewTextureWithDescriptor(depth_texture_desc);
        RenderPassDescriptor renderpass_desc = RenderEncoderCode::NewRenderPassDescriptor();
        //depth attachment description
        renderpass_desc.depth_attachment.description.texture       = depth_texture;
        renderpass_desc.depth_attachment.description.loadAction    = LoadActionClear;
        renderpass_desc.depth_attachment.description.storeAction   = StoreActionStore;
        renderpass_desc.depth_attachment.clear_depth    = 10000.0f;
        //stencil attachment description
        renderpass_desc.stencil_attachment.description.texture     = depth_texture;
        renderpass_desc.stencil_attachment.description.loadAction  = LoadActionClear;
        renderpass_desc.stencil_attachment.description.storeAction = StoreActionStore;
        
        RendererCode::SetRenderPassDescriptor(&renderpass_desc);
        float4 sky_blue = float4(0.529f,0.807f,0.9215f,1.0f);
        RenderPassColorAttachmentDescriptor rp_ca_desc = {};
        rp_ca_desc.clear_color = sky_blue;
        rp_ca_desc.description.loadAction = LoadActionClear;
        rp_ca_desc.description.storeAction = StoreActionStore;
        
        //TODO(Ray):Create an actual render texture to render into we can no longer just
        //render into the default cametallayer render texture. for testing we render default for now.
        RenderEncoderCode::AddRenderPassColorAttachment(&renderpass_desc,&rp_ca_desc);
        RenderEncoderCode::SetRenderPassColorAttachmentDescriptor(&renderpass_desc,0);
        int no_of_passes = 1;
        int sizeofc = sizeof(RenderCommandBuffer);
        RenderPass* subpass = RenderPassCode::AddRenderPass(&pass->pass_buffer, no_of_passes, sizeofc,0,&renderpass_desc);
        subpass->viewport = float4(0,0,ps->window.dim.x(),ps->window.dim.y());
        //add a command buffer to a pass associated with a render a pass
        pass->pass_command_buffer = RenderPassCode::CreateRenderPassCommandBuffer(subpass, &command_buffer);
        
        cam->projection_matrix = init_pers_proj_matrix(ps->window.dim,cam->fov,cam->near_far_planes);
        float3 cam_p = float3(0,2,5);//Parametize init p
        cam->matrix = set_camera_view(cam_p, float3(0,0,1), float3(0,1,0));
    }
    
    //TODO(Ray):Create a render quad pass able to render into any quad
    //texture.  Used for full screen passes or downsampled passes upsampled passes render textures etc...
    void InitRenderQuadPass(RenderCamera* cam,PlatformState* ps)
    {
    }
    
    void InitTransparentPass(RenderCamera* cam,PlatformState* ps)
    {
    }
    
    void Init(RenderCamera* cam,PlatformState* ps)
    {
        passes.buffer = YoyoInitVector(10, RenderPassBuffer, false);
        InitPerProjPass(cam, ps, &gbufferpass);
        YoyoPushBack(&passes.buffer, gbufferpass);

        //imgui
        
        IMGUIRender::Init(&imguipass,ps);
        YoyoPushBack(&passes.buffer, imguipass);
    }
    
    //TODO(Ray):Later we should have a render graph that would have intimate knowledge of the passes to be executed
    //and control what does what and when.  For now we just execute all passes everytime as the low lever renderer is
    //just a basic command execute loop
    void ExecutePasses()
    {
        void* c_buffer = RenderEncoderCode::CommandBuffer();

        RenderPassBuffer* render_pass;
        while (render_pass = YoyoIterateVector(&passes.buffer, RenderPassBuffer))
        {
            if(render_pass->ExecutePasses)
                render_pass->ExecutePasses(render_pass,c_buffer,0);
        }
        YoyoResetVectorIterator(&passes.buffer);
        
        Drawable current_drawable = RenderEncoderCode::GetDefaultDrawableFromView();
        if(current_drawable.state)
        {
            RenderEncoderCode::PresentDrawable(c_buffer,current_drawable.state);
        }
        RenderEncoderCode::Commit(c_buffer);

    }
};
#endif

