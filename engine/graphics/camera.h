#if !defined(CAMERA_H)
//NOTE(Ray):
/*
 * ObjectTransform mot;
    mot.p = float3(0,0,0) + viz_move;
    mot.r = axis_angle(float3(1,0,0),90);//quaternion::axis_angle(float3(0,0,1),0);
    mot.s = float3(1);
    YoyoUpdateObjectTransform(&mot);
    float4x4 m_matrix = mot.m;

    
    float3 cam_p = float3(0,0,15);// + viz_move;
    float3 new_p = cam_p;//float3(sin(radians(dummy_inc)) * -5,0,sin(radians(dummy_inc)) * -2) + render_cam_p;
    float3 look_dir = mot.p - new_p;//model_ot[0].p - new_p;
    ObjectTransform cam_ot;
    cam_ot.p = new_p;//render_cam_p;
    cam_ot.r = quaternion::look_rotation(-look_dir,float3(0,1,0));//quaternion::identity();//axis_angle(float3(0,0,1),90);//
    cam_ot.s = float3(1);
    Camera::main.matrix = YoyoSetCameraView(&cam_ot);//set_camera_view(cam_p, float3(0,0,1), float3(0,1,0));
//In this case when the model goes up in y the camera loses track investigate later
 */
namespace Camera
{
    extern RenderCamera main;
    void Init(float fov = 68,render_camera_project_type type = perspective,float2 near_far_planes = float2(0.05f,1000.0f));
}

#define CAMERA_H
#endif
