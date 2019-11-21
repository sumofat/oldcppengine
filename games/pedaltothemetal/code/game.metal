
//
//  Shaders.metal
//  pedaltothemetalios
//
//  Created by Ray Olen Garner on 2018/07/07.
//  Copyright © 2018 DENA. All rights reserved.
//

// File for Metal kernel and shader functions

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

// Including header shared between this Metal shader code and Swift/C code executing Metal API commands
#import "ShaderTypes.h"

typedef struct
{
    float3 p [[attribute(0)]];
    float3 n [[attribute(1)]];
    float2 uv[[attribute(2)]];
} Vertex;

typedef struct
{
    float3 p [[attribute(0)]];
    float2 uv [[attribute(1)]];
} FullScreenVertex;

typedef struct
{
    float4 p [[position]];
    float4 n;
    float2 uv;
    float4 frag_p;
    float4 view_p;
} ColorInOut;

typedef struct
{
    float4 p [[position]];
    float2 uv;
} FullScreenColorInOut;

vertex FullScreenColorInOut fullscreen_vs(FullScreenVertex in [[stage_in]])
{
    FullScreenColorInOut out;
    out.p = float4(in.p, 1.0) ;
    out.uv = in.uv;
    return out;
}

fragment float4 fullscreen_fs(FullScreenColorInOut in [[stage_in]])
{
    return float4(0.4f,1,0,1);
}

struct Light
{
    float4 p;
    float4 color;
    float intensity;
    float attenuation;
};

//function for brdf
constant const float PI = 3.14159265358979323846;
float sqr(float x) { return x*x; }

///test variables
constant const float3 baseColor = float3(1.0, 0.0, 0.0);
constant float3 metallic = float3(0, 0.0, 0.0);
constant float3 subsurface = float3(0, 0.0, 0.0);
constant float3 specular = float3(0.0, 0.0, 0.0);
constant float roughness = 0.6f;//float3(0, 1, 0.5);
constant float3 specularTint = float3(0, 0.0, 0.0);
constant float anisotropic = 1;//float3(0, 1, 0);
constant float3 sheen = float3(0, 0.0, 0.0);
constant float3 sheenTint = float3(0, 0.0, 0.0);
constant float3 clearcoat = float3(0, 0.0, 0.0);
constant float clearcoatGloss = 0.0;//float3(0, 1, 1);
//end test variables

float SchlickFresnel(float u)
{
    float m = clamp((float)1-u, (float)0, (float)1);
    float m2 = m*m;
    return m2*m2*m; // pow(m,5)
}

float GTR1(float NdotH, float a)
{
    if (a >= 1) return 1/PI;
    float a2 = a*a;
    float t = 1 + (a2-1)*NdotH*NdotH;
    return (a2-1) / (PI*log(a2)*t);
}

float GTR2(float NdotH, float a)
{
    float a2 = a*a;
    float t = 1 + (a2-1)*NdotH*NdotH;
    return a2 / (PI * t*t);
}

float GTR2_aniso(float NdotH, float HdotX, float HdotY, float ax, float ay)
{
    return 1 / (PI * ax*ay * sqr( sqr(HdotX/ax) + sqr(HdotY/ay) + NdotH*NdotH ));
}

float smithG_GGX(float NdotV, float alphaG)
{
    float a = alphaG*alphaG;
    float b = NdotV*NdotV;
    return 1 / (NdotV + sqrt(a + b - a*b));
}

float smithG_GGX_aniso(float NdotV, float VdotX, float VdotY, float ax, float ay)
{
    return 1 / (NdotV + sqrt( sqr(VdotX*ax) + sqr(VdotY*ay) + sqr(NdotV) ));
}

float3 mon2lin(float3 x)
{
    return float3(pow(x[0], 2.2), pow(x[1], 2.2), pow(x[2], 2.2));
}

float3 BRDF( float3 L, float3 V, float3 N, float3 X, float3 Y )
{
    float NdotL = dot(N,L);
    float NdotV = dot(N,V);
    if (NdotL < 0 || NdotV < 0) return float3(0);
    
    float3 H = normalize(L+V);
    float NdotH = dot(N,H);
    float LdotH = dot(L,H);
    
    float3 Cdlin = mon2lin(baseColor);
    float Cdlum = .3*Cdlin[0] + .6*Cdlin[1]  + .1*Cdlin[2]; // luminance approx.
    
    float3 Ctint = Cdlum > 0 ? Cdlin/Cdlum : float3(1); // normalize lum. to isolate hue+sat
    float3 Cspec0 = mix(specular*.08*mix(float3(1), Ctint, specularTint), Cdlin, metallic);
    float3 Csheen = mix(float3(1), Ctint, sheenTint);
    
    // Diffuse fresnel - go from 1 at normal incidence to .5 at grazing
    // and mix in diffuse retro-reflection based on roughness
    float FL = SchlickFresnel(NdotL), FV = SchlickFresnel(NdotV);
    float Fd90 = 0.5f + 2 * LdotH * LdotH * roughness;
    float Fd = mix(1.0, Fd90, FL) * mix(1.0, Fd90, FV);
    
    // Based on Hanrahan-Krueger brdf approximation of isotropic bssrdf
    // 1.25 scale is used to (roughly) preserve albedo
    // Fss90 used to "flatten" retroreflection based on roughness
    float Fss90 = LdotH*LdotH*roughness;
    float Fss = mix(1.0, Fss90, FL) * mix(1.0, Fss90, FV);
    float ss = 1.25 * (Fss * (1 / (NdotL + NdotV) - .5) + .5);
    
    // specular
    float aspect = sqrt(1-anisotropic*.9);
    float ax = max(.001, sqr(roughness)/aspect);
    float ay = max(.001, sqr(roughness)*aspect);
    float Ds = GTR2_aniso(NdotH, dot(H, X), dot(H, Y), ax, ay);
    float FH = SchlickFresnel(LdotH);
    float3 Fs = mix(Cspec0, float3(1), FH);
    float Gs;
    Gs  = smithG_GGX_aniso(NdotL, dot(L, X), dot(L, Y), ax, ay);
    Gs *= smithG_GGX_aniso(NdotV, dot(V, X), dot(V, Y), ax, ay);
    
    // sheen
    float3 Fsheen = FH * sheen * Csheen;
    
    // clearcoat (ior = 1.5 -> F0 = 0.04)
    float Dr = GTR1(NdotH, mix(.1,.001,clearcoatGloss));
    float Fr = mix(.04, 1.0, FH);
    float Gr = smithG_GGX(NdotL, .25) * smithG_GGX(NdotV, .25);
    
    return ((1/PI) * mix(Fd, ss, subsurface)*Cdlin + Fsheen)
    * (1-metallic)
    + Gs*Fs*Ds + .25*clearcoat*Gr*Fr*Dr;
}

ShaderInputs GetDefaultInputs()
{
    ShaderInputs result;
    result.base_color = float4(1,1,1,1);
    //result.metallic = 0.3f;
   // result.specular = 0.5f;
   // result.roughness = 0.3f;
}

Light GetDefaultLight()
{
    Light result;
    result.intensity = 1.0f;
    result.color = float4(1,1,1,1);
    result.p = float4(0.0f,200.0f,-300.0f,1);
    return result;
}

float4 DoBRDF(float4 frag_p,float4 normal,float4 view_p,float4 surface_color)
{
    Light light  = GetDefaultLight();
    float3 l = normalize(light.p - frag_p).xyz;
    float3 v = normalize(view_p - frag_p).xyz;
    float3 n = normal.xyz;
    return float4(BRDF(l,v,n,0,0),1);
}
float4 GetAmbience()
{
    return float4(0.3f);
}

float4 DoDefaultSpecularLighting(float4 frag_p,float4 normal,float4 surface_color,float4 view_p,float4 specular,float shininess)
{
    Light light = GetDefaultLight();
    float3 world_p = frag_p.xyz;
    float3 n = normalize(normal.xyz);
    float3 light_dir = normalize(light.p.xyz - world_p);
    float3 view_dir  = normalize(view_p.xyz  - world_p);
    float ndl = dot(n, light_dir);
    float4 result = float4(0,0,0,0);
    if(ndl > 0)
    {
        float3 reflect_dir = normalize(reflect(-light_dir, n));
        float nd = dot(view_dir, reflect_dir);
        result = pow(max(nd,0.0), shininess);       
    }
    else
    {
         result = float4(0,0,0,0);
    }
    return result * surface_color * light.color * specular;
}

float4 DoDefaultDiffuseLighting(float4 frag_p,float4 normal,float4 surface_color)
{
    Light light = GetDefaultLight();
    float4 world_p = frag_p;
    float3 n = normalize(normal.xyz);
    float3 light_dir = normalize(light.p.xyz - world_p.xyz);
    float3 diffuse_lighting = max(dot(n,light_dir),0.0f);
    return float4(diffuse_lighting,1) * light.color;
}

vertex ColorInOut diffuse_vs(Vertex in [[stage_in]],
                             constant Uniforms& uniforms[[buffer(buffer_uniforms)]])
{
    ColorInOut out;
    float4 position = uniforms.pcm_mat * float4(in.p, 1.0) ;
    out.p = position;
    out.frag_p = uniforms.world_mat * float4(in.p,1.0f);
    out.n = uniforms.world_mat * float4(in.n,0);
    out.uv = in.uv;
    return out;
}

fragment float4 diffuse_fs(ColorInOut in [[stage_in]],
                           constant Uniforms& uniforms[[buffer(buffer_uniforms)]],
                           texture2d<float> base_color_texture[[ texture(base_color_texture) ]])
{
    Light light = GetDefaultLight();
    constexpr sampler base_sampler(address::repeat);
    float4 base_color = base_color_texture.sample(base_sampler, in.uv);
    ShaderInputs inputs;
    inputs.base_color = base_color;
    float4 ambient = GetAmbience();
    float4 diffuse = DoDefaultDiffuseLighting(in.frag_p,in.n,inputs.base_color);
    float4 specular = DoDefaultSpecularLighting(in.frag_p,in.n,inputs.base_color,uniforms.view_p,float4(0.9f,0,0,1),32.0f);
    return (( diffuse + specular) * light.intensity) * inputs.base_color;
}

fragment float4 diffuse_color_fs(ColorInOut in [[stage_in]],
                                 constant Uniforms& uniforms[[buffer(buffer_uniforms)]],
                                 texture2d<float> base_color_texture[[ texture(base_color_texture) ]])
{
    Light light = GetDefaultLight();
//    constexpr sampler base_sampler(address::repeat);
    ShaderInputs inputs;
    inputs.base_color = uniforms.inputs.base_color;
    float4 ambient = GetAmbience();
    float4 diffuse = DoDefaultDiffuseLighting(in.frag_p,in.n,inputs.base_color);
    float4 specular = DoDefaultSpecularLighting(in.frag_p,in.n,inputs.base_color,uniforms.view_p,float4(0.9f),128.0f);
    //    float4 brdf = DoBRDF(in.frag_p,in.n,uniforms.view_p,inputs.base_color);
    //    return ((brdf + ambient) * light.intensity);// * inputs.base_color;
    return ((ambient + diffuse + specular) * light.intensity) * inputs.base_color;
}

vertex ColorInOut carbonfiber_vs(Vertex in [[stage_in]],
                                 constant Uniforms& uniforms[[buffer(buffer_uniforms)]])
{
    ColorInOut out;
    out.p = uniforms.pcm_mat * float4(in.p, 1.0) ;
    out.frag_p = uniforms.world_mat * float4(in.p,1.0f);
    out.n = uniforms.world_mat * float4(in.n,0);
    out.uv = in.uv;
    return out;
}

vertex ColorInOut composite_vs(FullScreenVertex in [[stage_in]])
{
    ColorInOut out;
    out.p = float4(in.p,1);
    out.uv = in.uv;
    return out;
}
// NOTE(Ray):Not compositing anything at the moment but will be.
fragment float4 composite_fs(ColorInOut in[[stage_in]],
                             texture2d<float> base_color_texture[[texture(base_color_texture)]])
{
    constexpr sampler base_sampler(address::repeat);
    float4 base_color = base_color_texture.sample(base_sampler, in.uv);
    return base_color;
}

fragment float4 carbonfiber_fs(ColorInOut in [[stage_in]],
                               constant Uniforms& uniforms[[buffer(buffer_uniforms)]],
                               texture2d<float> base_color_texture[[ texture(base_color_texture) ]])
{
    Light light = GetDefaultLight();
    constexpr sampler base_sampler(address::repeat);
    float4 base_color = base_color_texture.sample(base_sampler, in.uv);
    //ShaderInputs inputs = uniforms.inputs;
    ShaderInputs inputs;
    inputs.base_color = base_color;
    float4 ambient = GetAmbience();
    float4 diffuse = DoDefaultDiffuseLighting(in.frag_p,in.n,inputs.base_color);
    float4 specular = DoDefaultSpecularLighting(in.frag_p,in.n,inputs.base_color,uniforms.view_p,float4(0.5f),32.0f);
    return ((ambient + diffuse + specular) * light.intensity) * inputs.base_color;
}
/*
 fragment float4 Composite(ColorInOut in [[stage_in]],
 constant Uniforms& uniforms[[buffer(buffer_uniforms)]],
 texture2d<float> texture_one[[ texture(base_color_texture) ]],
 texture2d<float> texture_two[[ texture(base_color_texture_two) ]])
 {
 constexpr sampler base_sampler(address::repeat);
 float4 base_color = base_color_texture.sample(base_sampler, in.uv);
 float4 base_color_two = base_color_texture_two.sample(base_sampler, in.uv);
 float4 final_color = base_color + base_color_two;
 }
 */

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
    float4 color [[attribute(1)]];
    float2 uv [[attribute(2)]];
    float2 mask_uv[[attribute(3)]];
} MaskVertexIn;

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

typedef struct
{
    float4 p [[position]];
    float4 color;
    float2 uv;
    uint aindex;
    float2 mask_uv;
} MaskInOut;

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

vertex InOut ccShader_Position_uColor_vert(VertexIn in[[stage_in]],
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

fragment float4 ccShader_Position_uColor_frag(InOut in [[stage_in]])
{
    return in.color;
}

//END SPRITE BATCH SHADERS
vertex InOut ccShader_PositionColorLengthTexture_vert(VertexIn in[[stage_in]],
                                           device const VertexInAtlasIndex* atlas[[ buffer(1) ]],
                                           texture2d<float> diffuse_texture[[ texture(2) ]],
                                           constant SpriteUniforms& gl_uniforms[[buffer(gl_emu_uniforms)]],
                                           constant float4x4* matrix_buffer_param[[buffer(matrix_buffer)]],
                                           uint vid[[vertex_id]])
{
    InOut result;
    float4x4 matrix_buffer_result = gl_uniforms.pcm_mat;
    result.p = matrix_buffer_result * float4(in.p,1.0);
    float4 color = float4(in.color.rgb * in.color.a,in.color.a);
    result.color = color;
    result.uv = in.uv;
    return result;
}

fragment float4 ccShader_PositionColorLengthTexture_frag(InOut in [[stage_in]])
{
    return in.color * step(0.0,1.0 - length(in.uv));
}

//Spritebatch renderer shader code

/*
const char* pVertexShaderCode =
    "                                                           \n\
    uniform mat4 CC_MVPMatrixArray[%d];                         \n\
    attribute vec4 a_position;                                  \n\
    attribute vec2 a_texCoord;                                  \n\
    attribute vec4 a_color;                                     \n\
    attribute float a_index;                                    \n\
    \n\
    #ifdef GL_ES                                                \n\
    varying lowp vec4 v_fragmentColor;                          \n\
    varying mediump vec2 v_texCoord;                            \n\
    #else                                                       \n\
    varying vec4 v_fragmentColor;                               \n\
    varying vec2 v_texCoord;                                    \n\
    #endif                                                      \n\
    \n\
    void main()                                                 \n\
    {                                                           \n\
    gl_Position = CC_MVPMatrixArray[int(a_index)] * a_position;  \n\
	v_fragmentColor = a_color;                                  \n\
	v_texCoord = a_texCoord;                                    \n\
    }                                                           \n\
    ";
    
    const char* pFragmentShaderCode =
    "                                                           \n\
    #ifdef GL_ES                                                \n\
    precision lowp float;                                       \n\
    #endif                                                      \n\
    \n\
    varying vec4 v_fragmentColor;                               \n\
    varying vec2 v_texCoord;                                    \n\
    uniform sampler2D CC_Texture0;                              \n\
    \n\
    void main()                                                 \n\
    {                                                           \n\
    gl_FragColor = v_fragmentColor * texture2D(CC_Texture0, v_texCoord);			\n\
    }                                                           \n\
    ";
    */

//END SPRITE BATCH SHADERS
vertex InOut SpriteBatchRenderer_Vert(VertexIn in[[stage_in]],
                                           texture2d<float> diffuse_texture[[ texture(2) ]],
                                           constant SpriteUniforms& gl_uniforms[[buffer(gl_emu_uniforms)]],
                                           constant float4x4* matrix_buffer[[buffer(matrix_buffer)]],
                                            constant uint* index_buffer[[buffer(index_buffer)]],
                                           uint vid[[vertex_id]])
{
    InOut result;
    uint index = index_buffer[vid / 6];
    //float4x4 matrix_buffer_result = gl_uniforms.pcm_mat;
    float4x4 matrix_buffer_result = matrix_buffer[index];
    result.p = matrix_buffer_result * float4(in.p,1.0);
    float4 color = float4(in.color.rgb * in.color.a,in.color.a);
    result.color = color;
    result.uv = in.uv;
    return result;
}

fragment float4 PositionTextureColor_frag(InOut in [[stage_in]],
                                          texture2d<float> diffuse_texture[[ texture(2) ]],
                                          sampler diffuse_sampler [[sampler(2)]])
{
    return  diffuse_texture.sample(diffuse_sampler,in.uv) * in.color;
}

vertex MaskInOut mask_vert(MaskVertexIn in[[stage_in]],
constant SpriteUniforms& gl_uniforms[[buffer(gl_emu_uniforms)]],
uint vid[[vertex_id]])
{
    MaskInOut result;
    result.p = gl_uniforms.pcm_mat * float4(in.p,1.0);
    result.color = in.color;
    result.uv = in.uv;
    result.mask_uv = in.mask_uv;
    
    return result;
}

fragment float4 mask_frag(MaskInOut in [[stage_in]],
                                          texture2d<float> diffuse_texture[[ texture(2) ]],
                                          texture2d<float> mask_texture[[ texture(3) ]],
					  constant MaskSpriteUniforms& gl_uniforms[[buffer(gl_emu_uniforms)]],
                                          sampler diffuse_sampler [[sampler(2)]],
                                          sampler mask_sampler [[sampler(3)]])
{
  float4 tex_color = diffuse_texture.sample(diffuse_sampler,in.uv) * in.color;
  float4 mask_color = mask_texture.sample(mask_sampler,in.mask_uv);
  float a = tex_color.a + 1.0/256.0;
  float4 final_color = float4(tex_color.r/a , tex_color.g/a , tex_color.b/a , (1.0 - ( 1.0 - mask_color.a ) * gl_uniforms.mask_alpha)* tex_color.a);
  return in.color * final_color;
}

