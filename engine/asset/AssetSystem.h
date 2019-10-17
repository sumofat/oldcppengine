
#if !defined(ASSETSYSTEM_H)

struct RenderAssetKey
{
    ModelAsset asset;
};

namespace AssetSystem
{
#define MAX_GPU_TEXTURE_STORAGE_COUNT 1000//number pulled from random
    //TODO(Ray):Not acutual max sprites lets migrate all max defs for assets here.
#define MAX_SPRITES 1000
    void Init();
    //ModelAsset* LoadModel(char* file_name,PlatformState* ps);
    extern RenderMaterial default_mat;    
    bool AddOrGetTexture(Yostr path,LoadedTexture* result);
    bool FBXSDKLoadModel(char* file_path,ModelAsset* result);
    void UploadModelAssetToGPU(ModelAsset* ma);

    Yostr GetDataPath(const char* file,MemoryArena* arena);
    
    RenderMaterial CreateMaterialFromDescription(Yostr* vs_name,Yostr* as_name,float4 base_color);
    RenderMaterial CreateDefaultQuadMaterial();
}

//#endif

#define ASSETSYSTEM_H
#endif
