
#if !defined(ASSETSYSTEM_H)

namespace AssetSystem
{
#define MAX_GPU_TEXTURE_STORAGE_COUNT 1000//number pulled from random
    //TODO(Ray):Not acutual max sprites lets migrate all max defs for assets here.
#define MAX_SPRITES 1000
    void Init();
    //ModelAsset* LoadModel(char* file_name,PlatformState* ps);
    
    bool AddOrGetTexture(Yostr path,LoadedTexture* result);
}

//#endif

#define ASSETSYSTEM_H
#endif
