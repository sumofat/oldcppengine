
#if ENGINEIMPL
using namespace rapidjson;

struct MetaScenes
{
    uint32_t entry_scene_index;//this scene will be the one loaded after the engine is inited
    YoyoVector scenes;//Yostrings
};

            //we will put all metafiles in one big dump folder for now
//We will try to make it so that humans will not have to actually write these.
            //Problem is they are jsong and that sucks ...
            //Assets will not directly read from these properties only the editor.
//These metal files will creat other type of assets. and they are only here for ease of use and flexibility as far
//as I can see right now.
//When deploying these files should be flat binary perhaps.  Thats a decision to be made on a game by game basis.
namespace MetaFiles
{
    bool log_output = true;
    Yostr file_write_path;
    char* game_data_dir;// = "~/work/pedaltothemetal/games/pedaltothemetal/data/";
    char* game_data_meta_dir;// = "/Users/ray.garner/work/mar/games/pedaltothemetal/data/metafiles/";

    void Init()
    {
        Yostr asset_path = BuildPathToAssets(&StringsHandler::transient_string_memory,0);
        file_write_path = AppendString(asset_path,CreateStringFromLiteral("/metafiles/",&StringsHandler::transient_string_memory),&StringsHandler::string_memory);
        game_data_dir =      "/Users/ray.garner/work/mar/games/pedaltothemetal/data/";
        game_data_meta_dir = "/Users/ray.garner/work/mar/games/pedaltothemetal/data/metafiles/";
        
#ifdef IOS
        game_data_meta_dir = file_write_path.String;
#endif
    }
    
    ShaderValueType::Type GetShaderType(Yostr type)
    {
        for(auto element : ShaderValueType::as_array)
        {
            if(CompareStringtoChar(type,"float4"))
            {
                return ShaderValueType::float4;                                    
            }
            else if(CompareStringtoChar(type,"float3"))
            {
                return ShaderValueType::float3;                                    
            }
            else if(CompareStringtoChar(type,"float2"))
            {
                return ShaderValueType::float2;                                    
            }
            else if(CompareStringtoChar(type,"float"))
            {
                return ShaderValueType::afloat;                                    
            }
            else if(CompareStringtoChar(type,"texture"))
            {
                return ShaderValueType::texture;                                    
            }
            else
            {
                PlatformOutput(log_output,"No shader value of that type supported by the asset system.");                
            }
        }
        return ShaderValueType::unknown;
    }
    
    MetaFileType::Type GetFileExtensionType(Yostr file)
    {
        for(auto element : MetaFileType::as_array)
        {
            if(CompareStringtoChar(file,"fbx"))
            {
                return MetaFileType::FBX;                                    
            }
            else if(CompareStringtoChar(file,"psd"))
            {
                return MetaFileType::PSD;                                    
            }
            else if(CompareStringtoChar(file,"png"))
            {
                return MetaFileType::PNG;                                    
            }
            else
            {
                PlatformOutput(log_output,"No File extenstion of that type supported by metal file system.");                
            }
        }
        return MetaFileType::NONE;
    }

    Yostr GetMetaFile(Yostr path_to_asset)
    {
        Yostr result;
        Yostr file_name = GetFilenameFromPath(path_to_asset, &StringsHandler::transient_string_memory);
        Yostr file_namenoext = StripExtension(&file_name, &StringsHandler::transient_string_memory);
        file_namenoext = AppendString(file_namenoext, CreateStringFromLiteral(".mat", &StringsHandler::transient_string_memory), &StringsHandler::transient_string_memory);
        Yostr path_to_meta_file = AppendStringToChar(game_data_meta_dir,file_namenoext,&StringsHandler::transient_string_memory);
        read_file_result meta_file_result = PlatformReadEntireFile(&path_to_meta_file);
        result = CreateStringFromLength((char*)meta_file_result.Content,meta_file_result.ContentSize,&StringsHandler::transient_string_memory);
//ok as long as we are an ascii string//1 byte = 1 character
        return result;
    }

    enum ShaderInputTypes
    {
        shader_input_float4,
        shader_input_float3,
        shader_input_float2,
        shader_input_float,
        shader_input_texture,
    };

    Value AddInputEntryToArray(char* name,ShaderInputTypes input_type,void* value,rapidjson::Document::AllocatorType& allocator)
    {
        Value input_object(kObjectType);
        Value base_color_name(kObjectType);
        base_color_name.SetString(name,(SizeType)CalculateCharLength(name),allocator);
        input_object.AddMember("name",base_color_name,allocator);

        Yostr type_text;
        switch(input_type)
        {
            case shader_input_float:
            {
                type_text = CreateStringFromLiteral("float",&StringsHandler::transient_string_memory);
                Value base_color_type(kObjectType);
                base_color_type.SetString(type_text.String,(SizeType)type_text.Length,allocator);
                input_object.AddMember("type",base_color_type,allocator);
                float final_value = *((float*)value);
                Value base_color_value(kArrayType);
                base_color_value.PushBack(final_value,allocator);
                input_object.AddMember("value",base_color_value,allocator);
            }break;
            case shader_input_float2:
            {
                type_text = CreateStringFromLiteral("float2",&StringsHandler::transient_string_memory);
                Value base_color_type(kObjectType);
                base_color_type.SetString(type_text.String,(SizeType)type_text.Length,allocator);
                input_object.AddMember("type",base_color_type,allocator);
                float2 final_value = *((float2*)value);
                Value base_color_value(kArrayType);
                base_color_value.PushBack(final_value.x(),allocator).PushBack(final_value.y(),allocator);
                input_object.AddMember("value",base_color_value,allocator);
            }break;
            case shader_input_float3:
            {
                type_text = CreateStringFromLiteral("float3",&StringsHandler::transient_string_memory);
                Value base_color_type(kObjectType);
                base_color_type.SetString(type_text.String,(SizeType)type_text.Length,allocator);
                input_object.AddMember("type",base_color_type,allocator);
                float3 final_value = *((float3*)value);
                Value base_color_value(kArrayType);
                base_color_value.PushBack(final_value.x(),allocator).PushBack(final_value.y(),allocator).PushBack(final_value.z(),allocator);
                input_object.AddMember("value",base_color_value,allocator);
            }break;
            case shader_input_float4:
            {
                type_text = CreateStringFromLiteral("float4",&StringsHandler::transient_string_memory);
                Value base_color_type(kObjectType);
                base_color_type.SetString(type_text.String,(SizeType)type_text.Length,allocator);
                input_object.AddMember("type",base_color_type,allocator);
                float4 final_value = *((float4*)value);
                Value base_color_value(kArrayType);
                base_color_value.PushBack(final_value.x(),allocator).PushBack(final_value.y(),allocator).PushBack(final_value.z(),allocator).PushBack(final_value.w(),allocator);
                input_object.AddMember("value",base_color_value,allocator);
            }break;
            case shader_input_texture:
            {
#if 1
                type_text = CreateStringFromLiteral("texture",&StringsHandler::transient_string_memory);
                Value base_color_type(kObjectType);
                base_color_type.SetString(type_text.String,(SizeType)type_text.Length,allocator);
                input_object.AddMember("type",base_color_type,allocator);

                char* final_value = ((char*)value);
                uint32_t string_length = CalculateCharLength(final_value);
                Value base_color_value(kObjectType);
                base_color_value.SetString(final_value,(SizeType)string_length,allocator);
//                base_color_value.PushBack(final_value.x(),allocator).PushBack(final_value.y(),allocator).PushBack(final_value.z(),allocator).PushBack(final_value.w(),allocator);
                input_object.AddMember("value",base_color_value,allocator);
#endif
                //Not supported yet.
                //Assert(false);
            }break;
            default:
            {
                //Unsupported type.
                Assert(false);
            }break;
        }
        return input_object;        
    }
    
    Value AddInputEntryToArray(Yostr* name,ShaderInputTypes input_type,void* value,rapidjson::Document::AllocatorType& allocator)
    {
        return AddInputEntryToArray(name->String,input_type,value,allocator);
    }
    
    struct AssetLoadingTextureKey
    {
        uint32_t offset;
        uint64_t ptr_to_bin;
    };

    struct AssetLoadingTextureValue
    {
//        LoadedTexture lt;
        Yostr path;
        Yostr name;
    };
    
    void StartMetaFileCreation(InProgressMetaFile* mf,Yostr filepath,uint32_t mesh_count)
    {
        Assert(mf);
//        mf->d;
        mf->d.SetObject();
        mf->file_path = filepath;
        mf->model = {};
        mf->model.meshes = YoyoInitVector(mesh_count, MeshAsset , false);
        AnythingCacheCode::Init(&mf->tex_cache, 4096,sizeof(AssetLoadingTextureValue),sizeof(AssetLoadingTextureKey));
        
        rapidjson::Document::AllocatorType& allocator = mf->d.GetAllocator();
        size_t sz = allocator.Size();
        Yostr name = GetFilenameFromPath(filepath,&StringsHandler::transient_string_memory);
        Yostr namenoext = StripExtension(&name,&StringsHandler::transient_string_memory);
        Value n(namenoext.String, allocator);            
        mf->d.AddMember("model_file", n, allocator);        
        Value meshes_json(kArrayType);
        mf->meshes_json = meshes_json;
        
    }

    void SetDefaultMaterial(InProgressMetaFile* mf,Value* obj,MeshAsset* mesh)
    {
        rapidjson::Document::AllocatorType& allocator = mf->d.GetAllocator();
        Value mat_obj(kObjectType);
        Value mat_val(kObjectType);
        Yostr default_mat_string = CreateStringFromLiteral("default_material",&StringsHandler::transient_string_memory);
        mat_val.SetString(default_mat_string.String,(SizeType)default_mat_string.Length,allocator);
        mat_obj.AddMember("material_name",mat_val,allocator);

        Value materials_array(rapidjson::kArrayType);
        Value inputs_array(rapidjson::kArrayType);
        //Set defaults as
        //base color float4(1)
        //textures empty

        Yostr bcname = CreateStringFromLiteral("base_color",&StringsHandler::transient_string_memory);
        float4 bcvalue = float4(1);
        Value input_object = AddInputEntryToArray(&bcname,shader_input_float4,&bcvalue,allocator);
//TODO(Ray):FOr PBR materials we will need to add defaults for rougness specular and others.
/*
  Yostr* bcname = CreateStringFromLiteral("base_color",&StringsHandler::transient_string_memory);
  float4 bcvalue = float4(1);
  Value input_object = AddInputEntryToArray(bcname,shader_input_float4,&bcvalue,allocator);
*/

        //NOTE(Ray):All meshes are assigned the defauilt material till we parse a better one from the gltf.
        //Or extract one from engine meta data from engine created materials
        mesh->r_material = AssetSystem::default_mat;
        
        inputs_array.PushBack(input_object,allocator);
        mat_obj.AddMember("inputs",inputs_array,allocator);                
        materials_array.PushBack(mat_obj,allocator);
                
        obj->AddMember("materials",materials_array, allocator);
    }
    
    void WriteAddTexture(InProgressMetaFile* mf,cgltf_texture_view tv,char* tex_name,Value* inputs_array,cgltf_mesh* ma)
    {
        rapidjson::Document::AllocatorType& allocator = mf->d.GetAllocator();

        uint32_t offset = tv.texture->image->buffer_view->offset;
        void* tex_data = (uint8_t*)tv.texture->image->buffer_view->buffer->data + offset;
        uint64_t data_size = tv.texture->image->buffer_view->size;
        AssetLoadingTextureKey k = {offset,(uint64_t)tex_data};
        Yostr final_twrfp = {};
        AssetLoadingTextureValue ltv = {};
                    
        if(!AnythingCacheCode::DoesThingExist(&mf->tex_cache, &k))
        {

            Yostr bcname = CreateStringFromLiteral(tex_name,&StringsHandler::transient_string_memory);
            Yostr bccname =  AppendCharToString(bcname,ma->name,&StringsHandler::transient_string_memory);
            Yostr texture_write_file_path = AppendString(CreateStringFromLiteral(game_data_dir,&StringsHandler::transient_string_memory), bccname, &StringsHandler::transient_string_memory) ;
            final_twrfp = AppendCharToString(texture_write_file_path,".png",&StringsHandler::transient_string_memory);
            LoadedTexture lt = {};
//            Resource::GetImageFromMemory(tex_data,data_size,&lt,4);
            ltv.name = bcname;
//            ltv.lt = lt;
            ltv.path = final_twrfp;

            //write texture bin to disk
            PlatformFilePointer fp = {};
//            PlatformWriteMemoryToFile(&fp, final_twrfp.String, lt.texels, lt.dim.x() * lt.dim.y() * lt.bytes_per_pixel,true,"w+");
            PlatformWriteMemoryToFile(&fp, final_twrfp.String, tex_data,data_size,true,"w+");
            AnythingCacheCode::AddThing(&mf->tex_cache, &k, &ltv);
        }
        else
        {
            ltv = *GetThingPtr(&mf->tex_cache,&k,AssetLoadingTextureValue);
        }
                    
        //value to pass is the path
        char* bcvalue = ltv.path.String;//final_twrfp.String;
        Value input_object = AddInputEntryToArray(&ltv.name,shader_input_texture,bcvalue,allocator);

        inputs_array->PushBack(input_object,allocator);
        //Get path to textures on disk relative to data folder.
        char* name = tv.texture->name;
        char* uri = tv.texture->image->uri;
        cgltf_sampler* sampler = tv.texture->sampler;
    }
    
    void AddMeshToMetaFile(InProgressMetaFile* mf,cgltf_mesh map)
    {
        cgltf_mesh* ma = &map;
        rapidjson::Document::AllocatorType& allocator = mf->d.GetAllocator();
        Value obj(kObjectType);
        Value val(kObjectType);
        uint32_t name_length = CalculateCharLength(ma->name);
        val.SetString(ma->name,(SizeType)name_length,allocator);
        obj.AddMember("meshname",val,allocator);
           
        //Extract mesh binary data
        for(int j = 0;j < ma->primitives_count;++j)
        {
            cgltf_primitive prim = ma->primitives[j];
            cgltf_material* mat  = prim.material;
            
            Value mat_obj(kObjectType);
            Value mat_val(kObjectType);
            Yostr default_mat_string = CreateStringFromLiteral(mat->name,&StringsHandler::transient_string_memory);
            mat_val.SetString(default_mat_string.String,(SizeType)default_mat_string.Length,allocator);
            mat_obj.AddMember("material_name",mat_val,allocator);
                
            Value materials_array(rapidjson::kArrayType);
            Value inputs_array(rapidjson::kArrayType);
            Value mesh_array(rapidjson::kArrayType);
            //Set defaults as
            //base color float4(1)
            //textures empty
            if(mat->normal_texture.texture)
            {
                cgltf_texture_view tv = mat->normal_texture;
                WriteAddTexture(mf,tv,"normal_texture",&inputs_array,ma);                
            }

            if(mat->occlusion_texture.texture)
            {
                cgltf_texture_view tv = mat->occlusion_texture;
                WriteAddTexture(mf,tv,"normal_texture",&inputs_array,ma);                
            }

            if(mat->emissive_texture.texture)
            {
                cgltf_texture_view tv = mat->emissive_texture;
                WriteAddTexture(mf,tv,"normal_texture",&inputs_array,ma);                
//                cgltf_float emissive_factor[3];
            }
            /*
             * 	cgltf_alpha_mode alpha_mode;
             cgltf_float alpha_cutoff;
             cgltf_bool double_sided;
             cgltf_bool unlit;
            */
            
            if(mat->has_pbr_metallic_roughness)
            {
                if(mat->pbr_metallic_roughness.base_color_texture.texture)
                {
                    cgltf_texture_view tv = mat->pbr_metallic_roughness.base_color_texture;
                    WriteAddTexture(mf,tv,"pbr_mettalic_roughness_texture_base_color",&inputs_array,ma);
                }
                if(mat->pbr_metallic_roughness.metallic_roughness_texture.texture)
                {
                    cgltf_texture_view tv = mat->pbr_metallic_roughness.metallic_roughness_texture;
                    WriteAddTexture(mf,tv,"pbr_mettalic_roughness_texture_metallic_roughness",&inputs_array,ma);
                }

                cgltf_float* bcf = mat->pbr_metallic_roughness.base_color_factor;
                float4 base_color_value = float4(bcf[0],bcf[1],bcf[2],bcf[3]);
                Value input_object = AddInputEntryToArray("base_color",shader_input_float4,(void*)&base_color_value,allocator);
                inputs_array.PushBack(input_object,allocator);

                cgltf_float* mf = &mat->pbr_metallic_roughness.metallic_factor;
                Value mf_input_object = AddInputEntryToArray("metallic_factor",shader_input_float,(void*)&mf,allocator);
                inputs_array.PushBack(mf_input_object,allocator);

                cgltf_float* rf = &mat->pbr_metallic_roughness.roughness_factor;
                Value rf_input_object = AddInputEntryToArray("roughness_factor",shader_input_float,(void*)&rf,allocator);
                inputs_array.PushBack(rf_input_object,allocator);
            }
            
            if(mat->has_pbr_specular_glossiness)
            {
                if(mat->pbr_specular_glossiness.diffuse_texture.texture)
                {
                    cgltf_texture_view tv = mat->pbr_specular_glossiness.diffuse_texture;
                    WriteAddTexture(mf,tv,"pbr_specular_glossiness_diffuse_texture",&inputs_array,ma);
                }
                if(mat->pbr_specular_glossiness.specular_glossiness_texture.texture)
                {
                    cgltf_texture_view tv = mat->pbr_specular_glossiness.specular_glossiness_texture;
                    WriteAddTexture(mf,tv,"pbr_specular_glossiness_specular_glossiness_texture",&inputs_array,ma);
                }

                cgltf_float* dcf = mat->pbr_specular_glossiness.diffuse_factor;
                float4 diffuse_value = float4(dcf[0],dcf[1],dcf[2],dcf[3]);
                Value input_object = AddInputEntryToArray("diffuse_factor",shader_input_float4,(void*)&diffuse_value,allocator);
                inputs_array.PushBack(input_object,allocator);

                cgltf_float* sf = mat->pbr_specular_glossiness.specular_factor;
                float3 specular_value = float3(sf[0],sf[1],sf[2]);
                Value sf_input_object = AddInputEntryToArray("specular_factor",shader_input_float3,(void*)&specular_value,allocator);
                inputs_array.PushBack(sf_input_object,allocator);

                cgltf_float* gf = &mat->pbr_specular_glossiness.glossiness_factor;
                Value gf_input_object = AddInputEntryToArray("glossiness_factor",shader_input_float,(void*)&gf,allocator);
                inputs_array.PushBack(gf_input_object,allocator);
            }

            mat_obj.AddMember("inputs",inputs_array,allocator);                
            materials_array.PushBack(mat_obj,allocator);
            obj.AddMember("materials",materials_array, allocator);

            MeshAsset mesh = {};
            mesh.name = CreateStringFromLiteral(ma->name,&StringsHandler::string_memory);
            mesh.r_material = AssetSystem::default_mat;
            //get buffer data from mesh
            if(prim.type == cgltf_primitive_type_triangles)
            {
                for(int k = 0;k < prim.attributes_count;++k)
                {
                    cgltf_attribute ac = prim.attributes[k];
                    //Get indices
                    if(prim.indices->component_type == cgltf_component_type_r_16u)
                    {
                        // cgltf_attribute ac = prim.attributes[k];
                        cgltf_buffer_view* ibf = prim.indices->buffer_view;
                            
                        uint64_t istart_offset = ibf->offset;
                        cgltf_buffer* ibuf = ibf->buffer;
                        uint16_t* indices_buffer = (uint16_t*)((uint8_t*)ibuf->data + istart_offset);
                        mesh.index_16_data = indices_buffer;
                        mesh.index_16_data_size = ibf->size;
                        mesh.index16_count = prim.indices->count;
                    }
                    //Get verts
                    cgltf_accessor* acdata = ac.data;
                    uint64_t count = acdata->count;
                    PlatformOutput(true,ac.name);
                            
                    cgltf_buffer_view* bf = acdata->buffer_view;
                            
                    //Get vertex buffer
                    uint64_t start_offset = bf->offset;
                    uint32_t stride = bf->stride;
                    cgltf_buffer* buf = bf->buffer;
                    float* buffer = (float*)((uint8_t*)buf->data + start_offset);
                            
                    if(ac.type == cgltf_attribute_type_position)
                    {
                        mesh.vertex_data = buffer;
                        mesh.vertex_data_size = bf->size;
                        mesh.vertex_count = count * 3;
                    }
                    else if(ac.type == cgltf_attribute_type_normal)
                    {
                        mesh.normal_data = buffer;
                        mesh.normal_data_size = bf->size;
                        mesh.normal_count = count * 3;
                    }
                    else if(ac.type == cgltf_attribute_type_tangent)
                    {
                        mesh.tangent_data = buffer;
                        mesh.tangent_data_size = bf->size;
                        mesh.tangent_count = count * 3;
                    }
                    else if(ac.type == cgltf_attribute_type_texcoord)
                    {
                        mesh.uv_data = buffer;
                        mesh.uv_data_size = bf->size;
                        mesh.uv_count = count * 2;
                    }
                }
            }
            YoyoStretchPushBack(&mf->model.meshes, mesh);
            obj.AddMember("mesh",mesh_array,allocator);
        }

        mf->meshes_json.PushBack(obj,allocator);
    }
    
    void EndMetaFileCreation(InProgressMetaFile* mf)
    {
        rapidjson::Document::AllocatorType& allocator = mf->d.GetAllocator();
        mf->d.AddMember("Meshes",mf->meshes_json , allocator);
        // Convert JSON document to string

        rapidjson::StringBuffer strbuf;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(strbuf);
        mf->d.Accept(writer);
        const char* output = strbuf.GetString();
        uint32_t length = String_GetLength_Char((char*)output);

        Yostr filename = GetFilenameFromPath(mf->file_path,&StringsHandler::transient_string_memory);
        Yostr filenamenoext = StripExtension(&filename,&StringsHandler::transient_string_memory);
        Yostr final_filename = AppendString(filenamenoext,CreateStringFromLiteral(".mat",&StringsHandler::transient_string_memory),&StringsHandler::transient_string_memory);
        PlatformFilePointer file{};
        Yostr final_output_path = AppendStringToChar(game_data_meta_dir,final_filename,&StringsHandler::transient_string_memory);
        PlatformWriteMemoryToFile(&file,final_output_path.String,(void*)output,length,true,"w+");
        Yostr result = CreateStringFromLength((char*)output, length, &StringsHandler::transient_string_memory);
        PlatformOutput(log_output,"%s",result.String);         
    }
    
    Yostr CreateDefaultModelMetaFile(Yostr filepath,ModelAsset* model)
    {
        //Create a new material if we couldnt find the one at data
//        if(mat_file_result.ContentSize <= 0)
        {
            //NOTE(Ray):If we dont have that file create it and fill out default materials than reload it.
            //Doint it this way just t oget things going will revist this later.
            Document d;
// must pass an allocator when the object may need to allocate memory
            // go through mesh and create a json doc full of default materials for every mesh.
            d.SetObject();
            rapidjson::Document::AllocatorType& allocator = d.GetAllocator();
            size_t sz = allocator.Size();
            
//TODO(Ray):Set last known filepath to mesh relative to data dir which will be platform
            //agnostic

            Yostr name = GetFilenameFromPath(filepath,&StringsHandler::transient_string_memory);
            Yostr namenoext = StripExtension(&name,&StringsHandler::transient_string_memory);
            Value n(namenoext.String, allocator);            
            d.AddMember("model_file", n, allocator);

            Value meshes_json(kArrayType);

            for(int i = 0;i < model->meshes.count;++i)
            {
                MeshAsset* ma = (MeshAsset*)model->meshes.base + i;
                Value obj(kObjectType);
                Value val(kObjectType);
                val.SetString(ma->name.String,(SizeType)ma->name.Length,allocator);
                obj.AddMember("meshname",val,allocator);

                //mesh object
                //material object
                //   name/link
                //   inputs array
                //       input object
                //            type//float4 float3 float2 float texture
                //            values
                //            name
                //TODO(Ray):These are linked by the user to the material
                //thee are inputs to the material.
                //When we create a material we have a list of inputs
                //and when you link a material to a we gen a list of default inputs to those
                //properties and also can be tweaked by the user via the ui. or directly here.
                //preferably UI.
                //Types supported are vectors of 2 , 3 , 4 and textures for now.
                Value mat_obj(kObjectType);
                Value mat_val(kObjectType);

                Yostr default_mat_string = CreateStringFromLiteral("default_material",&StringsHandler::transient_string_memory);
                
                mat_val.SetString(default_mat_string.String,(SizeType)default_mat_string.Length,allocator);
                mat_obj.AddMember("material_name",mat_val,allocator);
                
                Value materials_array(rapidjson::kArrayType);
                Value inputs_array(rapidjson::kArrayType);
                //Set defaults as
                //base color float4(1)
                //textures empty

                Yostr bcname = CreateStringFromLiteral("base_color",&StringsHandler::transient_string_memory);
                float4 bcvalue = float4(1);
                Value input_object = AddInputEntryToArray(&bcname,shader_input_float4,&bcvalue,allocator);
//TODO(Ray):FOr PBR materials we will need to add defaults for rougness specular and others.
/*
                Yostr* bcname = CreateStringFromLiteral("base_color",&StringsHandler::transient_string_memory);
                float4 bcvalue = float4(1);
                Value input_object = AddInputEntryToArray(bcname,shader_input_float4,&bcvalue,allocator);
*/
                
                inputs_array.PushBack(input_object,allocator);
                mat_obj.AddMember("inputs",inputs_array,allocator);                
                materials_array.PushBack(mat_obj,allocator);
                obj.AddMember("materials",materials_array, allocator); // 
                //TODO(Ray):Create a default shader/materialdefinition

/*                
                rapidjson::Value float_4_as_array(rapidjson::kArrayType);
                float_4_as_array.PushBack(1,allocator).PushBack(1,allocator).PushBack(1,allocator).PushBack(1,allocator);
                obj.AddMember("base_color",float_4_as_array,allocator);

                rapidjson::Value texture_array(rapidjson::kArrayType);
                Value tex_val(kObjectType);
//                tex_val.SetString(" ",(SizeType)8,allocator);
//                texture_array.PushBack(tex_val,allocator);
                obj.AddMember("textures",texture_array,allocator);
                
                //TODO(Ray):Shaders shoud be in the material file only
                Value shader_obj(kObjectType);                
                rapidjson::Value object(rapidjson::kObjectType);
//TODO(Ray):Allow user to define default shader names
                shader_obj.AddMember("vs","diffuse_vs",allocator);
                shader_obj.AddMember("ps","diffuse_fs",allocator);

                rapidjson::Value slot_array(rapidjson::kArrayType);
                rapidjson::Value slot_object(rapidjson::kObjectType);

                slot_object.AddMember("id",0,allocator);
                slot_array.PushBack(slot_object,allocator);
                shader_obj.AddMember("slots",slot_array,allocator);

                obj.AddMember("shader",shader_obj,allocator);
*/
                meshes_json.PushBack(obj,allocator);
            }
            
            d.AddMember("Meshes",meshes_json , allocator);

            // Convert JSON document to string
            rapidjson::StringBuffer strbuf;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(strbuf);
            d.Accept(writer);
            const char* output = strbuf.GetString();
            uint32_t length = String_GetLength_Char((char*)output);

            Yostr filename = GetFilenameFromPath(filepath,&StringsHandler::transient_string_memory);
            Yostr filenamenoext = StripExtension(&filename,&StringsHandler::transient_string_memory);
            Yostr final_filename = AppendString(filenamenoext,CreateStringFromLiteral(".mat",&StringsHandler::transient_string_memory),&StringsHandler::transient_string_memory);
            PlatformFilePointer file{};
            Yostr final_output_path = AppendStringToChar(game_data_meta_dir,final_filename,&StringsHandler::transient_string_memory);
            PlatformWriteMemoryToFile(&file,final_output_path.String,(void*)output,length,true,"w+");
            Yostr result = CreateStringFromLength((char*)output, length, &StringsHandler::transient_string_memory);
            PlatformOutput(log_output,"%s",result.String); 
            return result;
        }
    }
       
    Yostr GetOrCreateDefaultModelMetaFile(Yostr file,ModelAsset* model)
    {
        Yostr result = GetMetaFile(file);
        if(result.Length == 0 && model)//no meta file make one
        {
            Yostr file_ext = GetExtension(&file,&StringsHandler::transient_string_memory,false);
            if(MetaFileType::FBX == GetFileExtensionType(file_ext))
            {
                result = CreateDefaultModelMetaFile(file,model);
            }
//others here
        }
        return result;
    }
};

#endif
