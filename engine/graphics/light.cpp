
#ifdef ENGINEIMPL

namespace LightCode
{
    u64 light_index;
    AnythingCache point_light_buffer_cache;
    AnythingCache point_light_property_buffer_cache;    
    GPULightBuffer InitPointLightBuffer()
    {
        GPULightBuffer result = {};
        AnythingCacheCode::Init(&point_light_buffer_cache,4096,sizeof(Light),sizeof(u64),true);
        AnythingCacheCode::Init(&point_light_property_buffer_cache,4096,sizeof(LightProperties),sizeof(u64),true);
        u64 light_buff_size = MAX_LIGHT_COUNT * sizeof(Light);
        u64 light_prop_buff_size = MAX_LIGHT_COUNT * sizeof(LightProperties);
        u32 resource_options = ResourceStorageModeShared;
        result.light = RenderGPUMemory::NewBufferAndUpload(point_light_buffer_cache.anythings.base,light_buff_size,resource_options);
        result.light_prop = RenderGPUMemory::NewBufferAndUpload(point_light_buffer_cache.anythings.base,light_prop_buff_size,resource_options);
        return result;
    }
    
    void Init(AnythingCache* ac)
    {
        Assert(ac->anythings.count == 0);
        light_index = 0;
    }

    void CreatePointLight(f32 attenuation,f32 intensity,float4 color,float3 p)
    {
        f32 point_light_size = attenuation;
        Light result = {};
        LightProperties prop_result = {};
        prop_result.attenuation = attenuation;
        prop_result.intensity = intensity;
        prop_result.color = color;
        prop_result.index = light_index;
        prop_result.p = float4(p,1);
//        prop_result.type = t;
//        prop_result.dir = float4(dir,0); 
//        if (property_result != NULL)
        {
//            *property_result = prop_result;
        }
//        result.type = type;
//        ot.s = float3(1,1,1) * point_light_size;
//        result.ot = ot;
        result.index = light_index;
        light_index++;    
        AnythingCacheCode::AddThingFL(&point_light_buffer_cache,(void*)light_index,(void*)&result);
        AnythingCacheCode::AddThingFL(&point_light_property_buffer_cache,(void*)light_index,(void*)&result);
    }

    LightPackage GetPointLight(u64 id)
    {
        u64 key = id;
        LightPackage result = {};
        Light* l = nullptr;
        LightProperties* lp = nullptr;
        if(AnythingCacheCode::DoesThingExist(&point_light_buffer_cache,(void*)&key))
        {
            l = GetThingPtr(&point_light_buffer_cache,(void*)&id,Light);
        }
        else
        {
            Assert(false); //No light with that ID.
        }
        if(AnythingCacheCode::DoesThingExist(&point_light_buffer_cache,(void*)&key))
        {
            lp = GetThingPtr(&point_light_property_buffer_cache,(void*)&id,LightProperties);
        }
        else
        {
            Assert(false); //No light property with that ID.
        }

        result.l = l;
        result.lp = lp;
        return result;
    }
    
    void RemovePointLight(u64 id)
    {
        u64 key = id;
        if(AnythingCacheCode::DoesThingExist(&point_light_buffer_cache,(void*)&key))
        {
            AnythingCacheCode::RemoveThingFL(&point_light_buffer_cache,(void*)&key);
            AnythingCacheCode::RemoveThingFL(&point_light_property_buffer_cache,(void*)&key);                                
        }
    }

    void CalculateViewportAndOffsets(block_tile_entry_result d,Light*l,LightProperties*lp,u32 max_block_size,int i)
    {
        //NOTE(Ray):always squared dont need for dim
        float2 block_texel = d.block * max_block_size;
        float tile_texel_x = block_texel.x() + (d.tile.z() * d.tile.x());
        float tile_texel_y = block_texel.y() + (d.tile.w() * d.tile.y());
        l->viewports[i] = float4(d.tile.x(), d.tile.y(), tile_texel_x, tile_texel_y);
        l->offsets[i] = float4(d.abs.x(), d.abs.y(),d.tile.z(), d.tile.w());
        lp->absolute_offsets[i] = float4(d.abs.x(),d.abs.y(),0,0);
    }

    void CalculateViewMatrices(Light*l,LightProperties*lp,float3 new_cam_dir,float3 new_up_dir,float4x4 proj_matrix,int i)
    {
        float3 p = lp->p.xyz();
        float4x4 light_view_matrix = float4x4::identity();
        light_view_matrix = set_camera_view(p, new_cam_dir, new_up_dir);
        l->proj_matrix = proj_matrix;
        l->view_matrices[i] = light_view_matrix;
        lp->matrices[i] = mul(light_view_matrix , l->proj_matrix);
    }
    
    void UpdateLight(Light* light,LightProperties* light_prop)
    {
        if(light && light_prop)
        {
            light_prop->p = float4(light->ot.p,1);
            if(light->type == LightType_Point)
            {
                light->ot.s = float3(1,1,1) * (light_prop->attenuation * 2);
                light->range = light_prop->attenuation;
            }
            else if(light->type == LightType_Cone)
            {
                //TODO(Ray):Figure out what light angle should be relative to attenuation.
                light->ot.s = float3((light->angle * 0.75f),(light->angle * 0.75f),light->range * 0.75f);
                float3 d = normalize(up(light->ot.r)) * -1;
                light_prop->dir = float4(d,0);
                light_prop->spot_range_angle = float4(light->range,light->angle,light_prop->spot_range_angle.z(),light_prop->spot_range_angle.w());
            }
        }
        else
        {
            Assert(false);
        }
    }

};

#endif
