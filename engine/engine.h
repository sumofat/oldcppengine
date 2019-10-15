#if !defined(ENGINE_H)
//NOTE(Ray):Here will be all the declarations for the engine.

#include "metalizer/RendererInclude.h"

#include "input.h"
#include "platform/PlatformState.h"
#include "graphics/camera.h"

#include "external/imgui/imgui.h"

#include "editor/editorgui.h"
#include "editor/editor.h"

#include "graphics/deffered/deffered.h"

#include "asset/MetaFiles.h"
#include "asset/AssetSystem.h"

#if OSX || IOS
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#pragma clang diagnostic ignored "-Wnull-dereference"
#endif

#define FBXSDK_SHARED
#include <fbxsdk.h>

#if OSX || IOS
#pragma clang diagnostic pop
#endif


#include "metalizer/cgltf/cgltf.h"

#include "physics/physics.h"
#include "audio/SoundInterface.h"
namespace Engine
{
    void Init(float2 window_dim);
    void Update();
}
    
#define ENGINE_H
#endif
