#if ENGINEIMPL

namespace Camera
{
    RenderCamera main;

    void Init(float fov,render_camera_project_type type,float2 near_far_planes)
    {
        main.projection_type = type;
        main.fov = fov;
        main.matrix = float4x4::identity();
        main.near_far_planes = near_far_planes;
        main.spot_light_shadow_projection_matrix = init_ortho_proj_matrix(float2(1,1) * 20);
        main.point_light_shadow_projection_matrix = init_pers_proj_matrix(float2(1024,1024),90);
    }
}

#endif
