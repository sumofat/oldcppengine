#if !defined(CAMERA_H)

namespace Camera
{
    extern RenderCamera main;
    void Init(float fov = 68,render_camera_project_type type = perspective,float2 near_far_planes = float2(0.05f,1000.0f));
}

#define CAMERA_H
#endif
