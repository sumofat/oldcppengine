
#if ENGINEIMPL

namespace AssetSystem
{
    FbxManager* fbx_manager = NULL;
    FbxScene* fbx_scene = NULL;

    bool print_fbx = true;
    
    void InitializeSdkObjects(FbxManager*& fbx_manager, FbxScene*& fbx_scene)
    {
        //The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
        fbx_manager = FbxManager::Create();
        if (!fbx_manager)
        {
            PlatformOutput(print_fbx, "Error: Unable to create FBX Manager!\n");
            exit(1);
        }
        else PlatformOutput(print_fbx, "Autodesk FBX SDK version %s\n", fbx_manager->GetVersion());

        //Create an IOSettings object. This object holds all import/export settings.
        FbxIOSettings* ios = FbxIOSettings::Create(fbx_manager, IOSROOT);
        fbx_manager->SetIOSettings(ios);

        //Load plugins from the executable directory (optional)
        FbxString lPath = FbxGetApplicationDirectory();
        fbx_manager->LoadPluginsDirectory(lPath.Buffer());

        //Create an FBX scene. This object holds most objects imported/exported from/to files.
        fbx_scene = FbxScene::Create(fbx_manager, "My Scene");
        if (!fbx_scene)
        {
            PlatformOutput(print_fbx, "Error: Unable to create FBX scene!\n");
            exit(1);
        }
    }
    
    void Init()
    {
        InitializeSdkObjects(fbx_manager, fbx_scene);
        TextureCache::Init(3000);
        ModelCache::Init(3000);
    }
    

    //TODO(Ray):Create a proper interface to FBXSDK
    ModelAsset* LoadModel(char* file_name,PlatformState* ps)
    {
        //1.  When we request to load a model first we go look for the definition file.
        //2. if one does not exist we create one on the fly
        //   NOTE(Ray):Def file has all the info and settings and also references other assets
        //     ie: textures to render that might.
        //     //WE always maintain a direct 1:1 relationship with the source asset for realtime
        //     reloading of asset.  If an asset is save in the DCC we should always see the update
        //     in the engine asap.  Without doing anything.  THe DCC is always the truth of the asset.
        //     The def file is a way for the game to render it properly with the after import settings.
        //     The DEF file ALWAYS references the source asset. ALWAYS.

        //LOAD MODEL from disk
        Yostr* base_path_to_data = BuildPathToAssets(&ps->string_state.string_memory, Directory_None);
        Yostr* mat_final_path = AppendString(*base_path_to_data, *CreateStringFromLiteral(file_name,&ps->string_state.transient_string_memory), &ps->string_state.transient_string_memory);
        read_file_result mat_file_result =  PlatformReadEntireFile(mat_final_path);
        
        //Create a new material if we couldnt find the one at data
        if(mat_file_result.ContentSize <= 0)
        {
            //NOTE(Ray):If we dont have that file create it and fill out default materials than reload it.
            //Doint it this way just t oget things going will revist this later.
            Document d;
            // must pass an allocator when the object may need to allocate memory
            // go through mesh and create a json doc full of default materials for every mesh.
            d.SetObject();
            rapidjson::Document::AllocatorType& allocator = d.GetAllocator();
            size_t sz = allocator.Size();

            d.AddMember("Mesh", "challenger.fbx", allocator);

            Value materials_json(kArrayType);
            
            Yostr* final_path = AppendString(*base_path_to_data, *CreateStringFromLiteral("challenger.fbx",&ps->string_state.transient_string_memory), &ps->string_state.transient_string_memory);
            //After getting model get material definition from disk
            //Load model
            ModelAsset* new_model_asset;
            LoadedModel loaded_model = FBXSDKLoadModel(final_path->String, fbx_manager, fbx_scene, &ps->string_state.transient_string_memory,&ps->string_state.string_memory);
            LoadedModelToModelAsset(loaded_model, new_model_asset);
            for(int i = 0;i < new_model_asset->mesh_count;++i)
            {
                MeshAsset* ma = new_model_asset->meshes + i;
                Value obj(kObjectType);
                Value val(kObjectType);
                val.SetString(ma->name.String,(SizeType)ma->name.Length,allocator);
                obj.AddMember("name",val,allocator);

// create a rapidjson array type with similar syntax to std::vector
                rapidjson::Value float_4_as_array(rapidjson::kArrayType);
                float_4_as_array.PushBack(0,allocator).PushBack(0,allocator).PushBack(0,allocator).PushBack(0,allocator);
                obj.AddMember("base_color",float_4_as_array,allocator);

// create a rapidjson array type with similar syntax to std::vector
                rapidjson::Value texture_array(rapidjson::kArrayType);
                Value tex_val(kObjectType);
                tex_val.SetString("kart.png",(SizeType)8,allocator);
                texture_array.PushBack(tex_val,allocator);
                obj.AddMember("textures",texture_array,allocator);

                Value shader_obj(kObjectType);                
                rapidjson::Value object(rapidjson::kObjectType);
                shader_obj.AddMember("vs","diffuse_vs",allocator);
                shader_obj.AddMember("ps","diffuse_fs",allocator);

// create a rapidjson array type with similar syntax to std::vector
                rapidjson::Value slot_array(rapidjson::kArrayType);
                rapidjson::Value slot_object(rapidjson::kObjectType);
                slot_object.AddMember("id",0,allocator);
                slot_array.PushBack(slot_object,allocator);
                shader_obj.AddMember("slots",slot_array,allocator);

                obj.AddMember("shader",shader_obj,allocator);

                materials_json.PushBack(obj,allocator);
            }
            
            d.AddMember("Materials",materials_json , allocator);

            // Convert JSON document to string
            rapidjson::StringBuffer strbuf;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(strbuf);
            d.Accept(writer);
            const char* output = strbuf.GetString();
            int length = String_GetLength_Char((char*)output);
            PlatformFilePointer file{};
            PlatformWriteMemoryToFile(&file,"challenger.mat",(void*)output,length,true,"w");
//TODO(Ray):For now we just write a file to this location which is the default director on windows on osx
            ///Users/ray.garner/Library/Containers/dena.pedaltothemedal/Data
            //than we manually copy it over... obviously later we will need to fix that but need to continue focusing on the renderer for now.
            //come back to this lil quirk later.
            Assert(false);
            //PlatformOutput(true,"%s",output); 
        }
        
    }
        /*
        
// 1. Parse a JSON string into DOM.
        Document d;
        d.Parse((char*)mat_file_result.Content);
        if(!d.IsObject())
        {
            //Error handling
            assert(false);
        }

        const Value& mesh = d["Mesh"];
        Yostr* r = CreateStringFromLiteralConst(d["Mesh"].GetString(),&ps->string_state.transient_string_memory);
    
        Yostr* final_path = AppendString(*base_path_to_data, *CreateStringFromLiteral(r->String,&ps->string_state.transient_string_memory), &ps->string_state.transient_string_memory);
        //After getting model get material definition from disk

//Load model    
        LoadedModel loaded_model = FBXSDKLoadModel(final_path->String, fbx_manager, fbx_scene, &ps->string_state.transient_string_memory,&ps->string_state.string_memory);
        Entity* entity = PushStruct(&EntitySytem::runtime_entities, Entity);
        entity->ot.p = float3(0, 0, 0);
        entity->ot.s = float3(1, 1, 1);
        entity->ot.r = axis_angle(float3(0, 1, 0), 0);
        ModelAsset *new_model_asset = PushStruct(&runtime_assets, ModelAsset);
        entity->asset = new_model_asset;
        LoadedModelToModelAsset(loaded_model, new_model_asset);
        //TODO(Ray):Why does this fail sometimes!!
        UploadModelAssetToGPU(new_model_asset);
//END LOAD MODEL FROM DISK
        
//This next part gets and loads all the relevent information to render this object properly.
        //NOTE(Ray):
        //1. We dont have any good defaults or ways to generate this file from a newly imported mesh.
        //2. We will fail if the mesh is added to or removed need to have some mapping.
        //3. and add remove materials textures etc base on last use.
        //4. PSO's should also be allowed to be stored offline for immediate async creation upon startup
        //or at the users discretion.
        
        Texture uploaded_texture;
        RenderShader shader;
        const Value& materials = d["Materials"];
        int mesh_index = 0;
        for (auto& material : materials.GetArray())
        {
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
            }
        }
        return new_model_asset;
    }
    */
    
    //TODO(Ray):Next we need a facility for handling assets that are shared in memory between
    //mutiple assets.
    //The most obvious solution is a reference counting solution.
    //When an asset is done with another asset it can "release" it and decrement the internally handled
    //counter.
    //Until the counter reaches zero which at that point we really will "release"
    //Seems ok but there are also issues with it.

//NOTE(Ray):Maybe or we will just release everything on scence change for starters.
    //TODO(Ray):Handle loosing the device reference and rebuild everthing in that case...
    
    bool AddOrGetTexture(Yostr path,LoadedTexture* result)
    {
        bool bool_result = false;
        //result = nullptr;
        //TODO(Ray):Propery handling of a unfound asset
        if(!TextureCache::DoesTextureExist(&path))
        {
            //TODO(Ray):This gets it from disk but we should be able to get it from anywhere .. network stream etc...
            //We will add a facility for tagging assets and retrieving base on criteria and only the asset retriever
            //(Serializer) like system will be the only one to deal with what we are getting it from.
            LoadedTexture tex = YoyoSpriteBatchRenderer::GetLoadedImage(path.String,&StringsHandler::transient_string_memory);
            if(tex.texels)
            {
                tex.texture.state = PlatformGraphicsAPI_Metal::GPUAllocateTexture(tex.texels,tex.bytes_per_pixel,tex.dim.x(),tex.dim.y());
                TextureCache::AddTexture(&path,&tex);
                *result = tex;
                bool_result = true;
            }
        }
        else
        {
            LoadedTexture* tex = TextureCache::GetTexture(&path);
            if(tex)
            {
                *result = *tex;
                bool_result = true;
            }
        }
        return bool_result;
    }
};

#endif
