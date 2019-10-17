
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

#import "../../../engine/metalizer/BasicShaderTypes.h"//metalizer builtin
#import "ShaderTypes.h"

using namespace metal;

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
    float m = clamp((float)1-u, 0.0, 1.0);
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

float D_GGX(float NoH, float a) {
    float a2 = a * a;
    float f = (NoH * a2 - NoH) * NoH + 1.0;
    return a2 / (PI * f * f);
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
    result.metallic_factor = 0.3f;
    result.roughness_factor = 0.5f;
}

Light GetDefaultLight()
{
    Light result;
    result.intensity = 20.0f;
    result.color = float4(1,1,1,1);
    result.p = float4(0.0f,35.0f,0.0f,1);
    return result;
}

float4 GetAmbience()
{
    return float4(0.0f);
}

float4 DoDefaultSpecularLighting(float4 frag_p,float4 normal,float4 surface_color,float4 view_p,float4 specular,float shininess)//normal is assumed to be pre normalzied
{
    Light light = GetDefaultLight();
    float4 light_dir = normalize(frag_p - light.p);
    float4 view_dir = normalize(view_p - frag_p);
    float4 reflect_dir = reflect(-light_dir, normal);
    float4 result = pow(max(dot(view_dir, reflect_dir), 0.0), shininess);
    return result * specular * light.color;
}

float4 DoDefaultDiffuseLighting(float4 frag_p,float4 normal,float4 surface_color)//normal is assumed to be pre normalzied
{
    Light light = GetDefaultLight();
    float4 object_color = surface_color;
    float4 world_p = frag_p;
    float4 light_dir = normalize(light.p - world_p);
    float4 diffuse_lighting = max(dot(light_dir,normal),0.0f) * light.color;
    return diffuse_lighting;
}

vertex ColorInOut diffuse_vs(Vertex in [[stage_in]],
                             constant Uniforms& uniforms[[buffer(3)]])
{
    ColorInOut out;
    float4 position = uniforms.pcm_mat * float4(in.p, 1.0) ;
    out.p = position;
    out.frag_p = uniforms.world_mat * float4(in.p,1.0);
    out.n = uniforms.world_mat * float4(in.n,0);
    out.uv = in.uv;
    return out;
}

fragment float4 diffuse_fs(ColorInOut in [[stage_in]],
                           constant Uniforms& uniforms[[buffer(3)]],
                           texture2d<float> base_color_texture[[ texture(0) ]])
{
    Light light = GetDefaultLight();
    constexpr sampler base_sampler(address::repeat);
    float4 base_color = base_color_texture.sample(base_sampler, in.uv);
    ShaderInputs inputs;
    inputs.base_color = base_color;
    float4 ambient = GetAmbience();
    float4 diffuse = DoDefaultDiffuseLighting(in.frag_p,in.n,inputs.base_color);
    float4 specular = DoDefaultSpecularLighting(in.frag_p,in.n,inputs.base_color,uniforms.view_p,float4(0.9f),128.0f);
    return ((ambient + diffuse + specular) * light.intensity) * inputs.base_color;
}

float Fd_Lambert() {
    return 1.0 / PI;
}

float4 PDMBRDF(float3 l,float3 v,float3 n,float3 h,ShaderInputs inputs)
{
    Light light  = GetDefaultLight();
    
    float NoV = abs(dot(n, v)) + 1e-5;
    float NoL = clamp(dot(n, l), 0.0, 1.0);
    float NoH = clamp(dot(n, h), 0.0, 1.0);
    float LoH = clamp(dot(l, h), 0.0, 1.0);

    float LoN = clamp(dot(l, n), 0.0, 1.0);
    //float D = D_GGX(NoH, a);
    // perceptually linear roughness to roughness (see parameterization)
    float roughness = inputs.roughness_factor * inputs.roughness_factor;
    float4 Fd = inputs.base_color * Fd_Lambert();
    
    float illuminance = light.intensity * LoN;
    float4 luminance = Fd * illuminance;

    return luminance;
}

float4 DoBRDF(float4 frag_p,float4 normal,float4 view_p,ShaderInputs inputs)
{
    Light light  = GetDefaultLight();
    float3 l = normalize(light.p.xyz - frag_p.xyz);
    float3 v = normalize(view_p.xyz - frag_p.xyz);
    float3 n = normal.xyz;
    float3 h = normalize(v + l);

    return PDMBRDF(l,v,n,h,inputs);

//    float4 df = DoDefaultDiffuseLighting(frag_p,normal,inputs.base_color) * light.intensity;//normal is assumed to be pre normalzied
//    return df;
}

fragment float4 diffuse_color_fs(ColorInOut in [[stage_in]],
                           constant Uniforms& uniforms[[buffer(3)]],
                           texture2d<float> base_color_texture[[ texture(0) ]])
{
    Light light = GetDefaultLight();
    constexpr sampler base_sampler(address::clamp_to_zero);
    float4 base_color = base_color_texture.sample(base_sampler, in.uv) * uniforms.inputs.base_color;
    ShaderInputs inputs;
    inputs.base_color = base_color;
    float4 ambient = GetAmbience();
    float4 brdf = DoBRDF(in.frag_p,in.n,uniforms.view_p,inputs);
    return brdf + ambient;
}

vertex ColorInOut carbonfiber_vs(Vertex in [[stage_in]],
                             constant Uniforms& uniforms[[buffer(3)]])
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
                                 texture2d<float> base_color_texture[[texture(0)]])
{
    constexpr sampler base_sampler(address::repeat);
    float4 base_color = base_color_texture.sample(base_sampler, in.uv);
    return base_color;
}

fragment float4 carbonfiber_fs(ColorInOut in [[stage_in]],
                               constant Uniforms& uniforms[[buffer(3)]],
                               texture2d<float> base_color_texture[[ texture(0) ]])
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
                          constant Uniforms& uniforms[[buffer(3)]],
                          texture2d<float> texture_one[[ texture(base_color_texture) ]],
                          texture2d<float> texture_two[[ texture(base_color_texture_two) ]])
{
    constexpr sampler base_sampler(address::repeat);
    float4 base_color = base_color_texture.sample(base_sampler, in.uv);
    float4 base_color_two = base_color_texture_two.sample(base_sampler, in.uv);
    float4 final_color = base_color + base_color_two;
}
*/
