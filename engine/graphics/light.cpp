

#ifdef ENGINEIMPL
#if 0
namespace LightCode
{
    GPUBuffer InitPointLightBuffer()
    {
        GPUBuffer result = {};
        AnythingCacheCode::Init(&point_light_buffer_cache,4096,sizeof(Light),sizeof(u64),true);
//        point_light_buffer_cache
        
        result = NewBufferAndUpload(point_light_buffer_cache.anythings.base,,uint32_t resource_options);
    }
    
    void Init(AnythingCache* ac)
    {
        Assert(ac->anythings.count == 0);
    }


    void CreatePointLight(AnythingCache* ac,f32 attenuation,f32 intensity,float4 color,float3 p)
    {
        f32 point_light_size = attenuation;
        Light result; = {};
        LightProperties prop_result = {};

//    Light* result = PushStruct(&lights_memory,Light);
//    LightProperties* prop_result = PushStruct(&lights_properties_memory,LightProperties);
        prop_result->attenuation = attenuation;
        prop_result->intensity = intensity;
        prop_result->color = color;
        prop_result->index = light_index;
        prop_result->p = float4(ot.p,1);
        prop_result->type = t;
        prop_result->dir = float4(dir,0); 
        if (property_result != NULL)
        {
            *property_result = prop_result;
        }
        result->type = type;

        ot.s = float3(1,1,1) * point_light_size;
        result->ot = ot;
        result->index = light_index;
        AnythingCacheCode::AddThingFL();    
        light_index++;    
        return result;
    }
};
#endif

#endif
