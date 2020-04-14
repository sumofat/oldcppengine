
#if !defined(LIGHT_H)
#define MAX_LIGHT_COUNT 4096
enum LightType
{
    LightType_Point,
    LightType_Cone,
    LightType_Directional
};

//TODO(Ray):
//Its better to have the light to have just a shell of an id to a list of light properties...
//there should be a list of properties for each type of light and we just pass the index of the
//list to use and the index into that list to get teh properties in the shader.

struct Light
{
    u32 index;
    LightType type;
    ObjectTransform ot;
    float4x4 view_matrices[6];
    float4x4 proj_matrix;
    float4 viewports[6];
    float4 offsets[6];
    f32 range;
    f32 angle;
    bool is_dirty;
    bool is_entry;
};

struct LightProperties
{
    float4 color;
    int index;
    f32 attenuation;
    f32 intensity;
    int type;
    float4 p;
    float4 dir;//spot and dir lights only
    float4 spot_range_angle;
    float4 absolute_offsets[6];
    float4x4 matrices[6];
};

struct LightPackage
{
    Light* l;
    LightProperties* lp;
};

struct GPULightBuffer
{
    GPUBuffer light;
    GPUBuffer light_prop;
};

namespace LightCode
{
    extern  AnythingCache point_light_buffer_cache;
    extern  AnythingCache point_light_property_buffer_cache;
    GPULightBuffer InitPointLightBuffer();    
    void Init(AnythingCache* ac);
    void CreatePointLight(f32 attenuation,f32 intensity,float4 color,float3 p);
    LightPackage GetPointLight(u64 id);    
    void RemovePointLight(u64 id);
    void CalculateViewportAndOffsets(block_tile_entry_result t,Light*l,LightProperties*lp,u32 max_block_size);
    void UpdateLight(Light* light,LightProperties* light_prop);
};

#define LIGHT_H
#endif
