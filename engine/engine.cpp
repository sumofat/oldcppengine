
//NOTE(Ray):Following this model we will maintain a seperate implementation.
//We want the game to link to the engine as a dll or static lib.

#include "engine.h"
#include "input/input.cpp"

#ifdef ENGINEIMPL

#include "graphics/camera.cpp"
#include "graphics/deffered/deffered.cpp"
#include "external/imgui/imgui.cpp"
#include "external/imgui/imgui_draw.cpp"
#include "external/imgui/imgui_widgets.cpp"

//TODO(Ray):NOTE(Ray):Due to the use of static in the function calls here we need to add a
//a-fno_threadsafe_static call to the engine for reasons unknown revisit later.

#include "external/imgui/imgui_demo.cpp"
#include "graphics/deffered/imguirender.cpp"

#include "graphics/deffered/imgui_impl.cpp"

#include "editor/editor.h"

#include "external/rapidjson/include/rapidjson/document.h"
#include "external/rapidjson/include/rapidjson/writer.h"
#include "external/rapidjson/include/rapidjson/stringbuffer.h"
#include "external/rapidjson/include/rapidjson/prettywriter.h"

#include "asset/AssetSystem.cpp"
#include "asset/MetaFiles.cpp"

extern "C" void gameInit();
extern "C" void gameUpdate();
 
namespace Engine
{
    YoyoVector test_vector;
    bool engine_log = true;
    PlatformState ps = {};

    ModelAsset testmodel;
    RenderMaterial test_material;
    float3 viz_move;
    //NOTE(Ray):Here we init all the engine memory and ints "Subsystems"
    void Init(float2 window_dim)
    {
//1. Set options
        APIFileOptions::data_dir = "/data/";
//2. Init Base systems 
        StringsHandler::Init();
        RenderCache::Init(3000);
        MetaFiles::Init();

        //order of importantce
        //TODO(Ray):FBXSDK inclusion .. set up precompiled headers
        AssetSystem::Init();
        
        //TODO(Ray):Go ahead and set up physx and fmod see how compile times will fair    
        //TODO(Ray):AssetSystem:loading serializing meta prefabs shaders loading etc etc..

        //TODO(Ray):Graphics:Deffered needs some assets first.
        //TODO(Ray):Physx and Audio integration
        //TODO(Ray):Multi threading facilities
        //TODO(Ray):Memory tracking system
        //TODO(Ray):Profiling System
        //TODO(Ray):Networking 
        
        char* name = "dodge_challenger_model.fbx";
//        char* name = "box.fbx";
        // 
        Yostr* path = CreateStringFromLiteral(name, &StringsHandler::transient_string_memory);
        Yostr* buildpath = BuildPathToAssets(&StringsHandler::transient_string_memory,0);
        path = AppendString(*buildpath,*path,&StringsHandler::transient_string_memory);
        
        if(AssetSystem::FBXSDKLoadModel(path->String,&testmodel))
        {
            PlatformOutput(true, "Got Model mesh count:%d \n",testmodel.meshes.count);
            MetaFiles::GetOrCreateDefaultModelMetaFile(path,&testmodel);
            AssetSystem::UploadModelAssetToGPU(&testmodel);            
        }
        //TODO(Ray):Set Asset or file system to hold this 
//        es.base_path_to_data = BuildPathToAssets(&ps->string_state.string_memory, Directory_None);

        //test material
        RenderMaterial mat_result = {};
        mat_result.type = -1;//default opaque
        float4 base_color = float4(1);///float4(bc[0].GetFloat(),bc[1].GetFloat(),bc[2].GetFloat(),bc[3].GetFloat());
        mat_result.inputs.base_color = base_color;

        RenderShader shader;
        RenderShaderCode::InitShaderFromDefaultLib(&shader,"diffuse_vs","diffuse_color_fs");
        ShaderTextureSlot slot;
        slot.material_resource_id = 0;
        shader.texture_slot_count = 1;
        shader.texture_slots[0] = slot;

        RenderPipelineStateDesc render_pipeline_descriptor = RenderEncoderCode::CreatePipelineDescriptor(nullptr,nullptr,0);
        render_pipeline_descriptor.label = "opaque";
        render_pipeline_descriptor.vertex_function = shader.vs_object;
        render_pipeline_descriptor.fragment_function = shader.ps_object;
        render_pipeline_descriptor.depthAttachmentPixelFormat = PixelFormatDepth32Float_Stencil8;
        render_pipeline_descriptor.stencilAttachmentPixelFormat = PixelFormatDepth32Float_Stencil8;
        mat_result.shader = shader;

        //Vertex descriptions and application
        //TODO(Ray):So now we are going to return a vertex descriptor of our own and handle it
        //than set it back and let teh api do the translation for us. Thats the pattern here for pipeline
        //state creation... than we will do serialization to the pipeline desc for
        //pre creation of our pipeline states to save on load times and perfs if need be. but can save that
        //for much later stages.
        VertexDescriptor vertex_descriptor = RenderEncoderCode::NewVertexDescriptor();
        VertexAttributeDescriptor vad;
        vad.format = VertexFormatFloat3;
        vad.offset = 0;
        vad.buffer_index = 0;
        VertexAttributeDescriptor n_ad;
        n_ad.format = VertexFormatFloat3;
        n_ad.offset = 0;
        n_ad.buffer_index = 1;
        VertexAttributeDescriptor uv_ad;
        uv_ad.format = VertexFormatFloat2;
        uv_ad.offset = 0;
        uv_ad.buffer_index = 2;
        VertexBufferLayoutDescriptor vbld;
        vbld.step_function = step_function_per_vertex;
        vbld.step_rate = 1;
        vbld.stride = 12;
        VertexBufferLayoutDescriptor n_bld;
        n_bld.step_function = step_function_per_vertex;
        n_bld.step_rate = 1;
        n_bld.stride = 12;
        VertexBufferLayoutDescriptor uv_bld;
        uv_bld.step_function = step_function_per_vertex;
        uv_bld.step_rate = 1;
        uv_bld.stride = 8;
        RenderEncoderCode::AddVertexDescription(&vertex_descriptor,vad,vbld);
        RenderEncoderCode::AddVertexDescription(&vertex_descriptor,n_ad,n_bld);
        RenderEncoderCode::AddVertexDescription(&vertex_descriptor,uv_ad,uv_bld);
        RenderEncoderCode::SetVertexDescriptor(&render_pipeline_descriptor,&vertex_descriptor);
            
/*
  if(mat_result.type == 1)//transparent set blending
  {
  RenderPipelineColorAttachmentDescriptorArray rpcada = render_pipeline_descriptor.color_attachments;
  RenderPipelineColorAttachmentDescriptor rad = rpcada.i[0];
  rad.writeMask = ColorWriteMaskAll;
  rad.blendingEnabled = true;
  rad.destinationRGBBlendFactor = BlendFactorOneMinusSourceAlpha;
  rad.destinationAlphaBlendFactor = BlendFactorOne;
  rad.sourceRGBBlendFactor = BlendFactorSourceAlpha;
  rad.sourceAlphaBlendFactor = BlendFactorOne;
  render_pipeline_descriptor.color_attachments.i[0] = rad;                    
  }
*/

//Create pipeline states    
        RenderPipelineState pipeline_state = RenderEncoderCode::NewRenderPipelineStateWithDescriptor(render_pipeline_descriptor);
        mat_result.pipeline_state = pipeline_state;

//create depth states            
        DepthStencilDescription depth_desc = RendererCode::CreateDepthStencilDescriptor();
        depth_desc.depthWriteEnabled = true;
        depth_desc.depthCompareFunction = compare_func_less;
        DepthStencilState depth_state = RendererCode::NewDepthStencilStateWithDescriptor(&depth_desc);
        mat_result.depth_stencil_state = depth_state;
            
//            mat_result.texture_slots[mat_result.texture_count] = uploaded_texture;
//            mat_result.texture_count++;                   
        test_material = mat_result;

        //NOTE(Ray):Probably move this to a more renderer specific area

        PlatformOutput(engine_log,"Engine Init Begin\n");
        ps.window.dim = window_dim;
        ps.window.is_full_screen_mode = false;
        //Init Input
        //TODO(Ray):Mouse buttons gamepads keyboard
        EngineInput::PullMouseState(&ps);
        
        //Init Renderer
        Camera::Init();
        DefferedRenderer::Init(&Camera::main, &ps);
        //Init Audio
        //Init Physics
        //Init TIme
        //Init Multithreading
        
        test_vector = YoyoInitVector(1,float2,false);
        test_vector.resize_ratio = 0.5f;
//TODO(Ray):
        //Init Network
        //Init etc..
        PlatformOutput(engine_log,"Engine Init Complete\n");

        gameInit();
        StringsHandler::ResetTransientStrings();
    }

    //TODO(Ray):Fixed update.
    
    //NOTE(Ray):Here we do the updating of everything engine related.
    void Update()
    {
        PlatformOutput(engine_log,"Engine Update Begin\n");
        
        EngineInput::PullMouseState(&ps);

        gameUpdate();

//Game UI
#if 0
    HotNodeState* hot_node_state = &ui->hot_node_state;
    //UI nodes
    UINode* node = &ui->ParentNode;
    hot_node_state->mouse_p = input->mouse.p;
    if(hot_node_state->node && input->keyboard.keys[keys.f].down)
    {
        float4 old_a = hot_node_state->node->node_rect.anchor;
        float4 new_a = float4(0,0,0,0);
        float2 a = input->mouse.uv;
        float4 diff_a = float4(0,0,0,0);
        float2 size = GetRectDim(hot_node_state->node->node_rect);
        //float2 size_uv =  float2(size.x() / ps->window.dim.x(),size.y() / ps->window.dim.y());
        float2 size_uv = size / ps->window.dim;
        float2 half_size = size * 0.5f;
        float2 half_size_uv = float2(half_size.x() / ps->window.dim.x(),half_size.y() / ps->window.dim.y());
        old_a.setX(a.x() - half_size_uv.x());
        old_a.setZ(old_a.x() + size_uv.x());
        old_a.setW(a.y() + half_size_uv.y());
        old_a.setY(old_a.w() - size_uv.y());
        hot_node_state->node->node_rect.anchor = old_a;
    }

    hot_node_state->prev_node = hot_node_state->node;
    hot_node_state->node = 0;
    
    //begin finalize/aggregating game state data
    EvaluateNode(node,sprite_batch,hot_node_state);
    //end finalize/aggregating game state data
#endif
        
//finalize all transforms
   //SceneCode::UpdateSceneBuffer(&current_scene_buffer);

#if 0
       //Draw UI NODES
    DrawNodes(&ui->ParentNode,sprite_batch);
#endif

//Testing rendering and camera moving
    //TODO(Ray):There is a small camera glitch when rotating? 
    viz_move += float3(0.0f,0.0f,0.1f);
    
    ObjectTransform mot;
    mot.p = float3(0,0,0) + viz_move;
    mot.r = axis_angle(float3(1,0,0),90) * axis_angle(float3(0,1,0),180);//quaternion::axis_angle(float3(0,0,1),0);
    mot.s = float3(1);
    YoyoUpdateObjectTransform(&mot);
    float4x4 m_matrix = mot.m;
    
    float3 cam_p = float3(-15,15,15);// + viz_move;
    float3 new_p = cam_p;//float3(sin(radians(dummy_inc)) * -5,0,sin(radians(dummy_inc)) * -2) + render_cam_p;
    float3 look_dir = mot.p - new_p;//model_ot[0].p - new_p;
    ObjectTransform cam_ot;
    cam_ot.p = new_p;//render_cam_p;
    cam_ot.r = quaternion::look_rotation(-look_dir,float3(0,1,0));//quaternion::identity();//axis_angle(float3(0,0,1),90);//
    cam_ot.s = float3(1);
    Camera::main.matrix = YoyoSetCameraView(&cam_ot);//set_camera_view(cam_p, float3(0,0,1), float3(0,1,0));
    //model has a link to MeshAsset/Renderer we for ever sceneobject using model asset flatten out meshassets
    //the renderer
//TODO(Ray):Coarse grain render bound on scene cpu based culling here.
//    for(int i = 0;i < it_count;++i) 
    {
        ModelAsset* model = &testmodel;//(ModelAsset*)AssetSystem::runtime_assets.base + 0;
        //NOTE(Ray)://TODO(Ray):Here is where you would split out your meshes to the renderer
        //based on what commandlist you want them in.
        for(int j = 0;j < model->meshes.count;++j)
        {
            MeshAsset* mesh = (MeshAsset*)model->meshes.base + j;
            if(GPUResourceCache::DoesGPUResourceExist(mesh))
            {
                GPUMeshResource* mr = GPUResourceCache::GetGPUResource(mesh);
                //Pass game data to renderer
                //Add a render command to the render command buffer.
                //For every mesh in a model generate a render command
                //one command represents a renderable
                RenderWithMaterialCommand command_with_material;
                command_with_material.material     = test_material;
                command_with_material.model_matrix = m_matrix;//float4x4::identity();//scene_obj.m;
                command_with_material.uniforms     = DefferedRenderer::uniform_buffer;
                command_with_material.resource     = *mr;
                //            if(mesh->material.type == 1)
                {
                    //                RenderCommandCode::AddRenderCommand(transparent_command_buffer, (void*)&command_with_material);
                }
                //            else
                {
                    RenderCommandCode::AddRenderCommand(DefferedRenderer::gbufferpass.pass_command_buffer, (void*)&command_with_material);
                }
            }
        }
    }
        
#if TARGET_OS_OSX
    ImGui_ImplOSX_NewFrame();
#endif
    ImGui::NewFrame();
        
    Editor::Update();
        
    // Pre render data preparation.
    ImGui::Render();
        
    //TODO(ray):Find a place to give this a more rigourous test.
    //PlatformOutput(engine_log,"Test vec size %d \n",test_vector.max_size);
    //float2 newf2 = float2(100,100);
    //YoyoStretchPushBack(&test_vector,newf2);
        
    //Update Graphics
    DefferedRenderer::ExecutePasses();
    //Update Audio
    //Update Physics
    PlatformOutput(engine_log,"Engine Update Complete\n");

    StringsHandler::ResetTransientStrings();
    }
};

#endif
