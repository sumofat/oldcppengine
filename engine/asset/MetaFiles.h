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

namespace MetaFiles
{
    MetaFileType::Type GetFileExtensionType(Yostr* file);
}

#define METAFILE_H
#endif
