#if !defined(ENGINE_H)
//NOTE(Ray):Here will be all the declarations for the engine.

#include "yoyo/RendererInclude.h"

#include "input.h"
#include "platform/PlatformState.h"
#include "graphics/camera.h"

#include "external/imgui/imgui.h"

#include "graphics/deffered/deffered.h"

#include "asset/AssetSystem.h"

#include "external/fbx/fbxsdk.h"

#include "external/rapidjson/document.h"
#include "external/rapidjson/writer.h"
#include "external/rapidjson/stringbuffer.h"
#include "external/rapidjson/prettywriter.h"

namespace Engine
{
    
    void Init(float2 window_dim);
    void Update();
}
 

    
#define ENGINE_H
#endif
