
#ifndef ShaderTypes_h
#define ShaderTypes_h

enum TextureInputs
{
    base_color_texture = 0,
    normal_color_texture = 1,
    sprite_atlas_texture = 2,
    base_color_texture_two = 0,
};

enum MeshVertexAttributes
{
    mesh_vertices   = 0,
    buffer_uniforms = 3,
    gl_emu_uniforms = 4,
};

enum SpriteVertexAttributes
{
    matrix_buffer = 3,
    index_buffer = 1
};

struct ShaderInputs
{
    float4 base_color;
    float metallic_factor;
    float roughness_factor;
    float2 dummy;
};

struct Uniforms
{
    float4x4 pcm_mat;
    float4x4 world_mat;
    float4 view_p;
    ShaderInputs inputs;
};

struct ShadowMapUniforms
{
    float4x4 pcm_mat;
    float4x4 world_mat;
    float4x4 light_view_matrix;
    float4 view_p;
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

struct MaskSpriteUniforms
{
    float4x4 pcm_mat;
    float4x4 world_mat;
    float4 view_p;
    uint32_t matrix_buffer_index;
    float mask_alpha;
    uint32_t dummy3;
    uint32_t dummy4;
};

struct SpriteMatricesUniform
{
    float4x4 pcm_mat;
    float4x4 matrices[10];
};

#endif /* BasicShaderTypes_h */
