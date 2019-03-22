//
//  BasicShaderTypes.h
//  pedaltothemedal
//
//  Created by Ray Olen Garner on 2018/11/09.
//  Copyright © 2018 DENA. All rights reserved.
//

#ifndef BasicShaderTypes_h
#define BasicShaderTypes_h

enum MeshVertexAttributes
{
    mesh_vertices   = 0,
    buffer_uniforms = 2,
    matrix_buffer = 3,
    gl_emu_uniforms = 4,
};

/*
enum TextureInputs
{
    base_color_texture = 0,
    normal_color_texture = 1,
    sprite_atlas_texture = 2,
    base_color_texture_two = 0,
};

enum FullScreenVertexAttributes
{
    fs_vertices     = 0,
    fs_uvs          = 1
};
*/
/*
struct ShaderInputs
{
    float4 base_color;
    float metallic;
    float specular;
    float roughness;
    float dummy;
    //TODO(Ray):Add rest of the inputs later.
};
*/

//TODO(Ray):We will need some concatanation or comiplation system to get materials to be truley modfiable.
// in the sense of arbiturary texture properties.
//For now we need to figure how we will match texture inputs and samplers
//and how we will eventually allow for some arbiturary shader code to run and compile down
//to the shader final inputs.
//for now lets just hardcode and get some texture passed in.
struct ShaderInputs
{
    float4 base_color;
    float metallic;
    float specular;
    float roughness;
    float dummy;
    /*
     float4 emissive_color;
     float opacity;
     float opacity_mask;
     */
    /*
     float3 world_position_offset;
     //    float3 world_displacement;//for use with tesselation not supported at the moment
     //    float tesselation_mulitiplier;
     float4 subsurface_color;
     float ambient_occulsion;//map ??
     float refraction;
     float clear_coat;
     float clear_coat_roughness;
     */
};

struct Uniforms
{
    float4x4 pcm_mat;
    float4x4 world_mat;
    float4 view_p;
    uint32_t matrix_buffer_index;
    uint32_t dummy;
    uint32_t dummy2;
};

struct SpriteUniforms
{
    float4x4 pcm_mat;
    float4x4 world_mat;
    float4 view_p;
    uint32_t matrix_buffer_index;
    uint32_t dummy2;
    uint32_t dummy3;
    uint32_t dummy4;
};

struct SpriteMatricesUniform
{
    float4x4 pcm_mat;
    float4x4 matrices[10];
};

#endif /* BasicShaderTypes_h */
