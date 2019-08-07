
//NOTE(Ray):Following this model we will maintain a seperate implementation.
//We want the game to link to the engine as a dll or static lib.

#include "engine.h"
#include "input/input.cpp"

#ifdef ENGINEIMPL

#include "graphics/camera.cpp"
#include    "graphics/deffered/deffered.cpp"
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
#include "physics/physics.cpp"
#include "audio/SoundInterface.cpp"

extern "C" void gameInit();
extern "C" void gameUpdate();
 
namespace Engine
{
    YoyoVector test_vector;

    bool engine_log = false;
    bool renderer_log = false;
    bool asset_log = true;
    
    PlatformState ps = {};

    ModelAsset testmodel;
    RenderMaterial test_material;
    RenderMaterial test_quad_material;
    
    float3 viz_move;
    bool is_init = false;
    //NOTE(Ray):Here we init all the engine memory and ints "Subsystems"

    SceneBuffer scene_buffer;
    Scene* default_empty_scene;

    AnythingCache material_cache;
    
    void Init(float2 window_dim)
    {
        PlatformOutput(engine_log,"Engine Init Begin\n");
        RendererCode::SetGraphicsOutputLog(false);
        EngineInput::log = false;
//1. Set options
        APIFileOptions::data_dir = "/data/";
//2. Init Base systems 
        StringsHandler::Init();
        RenderCache::Init(3000);
        //MaterialCache::Init(3000);
        AnythingCacheCode::Init(&material_cache,3000,sizeof(RenderMaterial),sizeof(uint64_t));
        MetaFiles::Init();
        SceneCode::InitScene(&scene_buffer,10);
        default_empty_scene = SceneCode::CreateEmptyScene(&scene_buffer);

        //Physics
        PhysicsCode::Init();

        //Audio
        SoundCode::Init();
        SoundAssetCode::Init();
        SoundCode::SetDefaultListener();
        
//order of importantce
        //TODO(Ray):FBXSDK inclusion .. set up precompiled headers
        AssetSystem::Init();

//NOTE(Ray):Physx and Audio integration
//NOTE(Ray):we have  set up physx and fmod see how compile times will dont change much but suffer for other reasons.

        //TODO(Ray):AssetSystem:loading serializing meta prefabs shaders loading etc etc..
        //TODO(Ray):Graphics:Deffered needs some assets first.
        //TODO(Ray):Multi threading facilities
        //TODO(Ray):Memory tracking system
        //TODO(Ray):Profiling System
        //TODO(Ray):Networking 

        //NOTE(Ray):Here we are test loading a scene and instanting object based on scene description
        //loading them into a sence hierarchy and than we will test grab a ref to the objects via some lookup
        //api.
#if 1
        Yostr test_scene_meta_file = MetaFiles::GetMetaFile(CreateStringFromLiteral("scene_example", &StringsHandler::transient_string_memory));
        Document sdoc;
        sdoc.Parse((char*)test_scene_meta_file.String);
        if(!sdoc.IsObject())
        {
            //Error handling
            Assert(false);
        }
 
        const Value& scene_name = sdoc["name"];
        PlatformOutput(asset_log,"processing metafile %s\n",scene_name.GetString());
        const Value& nodes = sdoc["nodes"];
        for (auto& node : nodes.GetArray())
        {
            const Value& node_name = node["name"];
            PlatformOutput(asset_log,"processing node %s\n",node_name.GetString());

            const Value& p_ = node["p"];
            const Value& s_ = node["s"];
            const Value& r_ = node["r"];
            float3 p = float3(p_["x"].GetFloat(),p_["y"].GetFloat(),p_["z"].GetFloat());
            float3 s = float3(s_["x"].GetFloat(),s_["y"].GetFloat(),s_["z"].GetFloat());
            quaternion r = quaternion(r_["x"].GetFloat(),r_["y"].GetFloat(),r_["z"].GetFloat(),r_["w"].GetFloat());
            //SceneObjectCode::AddSceneObject(&scene_buffer,p,s,r);

            PlatformOutput(asset_log,"root_node p %f %f %f\n",p.x(),p.y(),p.z());
            PlatformOutput(asset_log,"root_node r %f %f %f %f\n",r.x(),r.y(),r.z(),r.w());
            PlatformOutput(asset_log,"root_node s %f %f %f\n",s.x(),s.y(),s.z());

            const Value& child_nodes = node["nodes"];
            //if(child_nodes.GetCount() > 0)
            //if(child_nodes)
            {
                for (auto& child_node : child_nodes.GetArray())
                {
                    const Value& child_node_name = child_node["name"];
                    PlatformOutput(asset_log,"processing node %s\n",child_node_name.GetString());

                    const Value& cp_ = node["p"];
                    const Value& cs_ = node["s"];
                    const Value& cr_ = node["r"];
                    float3 cp = float3(cp_["x"].GetFloat(),cp_["y"].GetFloat(),cp_["z"].GetFloat());
                    float3 cs = float3(cs_["x"].GetFloat(),cs_["y"].GetFloat(),cs_["z"].GetFloat());
                    quaternion cr = quaternion(cr_["x"].GetFloat(),cr_["y"].GetFloat(),cr_["z"].GetFloat(),cr_["w"].GetFloat());
                    //AddChildToSceneObject(parent_index,&scene_buffer,cp,cs,cr);

                    PlatformOutput(asset_log,"root_node p %f %f %f\n",p.x(),p.y(),p.z());
                    PlatformOutput(asset_log,"root_node r %f %f %f %f\n",r.x(),r.y(),r.z(),r.w());
                    PlatformOutput(asset_log,"root_node s %f %f %f\n",s.x(),s.y(),s.z());
                }
            }
        }                
#endif
        
        char* name = "dodge_challenger_model.fbx";
        Yostr path = CreateStringFromLiteral(name, &StringsHandler::transient_string_memory);
        Yostr buildpath = BuildPathToAssets(&StringsHandler::transient_string_memory,0);
        path = AppendString(buildpath,path,&StringsHandler::transient_string_memory);
        
        if(AssetSystem::FBXSDKLoadModel(path.String,&testmodel))
        {
            
            PlatformOutput(true, "Got Model mesh count:%d \n",testmodel.meshes.count);
            Yostr model_meta_file = MetaFiles::GetOrCreateDefaultModelMetaFile(path,&testmodel);

// 1. Parse a JSON string into DOM.
            Document d;
            d.Parse((char*)model_meta_file.String);
            if(!d.IsObject())
            {
                //Error handling
                Assert(false);
            }
 
            const Value& metamodelname = d["model_file"];
            PlatformOutput(asset_log,"processing metafile %s\n",metamodelname.GetString());
//            Yostr* r = CreateStringFromLiteralConst(d["Meshes"].GetString(),&StringsHandler::transient_string_memory);
//            Yostr* final_path = AppendString(*base_path_to_data, *CreateStringFromLiteral(r->String,&ps->string_state.transient_string_memory), &StringsHandler::transient_string_memory);
            // 
        //After getting model get material definition from disk
/*
        Entity* entity = PushStruct(&EntitySytem::runtime_entities, Entity);
        entity->ot.p = float3(0, 0, 0);
        entity->ot.s = float3(1, 1, 1);
        entity->ot.r = axis_angle(float3(0, 1, 0), 0);
        ModelAsset *new_model_asset = PushStruct(&runtime_assets, ModelAsset);
        entity->asset = new_model_asset;
        LoadedModelToModelAsset(loaded_model, new_model_asset);
        //TODO(Ray):Why does this fail sometimes!!
        UploadModelAssetToGPU(new_model_asset);
        
            Texture uploaded_texture;
            RenderShader shader;
            const Value& materials = d["Materials"];
            int mesh_index = 0;
*/

//This next part gets and loads all the relevent information to render this object properly.
            //NOTE(Ray):
            //1. We dont have any good defaults or ways to generate this file from a newly imported mesh.
            //2. We will fail if the mesh is added to or removed need to have some mapping.
            //3. and add remove materials textures etc base on last use.
            //4. PSO's should also be allowed to be stored offline for immediate async creation upon startup
            //or at the users discretion.
            uint32_t mesh_index = 0;
            const Value& meshes = d["Meshes"];            
            for (auto& mesh : meshes.GetArray())
            {
                MeshAsset* rendermesh = (MeshAsset*)testmodel.meshes.base + mesh_index++;
                
                const Value& meshname = mesh["meshname"];
                PlatformOutput(asset_log,"mesh name%s\n",meshname.GetString());
            
                const Value& materials = mesh["materials"];            
                for (auto& material : materials.GetArray())
                {
                    const Value& material_name = material["material_name"];                    
                    PlatformOutput(asset_log,"material name %s\n",material_name.GetString());

                    uint32_t strlen = material_name.GetStringLength();
                    Yostr matstring;
                    matstring.Length = strlen;
                    matstring.String = (char*)material_name.GetString();
                    matstring.NullTerminated = true;
                    RenderMaterial render_material;
                    Yostr vs_name;
                    Yostr fs_name;
                    float4 base_color_input;

                    uint64_t mat_key = StringsHandler::StringHash(matstring.String,matstring.Length);
                    if(AnythingCacheCode::DoesThingExist(&material_cache,&mat_key))
                    {
                        render_material = *(RenderMaterial*)AnythingCacheCode::GetThing(&material_cache,&mat_key);//MaterialCache::GetMaterial(&matstring);
                        rendermesh->r_material = render_material;
                    }
                    else
                    {
//                        MaterialCache::AddMaterial(&matstring,&render_material);
                        AnythingCacheCode::AddThing(&material_cache,&mat_key,&render_material);
                        //Getmaterial file
                        Yostr meta_file_json = MetaFiles::GetMetaFile(matstring);
                        //Create RenderMaterial based on this and add it to the cache
                        PlatformOutput(asset_log,"Processing meta file");
// 1. Parse a JSON string into DOM.
                        Document rd;
                        rd.Parse((char*)meta_file_json.String);
                        if(!rd.IsObject())
                        {
                            //Error handling
                            Assert(false);
                        }
                        const Value& meta_mat_name = rd["material_name"];
                        PlatformOutput(asset_log,"processing metafile %s\n",meta_mat_name.GetString());
                        
                        const Value& shaders = rd["shaders"];
                        for (auto& shader : shaders.GetArray())
                        {

                            const Value& shader_type = shader["type"];                    
                            PlatformOutput(asset_log,"shader type %s\n",shader_type.GetString());
                            Yostr shader_type_string = {};
                            shader_type_string.Length = shader_type.GetStringLength();
                            shader_type_string.String = (char*)shader_type.GetString();
                            shader_type_string.NullTerminated = true;
                            
                            const Value& shader_name = shader["name"];
                            PlatformOutput(asset_log,"shader name %s\n",shader_name.GetString());

                            if(Compare(shader_type_string, CreateStringFromLiteral("vertex", &StringsHandler::transient_string_memory)))
                            {
                                vs_name.Length = shader_name.GetStringLength();
                                vs_name.NullTerminated = true;
                                vs_name.String = (char*)shader_name.GetString();
                            }
                            else if(Compare(shader_type_string, CreateStringFromLiteral("fragment", &StringsHandler::transient_string_memory)))
                            {
                                fs_name.Length = shader_name.GetStringLength();
                                fs_name.NullTerminated = true;
                                fs_name.String = (char*)shader_name.GetString();
                            }

                            const Value& input_slots = shader["input_slot"];
                            
                            for (auto& input_slot : input_slots.GetArray())
                            {
                                const Value& input_slot_name = input_slot["name"];                    
                                PlatformOutput(asset_log,"shader name %s\n",input_slot_name.GetString());

                                const Value& input_slot_type = input_slot["type"];                    
                                PlatformOutput(asset_log,"shader name %s\n",input_slot_type.GetString());
                            }
                        }
                    }
                     
                    const Value& inputs = material["inputs"];
                    for (auto& input : inputs.GetArray())
                    {
                        const Value& input_name = input["name"];                    
                        PlatformOutput(asset_log,"input name %s\n",input_name.GetString());

                        uint32_t instrlen = input_name.GetStringLength();
                        Yostr input_name_string;
                        input_name_string.Length = instrlen;
                        input_name_string.String = (char*)input_name.GetString();
                        input_name_string.NullTerminated = true;
                        
                        const Value& type = input["type"];
                        if(!type.IsString())Assert(false);
                        uint32_t strlen = type.GetStringLength();
                        Yostr typestring;
                        typestring.Length = strlen;
                        typestring.String = (char*)type.GetString();
                        typestring.NullTerminated = true;
                        ShaderValueType::Type value_type = MetaFiles::GetShaderType(typestring);

                        //TODO(Ray):For the time being we only have one float4 it is base color...
                        //but will want to make this more flexible later
                        if(value_type == ShaderValueType::float4)
                        {
                            float results[4];
                            const Value& values = input["value"];
                            int value_index = 0;
                            for (auto& value : values.GetArray())
                            {
                                if(!value.IsDouble() || value_index > 3)
                                {
//Must be a float here if not better chweck whqt went wrong                                      
                                    Assert(false);
                                }
                                results[value_index] = (float)value.GetDouble();
                                value_index++;
                            }
                            float4 result = float4(results[0],results[1],results[2],results[3]);
                            PlatformOutput(asset_log,"result float4 %f %f %f %f\n",result.x(),result.y(),result.z(),result.w());
                            render_material.inputs.base_color = result;
                            base_color_input = result;
                        }
                        
                        RenderMaterial final_mat = AssetSystem::CreateMaterialFromDescription(&vs_name,&fs_name,base_color_input);
                        rendermesh->r_material = final_mat;
/*
                        if(value_type == ShaderValueType::afloat)
                        {
                            float result;
                            const Value& values = input["value"];

                            if(!value.IsDouble())
                            {                            
//Must be a float here if not better chweck whqt went wrong                                      
                                Assert(false);
                            }
                            results = (float)value.GetDouble();
                            float result = float(result);
                            
                            PlatformOutput(asset_log,"result float4 %f %f %f %f\n",result.x(),result.y(),result.z(),result.w());
                            if(Compare(input_name_string,CreateStringFromLiteral("specular"),&StringsHandler::transient_string_memory))
                            {
                                render_material.inputs.specular = result;
                            }
                            else if(Compare(input_name_string,CreateStringFromLiteral("metallic"),&StringsHandler::transient_string_memory))
                            {
                                render_material.inputs.metallic = result;                                    
                            }
                            else if(Compare(input_name_string,CreateStringFromLiteral("roughness"),&StringsHandler::transient_string_memory))
                            {
                                render_material.inputs.roughness = result;
                            }
                        }
                        */
                    }
                }
            }            
        }

        /*
         if(mesh_index > new_model_asset->mesh_count - 1)Assert(false);//break;//TODO(Ray):materials need validation
         //Load materials for each mesh index the materials should be in mesh index order for now
         MeshAsset* ma = new_model_asset->meshes + mesh_index;// YoyoGetVectorElement(MeshAsset,&,i);
         if(ma)
         {
         RenderMaterial mat_result = {};
         
         auto s = material["name"].GetString();
         
         mat_result.type = -1;//default opaque
         if(material.HasMember("type"))
         {
         auto type = material["type"].GetInt();
         mat_result.type = (int)type;
         }
         
         const Value& bc = material["base_color"];
         float4 base_color = float4(bc[0].GetFloat(),bc[1].GetFloat(),bc[2].GetFloat(),bc[3].GetFloat());
         mat_result.inputs.base_color = base_color;
         
         int j = 0;
         const Value& texture_array = material["textures"];
         
         for (auto& texture_value : texture_array.GetArray())
         {
         Yostr* rrr = CreateStringFromLiteralConst(texture_value.GetString(),&ps->string_state.transient_string_memory);
         Yostr* tex_path = AppendString(*base_path_to_data, *CreateStringFromLiteral(rrr->String,&ps->string_state.transient_string_memory), &ps->string_state.transient_string_memory);
         if(!YoyoHashContains(&texture_hash,rrr->String,rrr->Length))
         {
         LoadedTexture loaded_tex;
         GetImageFromDisk(tex_path->String,&loaded_tex);
         uploaded_texture = UploadTextureToGPU(&loaded_tex);
         YoyoPushBack(&loaded_textures,uploaded_texture);
         Texture* tex_pointer = YoyoPeekVectorElement(Texture,&loaded_textures);
         YoyoAddElementToHashTable(&texture_hash,rrr->String,rrr->Length,tex_pointer);
         }
         else
         {
         uploaded_texture = *YoyoGetElementByHash(Texture,&texture_hash,rrr->String,rrr->Length);
         }
         mat_result.texture_slots[mat_result.texture_count] = uploaded_texture;
         mat_result.texture_count++;
         j++;
         }
         
         const Value& value = material["shader"];
         const Value& vs_value = value["vs"];
         const Value& ps_value = value["ps"];
         const Value& slots = value["slots"];
         
         RenderShaderCode::InitShaderFromDefaultLib(&shader,(char*)vs_value.GetString(),(char*)ps_value.GetString());
         
         ShaderTextureSlot slot;
         slot.material_resource_id = 0;
         shader.texture_slot_count = 1;
         shader.texture_slots[0] = slot;
         
         //Render pipeline descriptors init and setup based on material definitions
         RenderPipelineStateDesc render_pipeline_descriptor = RenderEncoderCode::CreatePipelineDescriptor(nullptr,nullptr,0);
         render_pipeline_descriptor.label = "test";
         render_pipeline_descriptor.vertex_function = shader.vs_object;
         render_pipeline_descriptor.fragment_function = shader.ps_object;
         render_pipeline_descriptor.depthAttachmentPixelFormat = PixelFormatDepth32Float_Stencil8;
         render_pipeline_descriptor.stencilAttachmentPixelFormat = PixelFormatDepth32Float_Stencil8;
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
         
         //Create pipeline states
         RenderPipelineState pipeline_state = RenderEncoderCode::NewRenderPipelineStateWithDescriptor(render_pipeline_descriptor);
         mat_result.pipeline_state = pipeline_state;
         
         //create depth states
         DepthStencilDescription depth_desc = RendererCode::CreateDepthStencilDescriptor();
         depth_desc.depthWriteEnabled = true;
         depth_desc.depthCompareFunction = compare_func_less;
         DepthStencilState depth_state = RendererCode::NewDepthStencilStateWithDescriptor(&depth_desc);
         mat_result.depth_stencil_state = depth_state;
         
         mat_result.shader = shader;
         ma->material = mat_result;
         mesh_index++;
         */
        AssetSystem::UploadModelAssetToGPU(&testmodel);
        
        //TODO(Ray):Set Asset or file system to hold this 
//        es.base_path_to_data = BuildPathToAssets(&ps->string_state.string_memory, Directory_None);

//TODO(ray):Lots to do here getting sleepy.
        test_material = AssetSystem::CreateDefaultMaterial();

        ps.window.dim = window_dim;
        ps.window.is_full_screen_mode = false;
        //Init Input
        //TODO(Ray):Mouse buttons gamepads keyboard
        //EngineInput::PullMouseState(&ps);
        
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
        is_init = true;
    }

    //TODO(Ray):Fixed update.
    
    //NOTE(Ray):Here we do the updating of everything engine related.
    void Update()
    {
        if(!is_init)return;
        PlatformOutput(engine_log,"Engine Update Begin\n");
        EngineInput::PullMouseState(&ps);
        gameUpdate();
        
        SoundCode::Update();
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
    
    float3 cam_p = float3(-3,0.4f,60);// + viz_move;
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
                command_with_material.material     = mesh->r_material;//test_material;
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
