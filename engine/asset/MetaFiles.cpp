
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
        file_write_path = *AppendString(*asset_path,*CreateStringFromLiteral("/metafiles/",&StringsHandler::transient_string_memory),&StringsHandler::string_memory);
    }
    
    MetaFileType::Type GetFileExtensionType(Yostr* file)
    {
        for(auto element : MetaFileType::as_array)
        {
            if(CompareStringtoChar(*file,"fbx"))
            {
                return MetaFileType::FBX;                                    
            }
            else if(CompareStringtoChar(*file,"psd"))
            {
                return MetaFileType::PSD;                                    
            }
            else if(CompareStringtoChar(*file,"png"))
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
    
    Yostr* GetMetaFile(Yostr* path_to_asset)
    {
        Yostr* result;
        Yostr file_name = GetFilenameFromPath(path_to_asset, &StringsHandler::transient_string_memory);
        Yostr* file_namenoext = StripExtension(&file_name, &StringsHandler::transient_string_memory);
        file_namenoext = AppendString(*file_namenoext, *CreateStringFromLiteral(".mat", &StringsHandler::transient_string_memory), &StringsHandler::transient_string_memory);
        Yostr* path_to_meta_file = AppendStringToChar(game_data_meta_dir,*file_namenoext,&StringsHandler::transient_string_memory);
        read_file_result meta_file_result = PlatformReadEntireFile(path_to_meta_file);
        result = CreateStringFromLength((char*)meta_file_result.Content,meta_file_result.ContentSize,&StringsHandler::transient_string_memory);
//ok as long as we are an ascii string//1 byte = 1 character
        return result;
    }
    
    Yostr* CreateDefaultModelMetaFile(Yostr* filepath,ModelAsset* model)
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

            Value materials_json(kArrayType);

            for(int i = 0;i < model->meshes.count;++i)
            {
                MeshAsset* ma = (MeshAsset*)model->meshes.base + i;
                Value obj(kObjectType);
                Value val(kObjectType);
                val.SetString(ma->name.String,(SizeType)ma->name.Length,allocator);
                obj.AddMember("name",val,allocator);

                //white as default color
                rapidjson::Value float_4_as_array(rapidjson::kArrayType);
                float_4_as_array.PushBack(1,allocator).PushBack(1,allocator).PushBack(1,allocator).PushBack(1,allocator);
                obj.AddMember("base_color",float_4_as_array,allocator);

                rapidjson::Value texture_array(rapidjson::kArrayType);
                Value tex_val(kObjectType);
//                tex_val.SetString(" ",(SizeType)8,allocator);
//                texture_array.PushBack(tex_val,allocator);
                obj.AddMember("textures",texture_array,allocator);

                //TODO(Ray):Shaders shoud just be a path to shader def file
                //with shaader name function names and slots available?
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

                materials_json.PushBack(obj,allocator);
            }
            
            d.AddMember("Materials",materials_json , allocator);

            // Convert JSON document to string
            rapidjson::StringBuffer strbuf;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(strbuf);
            d.Accept(writer);
            const char* output = strbuf.GetString();
            uint32_t length = String_GetLength_Char((char*)output);

            Yostr filename = GetFilenameFromPath(filepath,&StringsHandler::transient_string_memory);
            Yostr* filenamenoext = StripExtension(&filename,&StringsHandler::transient_string_memory);
            Yostr* final_filename = AppendString(*filenamenoext,*CreateStringFromLiteral(".mat",&StringsHandler::transient_string_memory),&StringsHandler::transient_string_memory);
            PlatformFilePointer file{};
            Yostr* final_output_path = AppendStringToChar(game_data_meta_dir,*final_filename,&StringsHandler::transient_string_memory);
            PlatformWriteMemoryToFile(&file,final_output_path->String,(void*)output,length,true,"w+");
            Yostr* result = CreateStringFromLength((char*)output, length, &StringsHandler::transient_string_memory);
            *result = NullTerminate(*result);
            PlatformOutput(log_output,"%s",result->String); 
            return result;
        }
    }
       
    Yostr* GetOrCreateDefaultModelMetaFile(Yostr* file,ModelAsset* model)
    {
        Yostr* result = GetMetaFile(file);
        if(result->Length == 0 && model)//no meta file make one
        {
            Yostr* file_ext = GetExtension(file,&StringsHandler::transient_string_memory,false);
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
