// dear imgui: Renderer for Metal
// This needs to be used along with a Platform Binding (e.g. OSX)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'MTLTexture' as ImTextureID. Read the FAQ about ImTextureID in imgui.cpp.

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you are new to dear imgui, read examples/README.txt and read the documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2018-11-30: Misc: Setting up io.BackendRendererName so it can be displayed in the About Window.
//  2018-07-05: Metal: Added new Metal backend implementation.
#ifdef ENGINEIMPL
#include "imgui.h"
#include "imgui_impl.h"
#ifdef IOS
#include <CoreFoundation/CoreFoundation.h>
#endif
struct MetalContext
{
    DepthStencilState depthStencilState;
};
static DispatchSemaphoreT semaphore;
static MetalContext g_sharedMetalContext;
static RenderPipelineState render_pipeline_state;
static int current_buffer = 0;
static int buffer_count = 3;
static GPUBuffer buffer[3];
static GPUBuffer gpu_index_buffer[3];
#pragma mark - ImGui API implementation
// We are retrieving and uploading the font atlas as a 4-channels RGBA texture here.
// In theory we could call GetTexDataAsAlpha8() and upload a 1-channel texture to save on memory access bandwidth.
// However, using a shader designed for 1-channel texture would make it less obvious to use the ImTextureID facility to render users own textures.
// You can make that change in your implementation.
void MakeFontTextureWithDevice(RenderDevice device)
{
    ImGuiIO &io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    TextureDescriptor textureDescriptor = RendererCode::Texture2DDescriptorWithPixelFormat(PixelFormatRGBA8Unorm, width, height, false);
    textureDescriptor.usage = TextureUsageShaderRead;
#if TARGET_OS_OSX
    textureDescriptor.storageMode = StorageModeManaged;
#else
    textureDescriptor.storageMode = StorageModeShared;
#endif
    
    Texture texture = RendererCode::NewTextureWithDescriptor(textureDescriptor);
    RenderRegion region =  {float3(0, 0,0), float2(width, height)};
    
    RenderGPUMemory::ReplaceRegion(texture, region, 0,pixels , width * 4);
    
    io.Fonts->TexID = texture.state; // ImTextureID == void*
}


RenderPipelineState InitRenderPipelineStateForFramebufferDescriptor(MetalContext* context,RenderDevice device)
{
    char *shaderSource = ""
    "#include <metal_stdlib>\n"
    "using namespace metal;\n"
    "\n"
    "struct Uniforms {\n"
    "    float4x4 projectionMatrix;\n"
    "};\n"
    "\n"
    "struct VertexIn {\n"
    "    float2 position  [[attribute(0)]];\n"
    "    float2 texCoords [[attribute(1)]];\n"
    "    uchar4 color     [[attribute(2)]];\n"
    "};\n"
    "\n"
    "struct VertexOut {\n"
    "    float4 position [[position]];\n"
    "    float2 texCoords;\n"
    "    float4 color;\n"
    "};\n"
    "\n"
    "vertex VertexOut vertex_main(VertexIn in                 [[stage_in]],\n"
    "                             constant Uniforms &uniforms [[buffer(1)]]) {\n"
    "    VertexOut out;\n"
    "    out.position = uniforms.projectionMatrix * float4(in.position, 0, 1);\n"
    "    out.texCoords = in.texCoords;\n"
    "    out.color = float4(in.color) / float4(255.0);\n"
    "    return out;\n"
    "}\n"
    "\n"
    "fragment half4 fragment_main(VertexOut in [[stage_in]],\n"
    "                             texture2d<half, access::sample> texture [[texture(0)]]) {\n"
    "    constexpr sampler linearSampler(coord::normalized, min_filter::linear, mag_filter::linear, mip_filter::linear);\n"
    "    half4 texColor = texture.sample(linearSampler, in.texCoords);\n"
    "    return half4(in.color) * texColor;\n"
    "}\n";
    
    void* library = RendererCode::CompileShader(shaderSource);
    
    if (library == nullptr)
    {
        PlatformOutput(true,"Could not compile imgui shader.\n");
        //        NSLog(@"Error: failed to create Metal library: %@", error);
        //return nullptr;
        Assert(false);
    }

    ShaderFunction v_func = RendererCode::NewFunctionWithName("vertex_main",library);
    void* vertexFunction = v_func.state;
    ShaderFunction p_func = RendererCode::NewFunctionWithName("fragment_main",library);
    void* fragmentFunction = p_func.state;
    
    if (vertexFunction == nullptr || fragmentFunction == nullptr)
    {
        //NSLog(@"Error: failed to find Metal shader functions in library: %@", error);
        PlatformOutput(true,"Error: failed to find Metal shader functions in library");
        Assert(false);
    }
    
    VertexDescriptor vertexDescriptor = RenderEncoderCode::NewVertexDescriptor();
    //[VertexDescriptor vertexDescriptor];
    VertexAttributeDescriptor vad;
    vad.offset = IM_OFFSETOF(ImDrawVert, pos);
    vad.format = VertexFormatFloat2; // position
    vad.buffer_index = 0;
    RenderEncoderCode::AddVertexAttribute(&vertexDescriptor,vad);
    
    vad.offset = IM_OFFSETOF(ImDrawVert, uv);
    vad.format = VertexFormatFloat2; // texCoords
    vad.buffer_index = 0;
    RenderEncoderCode::AddVertexAttribute(&vertexDescriptor,vad);
    
    vad.offset = IM_OFFSETOF(ImDrawVert, col);
    vad.format = VertexFormatUChar4; // color
    vad.buffer_index = 0;
    RenderEncoderCode::AddVertexAttribute(&vertexDescriptor,vad);
    
    VertexBufferLayoutDescriptor vbld;
    vbld.step_rate = 1;
    vbld.step_function = step_function_per_vertex;
    vbld.stride = sizeof(ImDrawVert);
    RenderEncoderCode::AddVertexLayout(&vertexDescriptor, vbld);
    
    RenderPipelineStateDesc pipelineDescriptor = RenderEncoderCode::CreatePipelineDescriptor(vertexFunction, fragmentFunction, 1);
    
    RenderEncoderCode::SetVertexDescriptor(&pipelineDescriptor,&vertexDescriptor);
    
    pipelineDescriptor.vertex_function = vertexFunction;
    pipelineDescriptor.fragment_function = fragmentFunction;
    //pipelineDescriptor.vertexDescriptor = vertexDescriptor;
    
    pipelineDescriptor.sample_count = 1;//renderpassdesc->sample_count;
    
    //Drawable d = RenderEncoderCode::GetDefaultDrawableFromView();

    pipelineDescriptor.color_attachments.i[0].pixelFormat = PixelFormatBGRA8Unorm;//d.texture.descriptor.pixelFormat;//renderpassdesc->color_pixel_format;
   //pipelineDescriptor.color_attachments.i[0].
    pipelineDescriptor.color_attachments.i[0].blendingEnabled = true;
    pipelineDescriptor.color_attachments.i[0].rgbBlendOperation = BlendOperationAdd;
    pipelineDescriptor.color_attachments.i[0].alphaBlendOperation = BlendOperationAdd;
    pipelineDescriptor.color_attachments.i[0].sourceRGBBlendFactor = BlendFactorSourceAlpha;
    pipelineDescriptor.color_attachments.i[0].sourceAlphaBlendFactor = BlendFactorSourceAlpha;
    pipelineDescriptor.color_attachments.i[0].destinationRGBBlendFactor = BlendFactorOneMinusSourceAlpha;
    pipelineDescriptor.color_attachments.i[0].destinationAlphaBlendFactor = BlendFactorOneMinusSourceAlpha;
    pipelineDescriptor.depthAttachmentPixelFormat = PixelFormatDepth32Float_Stencil8;//renderpassdesc->depth_pixel_format;
     
    pipelineDescriptor.stencilAttachmentPixelFormat = PixelFormatDepth32Float_Stencil8;//renderpassdesc->stencil_pixel_format;
   
    //    RenderPipelineState renderPipelineState = [device newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&error];
    RenderPipelineState renderPipelineState = RenderEncoderCode::NewRenderPipelineStateWithDescriptor(pipelineDescriptor);
    
    //if (error != nullptr)
    {
        //NSLog(@"Error: failed to create Metal pipeline state: %@", error);
    //    PlatformOutput(true,"Error: failed to create Metal pipeline state");
    }
    
    return renderPipelineState;
}

bool ImGui_ImplMetal_Init(RenderDevice device,RenderPassDescriptor* renderpassdesc)
{
    ImGuiIO& io = ImGui::GetIO();
    io.BackendRendererName = "imgui_impl_metal";

    g_sharedMetalContext = {};    
    
//Create a reasonably sized buffer.
    for(int i = 0;i < buffer_count;++i)
    {
        buffer[i] = RenderGPUMemory::NewBufferWithLength(MegaBytes(5), StorageModeShared);
        gpu_index_buffer[i] = RenderGPUMemory::NewBufferWithLength(MegaBytes(5), StorageModeShared);
    }
    
    //buffer = RenderGPUMemory::NewBufferAndUpload(0,MegaBytes(5),StorageModeManaged);
    //gpu_index_buffer = RenderGPUMemory::NewBufferAndUpload(0,MegaBytes(5),StorageModeManaged);

    //ImGui_ImplMetal_CreateDeviceObjects(device);
    render_pipeline_state = InitRenderPipelineStateForFramebufferDescriptor(&g_sharedMetalContext,device);
    DepthStencilDescription depthStencilDescriptor = RendererCode::CreateDepthStencilDescriptor();
    depthStencilDescriptor.depthWriteEnabled = false;
    depthStencilDescriptor.depthCompareFunction = compare_func_always;
    g_sharedMetalContext.depthStencilState = RendererCode::NewDepthStencilStateWithDescriptor(&depthStencilDescriptor);
    MakeFontTextureWithDevice(RendererCode::device);
    semaphore = RenderSynchronization::DispatchSemaphoreCreate(buffer_count);
    return true;
}

MetalContext Init()
{
    MetalContext result = {};
    return result;    
}

void RenderDrawData(ImDrawData *drawData,void* command_buffer,RenderCommandEncoder* command_encoder,RenderPassDescriptor* passdesc)
{

    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    ImGuiIO &io = ImGui::GetIO();
    int fb_width = (int)(drawData->DisplaySize.x * io.DisplayFramebufferScale.x);
    int fb_height = (int)(drawData->DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0 || drawData->CmdListsCount == 0)
        return;
    drawData->ScaleClipRects(io.DisplayFramebufferScale);

    // Setup viewport, orthographic projection matrix
    // Our visible imgui space lies from draw_data->DisplayPos (top left) to
    // draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayMin is typically (0,0) for single viewport apps.

    RenderEncoderCode::SetViewport(command_encoder,0.0f,0.0f,fb_width,fb_height,0.0f,1.0f);
//    [commandEncoder setViewport:viewport];
    float L = drawData->DisplayPos.x;
    float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
    float T = drawData->DisplayPos.y;
    float B = drawData->DisplayPos.y + drawData->DisplaySize.y;
    float N = 0.0f;//.znear;
    float F = 1.0f;//viewport.zfar;
    const float ortho_projection[4][4] =
    {
        { 2.0f/(R-L),   0.0f,           0.0f,   0.0f },
        { 0.0f,         2.0f/(T-B),     0.0f,   0.0f },
        { 0.0f,         0.0f,        1/(F-N),   0.0f },
        { (R+L)/(L-R),  (T+B)/(B-T), N/(F-N),   1.0f },
    };
    
//    [commandEncoder setVertexBytes:&ortho_projection length:sizeof(ortho_projection) atIndex:1];
    RenderEncoderCode::SetVertexBytes(command_encoder,(void*)&ortho_projection,sizeof(ortho_projection),1);
    size_t vertexBufferLength = 0;
    size_t indexBufferLength = 0;
    for (int n = 0; n < drawData->CmdListsCount; n++) 
    {
        const ImDrawList* cmd_list = drawData->CmdLists[n];
        vertexBufferLength += cmd_list->VtxBuffer.Size * sizeof(ImDrawVert);
        indexBufferLength += cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx);
    }

    //TODO(Ray):Use a simpler buffer here no need for all of this.
    //MetalBuffer *vertexBuffer = [self dequeueReusableBufferOfLength:vertexBufferLength device:commandBuffer.device];
    //MetalBuffer *indexBuffer = [self dequeueReusableBufferOfLength:indexBufferLength device:commandBuffer.device];

    RenderEncoderCode::SetRenderPipelineState(command_encoder,render_pipeline_state.state);

    RenderEncoderCode::SetCullMode(command_encoder, cull_mode_none);
    RenderEncoderCode::SetDepthStencilState(command_encoder, &g_sharedMetalContext.depthStencilState);
    RenderEncoderCode::SetVertexBuffer(command_encoder,&buffer[current_buffer],0,0);
    
    size_t vertexBufferOffset = 0;
    size_t indexBufferOffset = 0;
    ImVec2 pos = drawData->DisplayPos;
    
    RenderSynchronization::DispatchSemaphoreWait(&semaphore,YOYO_DISPATCH_TIME_FOREVER);
    
    RenderEncoderCode::AddCompletedHandler(command_buffer, [](void* arg)
    {
       DispatchSemaphoreT* sema = (DispatchSemaphoreT*)arg;
       RenderSynchronization::DispatchSemaphoreSignal(sema);
    }, &semaphore);
    
    for (int n = 0; n < drawData->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = drawData->CmdLists[n];
        ImDrawIdx idx_buffer_offset = 0;
        
        memcpy((char *)buffer[current_buffer].data + vertexBufferOffset, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
        memcpy((char *)gpu_index_buffer[current_buffer].data + indexBufferOffset, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));

        RenderEncoderCode::SetVertexBufferOffset(command_encoder,vertexBufferOffset,0);
        
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                // User callback (registered via ImDrawList::AddCallback)
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                ImVec4 clip_rect = ImVec4(pcmd->ClipRect.x - pos.x, pcmd->ClipRect.y - pos.y, pcmd->ClipRect.z - pos.x, pcmd->ClipRect.w - pos.y);
                if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
                {
                    ScissorRect scissorRect =
                        { (int)clip_rect.x,
                          (int)clip_rect.y,
                          (int)(clip_rect.z - clip_rect.x),
                          (int)(clip_rect.w - clip_rect.y) };
                    RenderEncoderCode::SetScissorRect(command_encoder,scissorRect);
                    // Bind texture, Draw
                    if (pcmd->TextureId != NULL)
                    {
                        Texture tex = {};
                        tex.state = pcmd->TextureId;
                        RenderEncoderCode::SetFragmentTexture(command_encoder,&tex,0);
                    }
                    RenderEncoderCode::DrawIndexedPrimitives(command_encoder,&gpu_index_buffer[current_buffer],primitive_type_triangle,pcmd->ElemCount,sizeof(ImDrawIdx) == 2 ? IndexTypeUInt16 : IndexTypeUInt32,indexBufferOffset + idx_buffer_offset);
                }
            }
            idx_buffer_offset += pcmd->ElemCount * sizeof(ImDrawIdx);
        }
        vertexBufferOffset += cmd_list->VtxBuffer.Size * sizeof(ImDrawVert);
        indexBufferOffset += cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx);
    }
    current_buffer = (current_buffer + 1) % buffer_count;
}

static CFAbsoluteTime g_Time = 0.0;
void ImGui_ImplOSX_NewFrame()
{
    // Setup display size
    ImGuiIO& io = ImGui::GetIO();
    const float dpi = RendererCode::display_scale_factor;//[view.window backingScaleFactor];
    io.DisplaySize = ImVec2(RendererCode::dim.x(),RendererCode::dim.y());//ImVec2((float)view.bounds.size.width, (float)view.bounds.size.height);
    io.DisplayFramebufferScale = ImVec2(dpi, dpi);
    
    // Setup time step
    if (g_Time == 0.0)
    g_Time = CFAbsoluteTimeGetCurrent();
    CFAbsoluteTime current_time = CFAbsoluteTimeGetCurrent();
    io.DeltaTime = current_time - g_Time;
    g_Time = current_time;
}

// Metal Render function.
void ImGui_ImplMetal_RenderDrawData(ImDrawData* draw_data, void* commandBuffer, RenderCommandEncoder* commandEncoder,RenderPassDescriptor* passdesc)
{
    RenderDrawData(draw_data,commandBuffer,commandEncoder,passdesc);
}
#endif
