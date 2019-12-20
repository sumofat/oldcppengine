#include "../../metalizer/glemu/glemu.h"
#include "deffered.h"
#include "imguirender.h"

#ifdef ENGINEIMPL
namespace DefferedRenderer
{
    DefferedRenderPassBuffer passes;
    DefferedRenderPass gbufferpass;

    //imgui passes
    DefferedRenderPass imgui3drendertexturepasses;
    DefferedRenderPass imguipass;
    DefferedRenderPass composite_pass;

    RenderPassBuffer shadow_gen_pass_buffer;//Creates shadow maps
    //RenderPassBuffer gbuffer_pass_buffer;//creates diffuse normal etc.. buffers
    RenderPassBuffer stencil_lighting_pass_buffer;//writes the lit areas to the stencil buffer
    RenderPassBuffer lighting_accumulation_pass_buffer;//does lighting 
    RenderPassBuffer composite_pass_buffer;//composites all gbuffers and lighting together
    RenderPassBuffer transparent_pass_buffer;
    RenderPassBuffer ui_pass_buffer;
    RenderPassBuffer overlay_pass_buffer;
    RenderPassBuffer imgui_pass_buffer;//editor ui 

    GPUBuffer uniform_buffer;

    //temp
    Texture render_tex;
    RenderCamera full_screen_quad_cam;
    GPUBuffer full_screen_quad_resource;
    RenderMaterial full_screen_quad_material;

    bool output_log = false;
    OpenGLEmuState ogl_test_state;
    GLProgram diffuse_program;    
    void Init(bool is_log)
    {
        output_log = is_log;

    }
    
    void ExecuteFullScreenQuadCommands(RenderPass* pass,RenderCommandBuffer* buffer,void* command_params_)
    {
        PlatformOutput(output_log,"Executing FullScreen Quad Command.\n");
        RenderCommandEncoder re = pass->render_encoder;
        RenderWithFullScreenCommand* command;
        while ((command = YoyoIterateVector(&buffer->buffer, RenderWithFullScreenCommand)))
        {
            RenderMaterial material = command->material;
            RenderEncoderCode::SetRenderPipelineState(&re,material.pipeline_state.state);
            RenderEncoderCode::SetDepthStencilState(&re,&material.depth_stencil_state);
            RenderEncoderCode::SetFragmentTexture(&re,&command->texture,0);
            RenderEncoderCode::SetVertexBuffer(&re,&command->resource,0,0);
            RenderEncoderCode::DrawPrimitives(&re,primitive_type_triangle,0,6);
        }
        YoyoClearVector(&buffer->buffer);
    }
    
    void InitPerProjPass(RenderCamera* cam,PlatformState* ps,DefferedRenderPass* pass,Texture* texture)
    {
        RenderPassCode::InitRenderPass(&pass->pass_buffer,1,RenderPassCode::ExecuteRenderPasses);
        RenderCommandBuffer command_buffer;
        RenderCommandCode::InitRenderCommand(&command_buffer,cam,sizeof(RenderWithMaterialCommand),0,RenderCommandCode::ExecuteCommands,(void*)&pass->material);
        pass->pass_buffer.id = (uint32_t)pass_gbuffer;
        //2.Add a Pass to RenderPass
        //first create a depth state
        TextureDescriptor depth_texture_desc = RendererCode::Texture2DDescriptorWithPixelFormat(PixelFormatDepth32Float_Stencil8,ps->window.dim.x(),ps->window.dim.y(),false);
        depth_texture_desc.usage       = (TextureUsage)(TextureUsageRenderTarget | TextureUsageShaderRead);
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

        RenderEncoderCode::SetRenderPassColorAttachmentTexture(texture,&renderpass_desc,0);
        RenderEncoderCode::SetRenderPassColorAttachmentDescriptor(&renderpass_desc,0);
        
        int no_of_passes = 1;
        int sizeofc = sizeof(RenderCommandBuffer);
        RenderPass* subpass = RenderPassCode::AddRenderPass(&pass->pass_buffer, no_of_passes, sizeofc,0,&renderpass_desc);
        subpass->viewport = float4(0,0,ps->window.dim.x(),ps->window.dim.y());
        //add a command buffer to a pass associated with a render a pass
        pass->pass_command_buffer = RenderPassCode::CreateRenderPassCommandBuffer(subpass, &command_buffer);
        
        cam->projection_matrix = init_pers_proj_matrix(ps->window.dim,cam->fov,cam->near_far_planes);
        float3 cam_p = float3(0,0,0);//Parametize init p
        cam->matrix = set_camera_view(cam_p, float3(0,0,1), float3(0,1,0));
        //Default uniform_buffer
        uniform_buffer = RenderGPUMemory::NewBufferWithLength(sizeof(Uniforms),ResourceCPUCacheModeDefaultCache);
    }

    void InitFullScreenQuadPass(RenderCamera* cam,PlatformState* ps,DefferedRenderPass* pass)
    {
        RenderPassCode::InitRenderPass(&pass->pass_buffer,1,RenderPassCode::ExecuteRenderPasses);
        RenderCommandBuffer command_buffer;
        RenderCommandCode::InitRenderCommand(&command_buffer,cam,sizeof(RenderWithMaterialCommand),0,ExecuteFullScreenQuadCommands,(void*)&pass->material);
        pass->pass_buffer.id = (uint32_t)pass_composite;        
        //2.Add a Pass to RenderPass
        //first create a depth state
        TextureDescriptor depth_texture_desc = RendererCode::Texture2DDescriptorWithPixelFormat(PixelFormatDepth32Float_Stencil8,ps->window.dim.x(),ps->window.dim.y(),false);
        depth_texture_desc.usage       = TextureUsageRenderTarget;
        depth_texture_desc.storageMode = StorageModePrivate;
        depth_texture_desc.pixelFormat = PixelFormatDepth32Float_Stencil8;
        Texture depth_texture = RendererCode::NewTextureWithDescriptor(depth_texture_desc);
        RenderPassDescriptor renderpass_desc = RenderEncoderCode::NewRenderPassDescriptor();

        //depth attachment description
        renderpass_desc.depth_attachment.description.texture       = depth_texture;
        renderpass_desc.depth_attachment.description.loadAction    = LoadActionDontCare;
        renderpass_desc.depth_attachment.description.storeAction   = StoreActionDontCare;
        renderpass_desc.depth_attachment.clear_depth    = 10000.0f;
        //stencil attachment description
        renderpass_desc.stencil_attachment.description.texture     = depth_texture;
        renderpass_desc.stencil_attachment.description.loadAction  = LoadActionDontCare;
        renderpass_desc.stencil_attachment.description.storeAction = StoreActionDontCare;
        
        RendererCode::SetRenderPassDescriptor(&renderpass_desc);
        float4 sky_blue = float4(0.529f,0.807f,0.9215f,1.0f);
        RenderPassColorAttachmentDescriptor rp_ca_desc = {};
        rp_ca_desc.clear_color = sky_blue;
        rp_ca_desc.description.loadAction = LoadActionClear;
        rp_ca_desc.description.storeAction = StoreActionStore;

        RenderEncoderCode::AddRenderPassColorAttachment(&renderpass_desc,&rp_ca_desc);
        RenderEncoderCode::SetRenderPassColorAttachmentDescriptor(&renderpass_desc,0);
        
        int no_of_passes = 1;
        int sizeofc = sizeof(RenderCommandBuffer);
        RenderPass* subpass = RenderPassCode::AddRenderPass(&pass->pass_buffer, no_of_passes, sizeofc,0,&renderpass_desc);
        subpass->viewport = float4(0,0,ps->window.dim.x(),ps->window.dim.y());
        //add a command buffer to a pass associated with a render a pass
        pass->pass_command_buffer = RenderPassCode::CreateRenderPassCommandBuffer(subpass, &command_buffer);
        
//        cam->projection_matrix = init_pers_proj_matrix(ps->window.dim,cam->fov,cam->near_far_planes);
        cam->projection_matrix = float4x4::identity();//init_screen_space_matrix(ps->window.dim,cam->fov,cam->near_far_planes);
        float3 cam_p = float3(0,0,0);//Parametize init p
        cam->matrix = float4x4::identity();
        //Default uniform_buffer
        uniform_buffer = RenderGPUMemory::NewBufferWithLength(sizeof(Uniforms),ResourceCPUCacheModeDefaultCache);

        //Init the full screen quad and uvs
        float quad_p_uvs[] =
        {
            -1.0, -1.0, 0.0,            0.0, 1.0,
            1.0,  -1.0, 0.0,            1.0, 1.0,
            1.0,   1.0, 0.0,            1.0, 0.0,
            -1.0, -1.0, 0.0,            0.0, 1.0,
            1.0,   1.0, 0.0,            1.0, 0.0,
            -1.0,  1.0, 0.0,            0.0, 0.0
        };

        full_screen_quad_material = AssetSystem::CreateDefaultQuadMaterial();
        full_screen_quad_resource = RenderGPUMemory::NewBufferAndUpload((void*)&quad_p_uvs,sizeof(quad_p_uvs),ResourceStorageModeShared);
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
//TODO(Ray):Add a perliminary pass for the editor that will do any rendering to a texture target
        //for displaying to the uis that we will need to pass to imgui
        //InitRenderToTexturesForUI();
        TextureDescriptor texture_desc = RendererCode::Texture2DDescriptorWithPixelFormat(PixelFormatDepth32Float_Stencil8,ps->window.dim.x(),ps->window.dim.y(),false);
        texture_desc.usage       = (TextureUsage)(TextureUsageRenderTarget | TextureUsageShaderRead);
        texture_desc.storageMode = StorageModePrivate;
        texture_desc.pixelFormat = PixelFormatBGRA8Unorm;
        render_tex = RendererCode::NewTextureWithDescriptor(texture_desc);
        
        passes.buffer = YoyoInitVector(10, RenderPassBuffer, false);
        InitPerProjPass(cam, ps, &gbufferpass,&render_tex);
        //TODO(Ray):Wwe should initialize once but reset and recompile our pass orders every frame allowing more flexibility
        //in the overall composite and order.

        YoyoPushBack(&passes.buffer, gbufferpass);
#ifdef OSX
        //imgui
        IMGUIRender::Init(&imguipass,ps,&render_tex);
        YoyoPushBack(&passes.buffer, imguipass);
#endif
        //Composite pass will combine all passes and blit to drawable texture
        InitFullScreenQuadPass(&full_screen_quad_cam,ps,&composite_pass);
        YoyoPushBack(&passes.buffer, composite_pass);
        
        OpenGLEmu::Init();
    }
    
    void PreframeSetup()
    {
        OpenGLEmu::PreFrameSetup();
        ogle_pre_frame_setup(&ogl_test_state);
    }
    
    //TODO(Ray):Later we should have a render graph that would have intimate knowledge of the passes to be executed
    //and control what does what and when.  For now we just execute all passes everytime as the low lever renderer is
    //just a basic command execute loop
    void ExecutePasses()
    {
        RenderWithFullScreenCommand command_with_material;
        command_with_material.material     = full_screen_quad_material;
        command_with_material.resource     = full_screen_quad_resource;

#if 1
        void* c_buffer = RenderEncoderCode::CommandBuffer();
        Drawable current_drawable;
        RenderPassBuffer* render_pass = {};
        while ((render_pass = YoyoIterateVector(&passes.buffer, RenderPassBuffer)))
        {
            if(render_pass->ExecutePasses)
            {
#if 1
                if(render_pass->id == (uint32_t)pass_composite)//will be last pass
                {
                    current_drawable = RenderEncoderCode::GetDefaultDrawableFromView();
                    if(!current_drawable.state)continue;
                    command_with_material.texture = render_tex;         
                    RenderCommandCode::AddRenderCommand(DefferedRenderer::composite_pass.pass_command_buffer, (void*)&command_with_material);
                    render_pass->ExecutePasses(render_pass,c_buffer,(void*)&current_drawable.texture);                    
                }
                else
#endif
                {
                    render_pass->ExecutePasses(render_pass,c_buffer,0);                    
                }
            }
        }
        YoyoResetVectorIterator(&passes.buffer);
#endif
        
        ogle_execute_enqueue(&ogl_test_state,c_buffer);
        ogle_execute_commit(&OpenGLEmu::ogs,c_buffer,true);
//        ogle_execute_commit(&ogl_test_state,c_buffer,true);        
    }
};
#endif

