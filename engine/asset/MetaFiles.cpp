
#if ENGINEIMPL
using namespace rapidjson;

struct MetaScenes
{
    uint32_t entry_scene_index;//this scene will be the one loaded after the engine is inited
    vector scenes;//Yostrings
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
    char* game_data_dir = "~/work/pedaltothemetal/games/pedaltothemetal/data/";
    char* game_data_meta_dir = "/Users/ray.garner/work/pedaltothemetal/games/pedaltothemetal/data/metafiles/";
    Yostr file_write_path;

    void Init()
    {
        Yostr* asset_path = BuildPathToAssets(&StringsHandler::transient_string_memory,0);
        file_write_path = *AppendString(*asset_path,CreateStringFromLiteral("/metafiles/",&StringsHandler::transient_string_memory),&StringsHandler::string_memory);
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

    Yostr* GetMetaFile(Yostr path_to_asset)
    {
        Yostr* result;
        Yostr file_name = GetFilenameFromPath(path_to_asset, &StringsHandler::transient_string_memory);
        Yostr* file_namenoext = StripExtension(&file_name, &StringsHandler::transient_string_memory);
        file_namenoext = AppendString(*file_namenoext, CreateStringFromLiteral(".mat", &StringsHandler::transient_string_memory), &StringsHandler::transient_string_memory);
        Yostr* path_to_meta_file = AppendStringToChar(game_data_meta_dir,*file_namenoext,&StringsHandler::transient_string_memory);
        read_file_result meta_file_result = PlatformReadEntireFile(path_to_meta_file);
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
    
    Value AddInputEntryToArray(Yostr* name,ShaderInputTypes input_type,void* value,rapidjson::Document::AllocatorType& allocator)
    {
        Value input_object(kObjectType);
        Value base_color_name(kObjectType);
        base_color_name.SetString(name->String,(SizeType)name->Length,allocator);
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
#if 0
                type_text = CreateStringFromLiteral("texture",&StringsHandler::transient_string_memory);
                Value base_color_type(kObjectType);
                base_color_type.SetString(type_text.String,(SizeType)type_text.Length,allocator);
                input_object.AddMember("type",base_color_type,allocator);

                Texture final_value = *((Texture*)value);
                Value base_color_value(kArrayType);
                base_color_value.PushBack(final_value.x(),allocator).PushBack(final_value.y(),allocator).PushBack(final_value.z(),allocator).PushBack(final_value.w(),allocator);
                input_object.AddMember("value",base_color_value,allocator);
#endif
                //Not supported yet.
                Assert(false);
            }break;
            default:
            {
                //Unsupported type.
                Assert(false);
            }break;
        }
        return input_object;
    }
    
    Yostr* CreateDefaultModelMetaFile(Yostr filepath,ModelAsset* model)
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
            Yostr* namenoext = StripExtension(&name,&StringsHandler::transient_string_memory);
            Value n(namenoext->String, allocator);            
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
            Yostr* filenamenoext = StripExtension(&filename,&StringsHandler::transient_string_memory);
            Yostr* final_filename = AppendString(*filenamenoext,CreateStringFromLiteral(".mat",&StringsHandler::transient_string_memory),&StringsHandler::transient_string_memory);
            PlatformFilePointer file{};
            Yostr* final_output_path = AppendStringToChar(game_data_meta_dir,*final_filename,&StringsHandler::transient_string_memory);
            PlatformWriteMemoryToFile(&file,final_output_path->String,(void*)output,length,true,"w+");
            Yostr* result = CreateStringFromLength((char*)output, length, &StringsHandler::transient_string_memory);
            PlatformOutput(log_output,"%s",result->String); 
            return result;
        }
    }
       
    Yostr* GetOrCreateDefaultModelMetaFile(Yostr file,ModelAsset* model)
    {
        Yostr* result = GetMetaFile(file);
        if(result->Length == 0 && model)//no meta file make one
        {
            Yostr* file_ext = GetExtension(&file,&StringsHandler::transient_string_memory,false);
            if(MetaFileType::FBX == GetFileExtensionType(*file_ext))
            {
                result = CreateDefaultModelMetaFile(file,model);
            }
//others here
        }
        return result;
    }
};

#endif
