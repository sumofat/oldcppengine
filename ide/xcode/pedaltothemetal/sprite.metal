
#include <metal_stdlib>
    
#ifdef __METAL_VERSION__
#define NS_ENUM(_type, _name) enum _name : _type _name; enum _name : _type
#define NSInteger metal::int32_t
#else
#import <Foundation/Foundation.h>
#endif

#include <simd/simd.h>
using namespace metal;
    
#ifdef __METAL_VERSION__
#define NS_ENUM(_type, _name) enum _name : _type _name; enum _name : _type
#define NSInteger metal::int32_t
#else
#import <Foundation/Foundation.h>
#endif
    
#include <simd/simd.h>

#include "BasicShaderTypes.h"

struct  VertexInAtlasIndex
{
   uint aindex;
};
    
#define PI_32  3.14159265359f
#define PI_RAD_32 PI_32/180
#define ANGLE_TO_RADIAN * PI_RAD_32

typedef struct
{
    float3 p  [[attribute(0)]];
    float4 color [[attribute(1)]];
    float2 uv [[attribute(2)]];
} VertexIn;

typedef struct
{
    float3 p  [[attribute(0)]];
    uint8_t color [[attribute(1)]];
    float2 uv [[attribute(2)]];
} VertexInQuad;
    
typedef struct
{
    float3 p  [[attribute(0)]];
    float4 color [[attribute(1)]];
    float2 uv [[attribute(2)]];
    uint aindex[[attribute(3)]];
} VertexInMatrixID;
     
typedef struct
{
    float4 p [[position]];
    float4 color;
    float2 uv;
    uint aindex;
} InOut;

vertex InOut spritebatch_vs(VertexIn in [[stage_in]],
                            constant SpriteUniforms& gl_uniforms[[buffer(gl_emu_uniforms)]],
                            uint vid[[vertex_id]])
{
    InOut result;
    result.p = gl_uniforms.pcm_mat * float4(in.p,1.0);
    result.uv = in.uv;
    result.color = in.color;
    return result;
}
    
vertex InOut spritebatch_vs_matrix_indexed_direct(VertexIn in[[stage_in]],
                                           texture2d<float> diffuse_texture[[ texture(2) ]],
                                           constant SpriteUniforms& gl_uniforms[[buffer(gl_emu_uniforms)]],
                                           uint vid[[vertex_id]])
{
    InOut result;
    result.p = gl_uniforms.pcm_mat * float4(in.p,1.0);
    result.color = in.color;
    result.uv = in.uv;
    return result;
}
    
vertex InOut spritebatch_vs_matrix_indexed(VertexIn in[[stage_in]],
                                           device const VertexInAtlasIndex* atlas[[ buffer(1) ]],
                                              texture2d<float> diffuse_texture[[ texture(2) ]],
                                           constant SpriteUniforms& gl_uniforms[[buffer(gl_emu_uniforms)]],
                                              constant float4x4* matrix_buffer_param[[buffer(matrix_buffer)]],
                                           uint vid[[vertex_id]])
{
    InOut result;
    float4x4 matrix_buffer_result = gl_uniforms.pcm_mat;
    result.p = matrix_buffer_result * float4(in.p,1.0);
    result.color = in.color;
    result.uv = in.uv;
    return result;
}
    
vertex InOut spritebatch_vs_test_quad(VertexInQuad in[[stage_in]],
                                           device const VertexInAtlasIndex* atlas[[ buffer(1) ]],
                                              texture2d<float> diffuse_texture[[ texture(2) ]],
                                           constant SpriteUniforms& gl_uniforms[[buffer(gl_emu_uniforms)]],
                                              constant float4x4* matrix_buffer_param[[buffer(matrix_buffer)]],
                                           uint vid[[vertex_id]])
{
    InOut result;
    float4x4 matrix_buffer_result = gl_uniforms.pcm_mat;
    result.p = matrix_buffer_result * float4(in.p,1.0);
    result.color = float4(in.color  / 255);
    result.uv = in.uv;
    return result;
}

#include <simd/simd.h>
    
#define PI_32  3.14159265359f
#define PI_RAD_32 PI_32/180
#define ANGLE_TO_RADIAN * PI_RAD_32

    //NOTE(Ray):ccShader_PositionTextureColor_frag
fragment float4 spritebatch_fs_single(InOut in [[stage_in]],
                                      texture2d<float> diffuse_texture[[ texture(2) ]])
{
    constexpr sampler texture_sampler (coord::normalized);
    float4 albedo = in.color * diffuse_texture.sample(texture_sampler,in.uv);
    return albedo;
}
    
//NOTE(Ray):ccShader_PositionTextureColor_frag
fragment float4 spritebatch_fs_single_sampler(InOut in [[stage_in]],
                                      texture2d<float> diffuse_texture[[ texture(0) ]],
                                              sampler diffuse_sampler [[sampler(0)]])
{
   // constexpr sampler texture_sampler (coord::normalized);
    float4 albedo = in.color * diffuse_texture.sample(diffuse_sampler,in.uv);
    return albedo;
}
    
fragment float4 spritebatch_fs(InOut in [[stage_in]],
             texture2d_array<float> diffuse_texture_array[[ texture(2) ]])
{
    constexpr sampler texture_sampler (coord::normalized);
    uint index = in.aindex;
    float4 albedo = diffuse_texture_array.sample(texture_sampler,in.uv,index);
    //float4 albedo = float4(1,1,1,1);
    //albedo.rgb *= albedo.a;
    return albedo;
}

fragment float4 spritebatch_fs_sampler(InOut in [[stage_in]],
                               texture2d_array<float> diffuse_texture_array[[ texture(2) ]],
                                       sampler diffuse_sampler [[sampler(0)]])
{
    constexpr sampler texture_sampler (coord::normalized);
    uint index = in.aindex;
    float4 albedo = diffuse_texture_array.sample(diffuse_sampler,in.uv,index);
    //float4 albedo = float4(1,1,1,1);
    //albedo.rgb *= albedo.a;
    return albedo;
}

fragment float4 spritebatch_color_fs(InOut in [[stage_in]],
                               texture2d_array<float> diffuse_texture_array[[ texture(2) ]])
{
    //constexpr sampler texture_sampler (coord::normalized);
    //uint index = in.aindex;
    float4 albedo = float4(1,1,1,1);//diffuse_texture_array.sample(texture_sampler,in.uv,index);
    albedo.rgb *= albedo.a;
    return albedo;
}

fragment float4 spritebatch_color_notex_fs(InOut in [[stage_in]],
                               texture2d_array<float> diffuse_texture_array[[ texture(2) ]])
{
    return in.color;
}
    
    
//END SPRITE BATCH SHADERS

