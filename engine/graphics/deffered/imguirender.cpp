#include "imguirender.h"
#include "imgui_impl.h"
#ifdef ENGINEIMPL

namespace IMGUIRender
{
    RenderCommandEncoder renderEncoder;
    void ExecutePass(RenderPassBuffer* buffer,void* c_buffer,void* global_pass_params);
    void Init(DefferedRenderPass* pass,PlatformState* ps)
    {
        //_commandQueue = [_device newCommandQueue];

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
        void* command_buffer = global_pass_params;
        ImGuiIO &io = ImGui::GetIO();
        float2 dim = RendererCode::dim;
        io.DisplaySize.x = dim.x();//view.bounds.size.width;
        io.DisplaySize.y = dim.y();//view.bounds.size.height;

        float frame_buffer_scale = RendererCode::display_scale_factor;
#if TARGET_OS_OSX
        //CGFloat framebufferScale = view.window.screen.backingScaleFactor ?: NSScreen.mainScreen.backingScaleFactor;
        //float frameBufferScale = RendererCode::display_scale_factor;
#else
       // CGFloat framebufferScale = view.window.screen.scale ?: UIScreen.mainScreen.scale;
#endif
        io.DisplayFramebufferScale = ImVec2(frame_buffer_scale, frame_buffer_scale);

        io.DeltaTime = 1 / float(RendererCode::preferred_frame_per_second ?: 60);
    
//        commandBuffer = [self.commandQueue commandBuffer];
//        commandBuffer = RenderEncoderCode::CommandBuffer();
        
        static bool show_demo_window = true;
        static bool show_another_window = false;
        static float clear_color[4] = { 0.28f, 0.36f, 0.5f, 1.0f };
    
//        renderPassDescriptor = view.currentRenderPassDescriptor;
//        if (renderPassDescriptor != nil)
        RenderPass* pass;
        //Here  you will do any setup for all the passes...
        while(pass = YoyoIterateVector(&buffer->buffer,RenderPass))
        {
//NOTE(Ray):We wont be clearing the color because we are loading and drawing on top
//            renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);

            Drawable current_drawable = RenderEncoderCode::GetDefaultDrawableFromView();
            if(current_drawable.state)
            {
                RenderEncoderCode::SetRenderPassColorAttachmentTexture(&current_drawable.texture,&pass->desc,0);
                RenderEncoderCode::SetRenderPassColorAttachmentDescriptor(&pass->desc,0);
                
                RenderCommandEncoder re = RenderEncoderCode::RenderCommandEncoderWithDescriptor(c_buffer,&pass->desc);
    //            renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
    //TODO(Ray):Add this to metalizer
    //            [renderEncoder pushDebugGroup:@"ImGui demo"];

                // Start the Dear ImGui frame
                //ImGui_ImplMetal_NewFrame(renderPassDescriptor);
    #if TARGET_OS_OSX
                ImGui_ImplOSX_NewFrame();
    #endif
                ImGui::NewFrame();

                // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
                if (show_demo_window)
                    ImGui::ShowDemoWindow(&show_demo_window);

                // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
                {
                    static float f = 0.0f;
                    static int counter = 0;

                    ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

                    ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
                    ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
                    ImGui::Checkbox("Another Window", &show_another_window);

                    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
                    ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

                    if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                        counter++;
                    ImGui::SameLine();
                    ImGui::Text("counter = %d", counter);

                    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                    ImGui::End();
                }

                // 3. Show another simple window.
                if (show_another_window)
                {
                    ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
                    ImGui::Text("Hello from another window!");
                    if (ImGui::Button("Close Me"))
                        show_another_window = false;
                    ImGui::End();
                }

                // Rendering
                ImGui::Render();
                ImDrawData *drawData = ImGui::GetDrawData();
                ImGui_ImplMetal_RenderDrawData(drawData, command_buffer, &re,&pass->desc);
                //TODO(Ray):Add to metalizer
    //            [renderEncoder popDebugGroup];
    //            [renderEncoder endEncoding];
                RenderEncoderCode::EndEncoding(&re);
    //            [commandBuffer presentDrawable:view.currentDrawable];
            }
        }
         YoyoResetVectorIterator(&buffer->buffer);
        //[commandBuffer commit];
    }
   
}

#endif
