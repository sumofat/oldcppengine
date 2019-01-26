#if !defined(ENGINE_H)
//NOTE(Ray):Here will be all the declarations for the engine.

#include "yoyo/RendererInclude.h"

#include "input.h"
#include "platform/PlatformState.h"
#include "graphics/camera.h"

#include "external/imgui/imgui.h"

#include "graphics/deffered/deffered.h"

#include "asset/MetaFiles.h"
#include "asset/AssetSystem.h"

#define FBXSDK_SHARED
#include <fbxsdk.h>

namespace Engine
{
    
    void Init(float2 window_dim);
    void Update();
}
 

    
#define ENGINE_H
#endif
