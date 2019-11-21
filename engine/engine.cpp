
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

#include "editor/editor.cpp"
#include "editor/editorgui.cpp"

#include "external/rapidjson/include/rapidjson/document.h"
#include "external/rapidjson/include/rapidjson/writer.h"
#include "external/rapidjson/include/rapidjson/stringbuffer.h"
#include "external/rapidjson/include/rapidjson/prettywriter.h"

#define CGLTF_IMPLEMENTATION
#include "metalizer/cgltf/cgltf.h"

#include "asset/AssetSystem.cpp"
#include "asset/MetaFiles.cpp"
#include "physics/physics.cpp"
#include "audio/SoundInterface.cpp"

#include "asset/JSONHelper.h"

extern "C" void gameInit();
extern "C" void gameUpdate();
#define PDM_EDITOR 1 
namespace Engine
{
    bool engine_log = false;
    bool renderer_log = false;
    bool asset_log = true;
    
    PlatformState ps = {};

    ModelAsset testmodel;
    
    float3 viz_move;
    bool is_init = false;

    SceneBuffer scene_buffer;
    Scene* default_empty_scene;

#if PDM_EDITOR
    bool debug_cam_mode = false;
    float2 cam_pitch_yaw;
    ObjectTransform debug_cam_ot;
#endif
    uint64_t core_count;    
    TicketMutex asset_ticket_mutex;
    YoyoVector threads;
    u64 fake_thread_id = 0;

    void* LoadAssetAsync(void* data)
    {
        u64 v = (u64)data;
        YoyoThread* t = YoyoGetVectorElement(YoyoThread,&threads,v);
        PlatformOutput(true,"Thread started threadid: %lu \n",t->id);
        BeginTicketMutex(&asset_ticket_mutex);
        PlatformOutput(true,"Thread executed threadid: %lu \n",t->id);
        EndTicketMutex(&asset_ticket_mutex);
        PlatformOutput(true,"Thread is leaving threadid: %lu \n",t->id);
        return 0;
    }
    
   
    void Init(float2 window_dim)
    {
#if 1
        core_count = GetLogicalCPUCoreCount();
        threads = YoyoInitVector(core_count,sizeof(YoyoThread),false);
        for(int i = 0;i < core_count - 2;++i)
        {
            YoyoThread thread = {};
            thread.id = fake_thread_id++;
            u64 index = YoyoPushBack(&threads,thread);
            YoyoCreateThread(LoadAssetAsync,(void*)index);
        }
#endif
        
#if PDM_EDITOR
        cam_pitch_yaw = float2(0.0f);
#endif
        
        PlatformOutput(engine_log,"Engine Init Begin\n");
        RendererCode::SetGraphicsOutputLog(false);
        EngineInput::log = false;
//Set Global options
        APIFileOptions::data_dir = "/data/";
//Init Base systems 
        StringsHandler::Init();
        RenderCache::Init(3000);

//Metafiles and other misc
        MetaFiles::Init();
        SceneCode::InitScene(&scene_buffer,10);
        default_empty_scene = SceneCode::CreateEmptyScene(&scene_buffer);

        //Physics
        PhysicsCode::Init();

        //Audio
        SoundCode::Init();
        SoundAssetCode::Init();
        SoundCode::SetDefaultListener();

        //Assets
        AssetSystem::Init();

        ogle_init(&DefferedRenderer::ogl_test_state);
        VertexDescriptor vd = ogle_create_default_vert_desc();

        //TODO(Ray):AssetSystem/MetaFiles:
        /*
          Load scenes from meta file
          Asset preprocessing
        */
        
        //TODO(Ray):Networking 
        //TODO(Ray):Memory tracking / profiling
        //TODO(Ray):Performance profiling
        
        //NOTE(Ray):Here we are test loading a scene and instanting object based on scene description
        //loading them into a sence hierarchy and than we will test grab a ref to the objects via some lookup
        //api.

#if 0
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

        char* gltfname = "littlest-tokyo/source/littlest_tokyo.glb";
        Yostr gltfpath = CreateStringFromLiteral(gltfname, &StringsHandler::transient_string_memory);
        Yostr gltfbuildpath = BuildPathToAssets(&StringsHandler::transient_string_memory,0);
        gltfpath = AppendString(gltfbuildpath,gltfpath,&StringsHandler::transient_string_memory);
        uint64_t model_key = StringsHandler::StringHash(gltfpath.String,gltfpath.Length);        
        if(AnythingCacheCode::DoesThingExist(&AssetSystem::render_asset_cache,&model_key))
        {
            testmodel = GetThingCopy(&AssetSystem::render_asset_cache,&model_key,ModelAsset);
        }
        else
        {
            if(AssetSystem::GLTFLoadModel(gltfpath.String,&testmodel))
            {
                AnythingCacheCode::AddThing(&AssetSystem::render_asset_cache,&model_key,&testmodel);            
                PlatformOutput(true, "Got Model mesh count:%d \n",testmodel.meshes.count);
                Yostr model_meta_file = MetaFiles::GetOrCreateDefaultModelMetaFile(gltfpath,&testmodel);

                Document d;
                d.Parse((char*)model_meta_file.String);
                if(!d.IsObject())
                {
                    //Error handling
                    Assert(false);
                }
 
                const Value& metamodelname = d["model_file"];
                PlatformOutput(asset_log,"processing metafile %s\n",metamodelname.GetString());
            
//Retrieve and load all the relevent information to render this model properly.
                uint32_t mesh_index = 0;
                const Value& meshes = d["Meshes"];            
                for (auto& mesh : meshes.GetArray())
                {

                    const Value& meshname = mesh["meshname"];
                    PlatformOutput(asset_log,"mesh name%s\n",meshname.GetString());
                    
                    Yostr vs_name = {};
                    Yostr fs_name = {};
                    //NOTE(Ray):Should only be one material per mesh
                    const Value& materials = mesh["materials"];

                    for (auto& material : materials.GetArray())
                    {
                        const Value& material_name = material["material_name"];
                        PlatformOutput(asset_log,"material name %s\n",material_name.GetString());

                        const Value& index_value = material["index"];
                        uint32_t m_i = (uint32_t)index_value.GetInt();
                        MeshAsset* rendermesh = (MeshAsset*)testmodel.meshes.base + m_i;
                        rendermesh->r_material = AssetSystem::default_mat;
                        Assert(CompareChars(rendermesh->name.String, meshname.GetString()));
                        uint32_t strlen = material_name.GetStringLength();
                        Yostr matstring;
                        matstring = JSONHelper::GetString(material_name);

                        RenderMaterial render_material = {};
                        uint64_t mat_key = StringsHandler::StringHash(matstring.String,matstring.Length);
                        //if(AnythingCacheCode::DoesThingExist(&AssetSystem::material_cache,&mat_key))
                        {
                            //render_material = *(RenderMaterial*)AnythingCacheCode::GetThing(&AssetSystem::material_cache,&mat_key);
                            render_material = AssetSystem::default_mat;
                            const Value& inputs = material["inputs"];
                            for (auto& input : inputs.GetArray())
                            {
                                const Value& input_name = input["name"];
                                PlatformOutput(asset_log,"input name %s\n",input_name.GetString());
                            
                                Yostr input_name_string = JSONHelper::GetString(input_name);
                                const Value& type = input["type"];
                                if(!type.IsString())Assert(false);
                                Yostr typestring = JSONHelper::GetString(type);
                            
                                ShaderValueType::Type value_type = MetaFiles::GetShaderType(typestring);
                                if(value_type == ShaderValueType::float4)
                                {
                                    const Value& values = input["value"];
                                    float4 result = JSONHelper::GetFloat4(values);
                                    PlatformOutput(asset_log,"result float4 %f %f %f %f\n",result.x(),result.y(),result.z(),result.w());
                                    render_material.inputs.base_color = result;
                                }

                                if(value_type == ShaderValueType::texture)
                                {
                                    const Value& values = input["value"];
                                    const Value& texcoord_v = input["texcoord"];
                                    float texcoord = texcoord_v.GetDouble();
                                    Yostr path = JSONHelper::GetString(values);
                                    PlatformOutput(asset_log,"result texture %s\n",path.String);
                                    LoadedTexture tex = {};
                                    tex.texcoord = texcoord;
                                    if(texcoord != 0)
                                    {
                                        int a = 0;
                                    }
                                    GLTexture gl_tex= {};
                                    if(AssetSystem::AddOrGetTexture(path,&tex,&gl_tex))
                                    {
                                        render_material.texture_slots[render_material.texture_count] = tex.texture;
                                        render_material.gl_tex_slots[render_material.texture_count] = gl_tex;
                                        
                                        render_material.shader.texture_slots[0].texcoord_index = texcoord;
                                    }
                                    render_material.texture_count++;
                                }
                            
                                if(value_type == ShaderValueType::afloat)
                                {
                                    const Value& values = input["value"];
                                    float result = JSONHelper::GetFloat(values);
                                    if(CompareCharToChar(input_name_string.String,"metallic_factor",100))
                                    {
                                        PlatformOutput(asset_log,"metallic_factor %f\n",result);
                                        render_material.inputs.metallic_factor = result;
//                                    base_color_input = result;                                                                    
                                    }
                                    else if(CompareCharToChar(input_name_string.String,"roughness_factor",100))
                                    {
                                        PlatformOutput(asset_log,"roughness_factor %f\n",result);
                                        render_material.inputs.roughness_factor = result;
                                        //                                  base_color_input = result;                                                                    
                                    }
                                }
                                if(value_type == ShaderValueType::texture)
                                {
                                }
                            }
                            const Value& options = material["options"];
                            const Value& ds_obj = options["double_sided"];
                            bool ds = ds_obj.GetBool();
                            render_material.double_sided = ds;
                            rendermesh->r_material = render_material;
                        }

#if 0
                        else
                        {
                            //Getmaterial file
                            Yostr meta_file_json = MetaFiles::GetMetaFile(matstring);
                            //Create RenderMaterial based on this and add it to the cache
                            PlatformOutput(asset_log,"Processing meta file");
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
                                Yostr shader_type_string = JSONHelper::GetString(shader_type);
                            
                                const Value& shader_name = shader["name"];
                                PlatformOutput(asset_log,"shader name %s\n",shader_name.GetString());

                                if(Compare(shader_type_string, CreateStringFromLiteral("vertex", &StringsHandler::transient_string_memory)))
                                {
                                    vs_name = JSONHelper::GetString(shader_name);
                                }
                                else if(Compare(shader_type_string, CreateStringFromLiteral("fragment", &StringsHandler::transient_string_memory)))
                                {
                                    fs_name = JSONHelper::GetString(shader_name);
                                }
                                else
                                {
                                    Assert(false);//Not supported
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
                        
                            const Value& inputs = material["inputs"];
                            for (auto& input : inputs.GetArray())
                            {
                                const Value& input_name = input["name"];
                                PlatformOutput(asset_log,"input name %s\n",input_name.GetString());
                            
                                Yostr input_name_string = JSONHelper::GetString(input_name);
                                const Value& type = input["type"];
                                if(!type.IsString())Assert(false);
                                Yostr typestring = JSONHelper::GetString(type);
                            
                                ShaderValueType::Type value_type = MetaFiles::GetShaderType(typestring);
                                //TODO(Ray):For the time being we only have one float4 it is base color...
                                //but will want to make this more flexible later
                                if(value_type == ShaderValueType::float4)
                                {
                                    const Value& values = input["value"];
                                    float4 result = JSONHelper::GetFloat4(values);
                                    PlatformOutput(asset_log,"result float4 %f %f %f %f\n",result.x(),result.y(),result.z(),result.w());
                                    render_material.inputs.base_color = result;
                                    //sbase_color_input = result;
                                }
                            
                                RenderMaterial base_mat = AssetSystem::CreateMaterialFromDescription(&vs_name,&fs_name,base_color_input);
                                render_material.shader = base_mat.shader;
                                render_material.pipeline_state = base_mat.pipeline_state;
                                render_material.depth_stencil_state = base_mat.depth_stencil_state;
                                rendermesh->r_material = render_material;

                                AnythingCacheCode::AddThing(&AssetSystem::material_cache,&mat_key,&render_material);
                            }
                                }
#endif                    
                    }
                }            
            }
        }
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
        DefferedRenderer::diffuse_program = ogle_add_prog_lib(&DefferedRenderer::ogl_test_state,"diffuse_vs","diffuse_fs",vertex_descriptor);
                                
        AssetSystem::UploadModelAssetToOpenGLEmuState(&DefferedRenderer::ogl_test_state,&testmodel);
//        AssetSystem::UploadModelAssetToGPUTest(&testmodel);

//TODO(Ray):Set Asset or file system to hold this 
//        es.base_path_to_data = BuildPathToAssets(&ps->string_state.string_memory, Directory_None);


        ps.window.dim = window_dim;
        ps.window.is_full_screen_mode = false;
        
        //Init Renderer
        Camera::Init();
        DefferedRenderer::Init(&Camera::main, &ps);

//TODO(Ray):
        //Init Network
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
#ifdef OSX
        EngineInput::PullMouseState(&ps);
#endif
        DefferedRenderer::PreframeSetup();
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

    Input* input = &ps.input;
            
    ObjectTransform mot;
    mot.p = float3(0,0,-50);// + viz_move;
    mot.r = axis_angle(float3(1,0,0),0) * axis_angle(float3(0,1,0),180);
    mot.s = float3(1.0f);
    YoyoUpdateObjectTransform(&mot);
    float4x4 m_matrix = mot.m;
    
    float3 cam_p = float3(0,0,0);
    float3 new_p = cam_p;
    float3 look_dir = mot.p - new_p;
    ObjectTransform cam_ot;
    cam_ot.p = new_p;
    cam_ot.r = quaternion::look_rotation(-look_dir,float3(0,1,0));
    cam_ot.s = float3(1);

    Camera::main.ot = cam_ot;
    Camera::main.matrix = YoyoSetCameraView(&cam_ot);//set_camera_view(cam_p, float3(0,0,1), float3(0,1,0));
    
#if PDM_EDITOR

    if (input->keyboard.keys['q'].released)
        debug_cam_mode = !debug_cam_mode;
    if(debug_cam_mode)
    {
        //Check if we are looking for selection.
		if (input->mouse.lmb.down)
		{
#if 0
            if(selected_gizmo == nullptr)
			{
				//Dragging the selected object along the forward direction of the gizmo
				switch (closest_gizmo_info.index)
				{
				case 0://up
				{
					es.selected_gizmo = &as->up;
					es.selected_gizmo_index = 0;
				}break;
				case 1://right
				{
					es.selected_gizmo = &as->right;
					es.selected_gizmo_index = 1;
				}break;
				case 2://forward
				{
					es.selected_gizmo = &as->forward;
					es.selected_gizmo_index = 2;
				}break;
				default:
				{
					//Assert(false);
				}
				}
			}
#endif
        }
        
        float move_speed = 50.0f;
        float3 move_dir = float3(0, 0, 0);
        {
            YoyoUpdateLocalaxis(&debug_cam_ot);

            if (input->keyboard.keys['i'].down)
            {
                move_dir += debug_cam_ot.forward;
            }
            if (input->keyboard.keys['k'].down)
            {
                move_dir += debug_cam_ot.forward * -1;
            }
            if (input->keyboard.keys['j'].down)
            {
                move_dir += debug_cam_ot.right;
            }
            if (input->keyboard.keys['l'].down)
            {
                move_dir += debug_cam_ot.right * -1;
            }
            cam_pitch_yaw += input->mouse.delta_p;
        }
        
        quaternion pitch = axis_angle(float3(1, 0, 0), cam_pitch_yaw.y());
        quaternion yaw   = axis_angle(float3(0, 1, 0), cam_pitch_yaw.x() * -1);
        quaternion turn_qt = (pitch * yaw);

        debug_cam_ot.p += (move_dir * -1) * move_speed  * 0.016f;//ps.time.delta_seconds;
        debug_cam_ot.r = turn_qt;
            
        float4x4 debug_view_matrix = YoyoSetCameraView(&debug_cam_ot);
        Camera::main.ot = debug_cam_ot;
        Camera::main.matrix = debug_view_matrix;
    }

#endif

    //model has a link to MeshAsset/Renderer we for ever sceneobject using model asset flatten out meshassets
    //the renderer
//TODO(Ray):Coarse grain render bound on scene cpu based culling here.
#if 0
//Gather lights and make a list
    for (int i = 0;i < AssetSystem::render_asset_cache.anythings.count;++i)
    {
        ModelAsset* model = (ModelAsset*)AssetSystem::render_asset_cache.anythings.base + i;
        //NOTE(Ray)://TODO(Ray):Here is where you would split out your meshes to the renderer
        //based on what commandlist you want them in.
        for(int j = 0;j < model->meshes.count;++j)
        {
            MeshAsset* mesh = (MeshAsset*)model->meshes.base + j;
//            if(GPUResourceCache::DoesGPUResourceExist(mesh))
            {
//                GPUMeshResource* mr = GPUResourceCache::GetGPUResource(mesh);
                //Pass game data to renderer
                //Add a render command to the render command buffer.
                //For every mesh in a model generate a render command
                //one command represents a renderable
                RenderWithMaterialCommand command_with_material;
                if((uint32_t)mesh->r_material.texture_slots[0].descriptor.width == 1024)
                {
                    int a =0;
                }
                command_with_material.material     = mesh->r_material;
                command_with_material.model_matrix = m_matrix;
                command_with_material.uniforms     = DefferedRenderer::uniform_buffer;
                command_with_material.resource     = mesh->mesh_resource;
                RenderCommandCode::AddRenderCommand(DefferedRenderer::gbufferpass.pass_command_buffer, (void*)&command_with_material);
            }
            //else
            {
               // Assert(false);
            }
        }
    }
#else

    OpenGLEmuState* s = &DefferedRenderer::ogl_test_state;
//Using GLEMU
    for (int i = 0;i < AssetSystem::render_asset_cache.anythings.count;++i)
    {
        ModelAsset* model = (ModelAsset*)AssetSystem::render_asset_cache.anythings.base + i;
        //NOTE(Ray)://TODO(Ray):Here is where you would split out your meshes to the renderer
        //based on what commandlist you want them in.
        for(int j = 0;j < model->meshes.count;++j)
        {
            MeshAsset* mesh = (MeshAsset*)model->meshes.base + j;
//            if(GPUResourceCache::DoesGPUResourceExist(mesh))
            {
//                GPUMeshResource* mr = GPUResourceCache::GetGPUResource(mesh);
                //Pass game data to renderer
                //Add a render command to the render command buffer.
                //For every mesh in a model generate a render command
                //one command represents a renderable

                ogle_use_program(s,DefferedRenderer::diffuse_program);
                ogle_enable_depth_test(s);
                ogle_depth_func(s,compare_func_less);
                
                RenderMaterial material = mesh->r_material;
                
                Uniforms* vuniforms = (Uniforms*)ogle_vert_set_uniform_(s,sizeof(Uniforms),3);
                vuniforms->world_mat = m_matrix;
                vuniforms->pcm_mat = mul(Camera::main.matrix,m_matrix);
                vuniforms->pcm_mat = mul(Camera::main.projection_matrix,vuniforms->pcm_mat);
                vuniforms->inputs.base_color = material.inputs.base_color;
                vuniforms->inputs.metallic_factor = material.inputs.metallic_factor;
                vuniforms->inputs.roughness_factor = material.inputs.roughness_factor;
        
                Uniforms* funiforms = (Uniforms*)ogle_frag_set_uniform_(s,sizeof(Uniforms),3);
                funiforms->view_p = float4(Camera::main.ot.p,1);

                GPUBuffer uv_buffer;
                for(int i = 0;i < material.texture_count;++i)
                {
                    ShaderTextureSlot slot = material.shader.texture_slots[i];
                    ogle_bind_texture_frag(s,material.gl_tex_slots[i],i);
                    if(slot.texcoord_index == 0)
                    {
                        uv_buffer = mesh->mesh_resource.uv_buff;
                    }
                    else if(slot.texcoord_index == 1)
                    {
                        uv_buffer = mesh->mesh_resource.uv2_buff;
                    }
                    else
                    {
                        Assert(false);
                    }
                    ogle_bind_buffer_raw(s,uv_buffer.id,2,0);
                }

                ogle_bind_buffer_raw(s,mesh->mesh_resource.vertex_buff.id,0,0);
                ogle_bind_buffer_raw(s,mesh->mesh_resource.normal_buff.id,1,0);
//TODO(Ray):The use of the index16count here is silly need to fix this.
                ogle_draw_elements(s,mesh->index16_count, mesh->mesh_resource.element_buff.index_type,mesh->mesh_resource.element_buff.id,0);
/*    
                RenderWithMaterialCommand command_with_material;
                if((uint32_t)mesh->r_material.texture_slots[0].descriptor.width == 1024)
                {
                    int a =0;
                }
                command_with_material.material     = mesh->r_material;
                command_with_material.model_matrix = m_matrix;
                command_with_material.uniforms     = DefferedRenderer::uniform_buffer;
                command_with_material.resource     = mesh->mesh_resource;
                RenderCommandCode::AddRenderCommand(DefferedRenderer::gbufferpass.pass_command_buffer, (void*)&command_with_material);
*/
            }
            //else
            {
               // Assert(false);
            }
        }
    }
#endif
    
    
#ifdef OSX || WINDOWS
    ImGui_ImplOSX_NewFrame();
    ImGui::NewFrame();
    Editor::Update();
    // Pre render data preparation.
    ImGui::Render();
#endif
        
    //Update Graphics
    DefferedRenderer::ExecutePasses();

    PlatformOutput(engine_log,"Engine Update Complete\n");
    StringsHandler::ResetTransientStrings();
    EngineInput::ResetKeys(&ps);
    }
};

#endif
