#if !defined(METAFILE_H)

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
    MetaFileType::Type GetFileExtensionType(Yostr* file);
    ShaderValueType::Type GetShaderType(Yostr* type);
}

#define METAFILE_H
#endif
