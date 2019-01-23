#include "imguirender.h"
#include "imgui_impl.h"
#ifdef ENGINEIMPL

namespace IMGUIRender
{
    PlatformState* platform_state;
    RenderCommandEncoder renderEncoder;
    void ExecutePass(RenderPassBuffer* buffer,void* c_buffer,void* global_pass_params);
    void Init(DefferedRenderPass* pass,PlatformState* ps)
    {
        //_commandQueue = [_device newCommandQueue];
        Assert(ps);
        platform_state = ps;
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        
        RenderPassCode::InitRenderPass(&pass->pass_buffer,1,IMGUIRender::ExecutePass);

        TextureDescriptor depth_texture_desc = RendererCode::Texture2DDescriptorWithPixelFormat(PixelFormatDepth32Float_Stencil8,ps->window.dim.x(),ps->window.dim.y(),false);
        depth_texture_desc.usage       = TextureUsageRenderTarget;
        depth_texture_desc.storageMode = StorageModePrivate;
        depth_texture_desc.pixelFormat = PixelFormatDepth32Float_Stencil8;
        Texture depth_texture = RendererCode::NewTextureWithDescriptor(depth_texture_desc);        
        RenderPassDescriptor renderpass_desc = RenderEncoderCode::NewRenderPassDescriptor();
        //depth attachment description
        renderpass_desc.depth_attachment.description.texture       = depth_texture;
        renderpass_desc.depth_attachment.description.loadAction    = LoadActionClear;
        renderpass_desc.depth_attachment.description.storeAction   = StoreActionDontCare;
        renderpass_desc.depth_attachment.clear_depth    = 10000.0f;
        //stencil attachment description
        renderpass_desc.stencil_attachment.description.texture     = depth_texture;
        renderpass_desc.stencil_attachment.description.loadAction  = LoadActionClear;
        renderpass_desc.stencil_attachment.description.storeAction = StoreActionDontCare;
        
        RendererCode::SetRenderPassDescriptor(&renderpass_desc);

        RenderPassColorAttachmentDescriptor rp_ca_desc = {};
        rp_ca_desc.clear_color = float4(1);
        rp_ca_desc.description.loadAction = LoadActionLoad;
        rp_ca_desc.description.storeAction = StoreActionStore;
        
        RenderEncoderCode::AddRenderPassColorAttachment(&renderpass_desc,&rp_ca_desc);
        RenderEncoderCode::SetRenderPassColorAttachmentDescriptor(&renderpass_desc,0);
        
        ImGui_ImplMetal_Init(RendererCode::device, &renderpass_desc);
        
        int no_os_passes = 1;
        RenderPass* subpass = RenderPassCode::AddRenderPass(&pass->pass_buffer, no_os_passes, 0,0,&renderpass_desc);
        subpass->viewport = float4(0,0,ps->window.dim.x(),ps->window.dim.y());
    }

    void ExecutePass(RenderPassBuffer* buffer,void* c_buffer,void* global_pass_params)
    {
        
        ImGuiIO &io = ImGui::GetIO();
        io.MousePos = ImVec2(platform_state->input.mouse.p.x(),platform_state->input.mouse.p.y());
        float2 dim = RendererCode::dim;
        io.DisplaySize.x = dim.x();//view.bounds.size.width;
        io.DisplaySize.y = dim.y();//view.bounds.size.height;

        float frame_buffer_scale = RendererCode::display_scale_factor;
        io.DisplayFramebufferScale = ImVec2(frame_buffer_scale, frame_buffer_scale);
        io.DeltaTime = 1 / float(RendererCode::preferred_frame_per_second ?: 60);
        RenderPass* pass;
        //Here  you will do any setup for all the passes...
        while(pass = YoyoIterateVector(&buffer->buffer,RenderPass))
        {

            Drawable current_drawable = RenderEncoderCode::GetDefaultDrawableFromView();
            if(current_drawable.state)
            {
                RenderEncoderCode::SetRenderPassColorAttachmentTexture(&current_drawable.texture,&pass->desc,0);
                RenderEncoderCode::SetRenderPassColorAttachmentDescriptor(&pass->desc,0);
                
                RenderCommandEncoder re = RenderEncoderCode::RenderCommandEncoderWithDescriptor(c_buffer,&pass->desc);
   //TODO(Ray):Add this to metalizer
    //            [renderEncoder pushDebugGroup:@"ImGui demo"];

                ImDrawData *drawData = ImGui::GetDrawData();
                ImGui_ImplMetal_RenderDrawData(drawData, c_buffer, &re,&pass->desc);
                //TODO(Ray):Add to metalizer
    //            [renderEncoder popDebugGroup];
    //            [renderEncoder endEncoding];
                RenderEncoderCode::EndEncoding(&re);
            }
        }
         YoyoResetVectorIterator(&buffer->buffer);
        //[commandBuffer commit];
    }
   
}

#endif
