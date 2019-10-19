#if !defined(METAFILE_H)

#include "../external/rapidjson/include/rapidjson/document.h"
#include "../external/rapidjson/include/rapidjson/writer.h"
#include "../external/rapidjson/include/rapidjson/stringbuffer.h"
#include "../external/rapidjson/include/rapidjson/prettywriter.h"
#include "../metalizer/cgltf/cgltf.h"

struct InProgressMetaFile
{
    rapidjson::Document d;
    Yostr file_path;
    rapidjson::Value meshes_json;
    AnythingCache tex_cache;
    ModelAsset model;
    uint32_t mesh_index;
};

namespace MetaFileType
{
    enum Type
    {
        FBX,
        PSD,
        PNG,
//    MetalFileType_JPG,//No thank you can but wont
//    MetalFileType_BANK,
        NONE,//unknown type or not a file
        COUNT
    };
    static const Type as_array[] = {FBX,PSD,PNG,NONE};
};

namespace ShaderValueType
{
    enum Type
    {
        float4,
        float3,
        float2,
        afloat,
        texture,
        unknown,
        COUNT
    };
    static const Type as_array[] = {float4,float3,float2,afloat,texture,unknown};
};

namespace MetaFiles
{
    extern char* game_data_dir;// = "~/work/pedaltothemetal/games/pedaltothemetal/data/";
    extern char* game_data_meta_dir;// = "/Users/ray.garner/work/mar/games/pedaltothemetal/data/metafiles/";

    MetaFileType::Type GetFileExtensionType(Yostr file);
    ShaderValueType::Type GetShaderType(Yostr type);
    
    void StartMetaFileCreation(InProgressMetaFile* mf,Yostr filepath,uint32_t mesh_count);
    void AddMeshToMetaFile(InProgressMetaFile* mf,cgltf_mesh ma);    
    void EndMetaFileCreation(InProgressMetaFile* mf);
}

#define METAFILE_H
#endif
