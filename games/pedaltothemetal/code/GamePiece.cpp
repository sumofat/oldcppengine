

struct PieceAssets
{
    GLTexture main;
};

struct GamePiece
{
    ObjectTransform ot;
    float3 v;
    bool is_alive;
    bool is_visible;
    bool is_spawn_complete;
    PieceAssets assets;
    GLTexture texture;
    GLProgram program;
    RigidBodyDynamic rbd;
    int life_bars_count = 1;
    ObjectTransform life_ot[10];
    GLTexture life_bar_texture;
};

/*
Scene* game_scene;
namespace GameSceneHandler
{
#define MAX_SCENES 10
//    SceneObjectBuffer* game_scene_buffer;
//    SceneBuffer current_scene_buffer;
    int scene_id;
    void Init()
    {
        scene_id = 0;//first scene is always 0
        SceneCode::InitScene(&current_scene_buffer,MAX_SCENES);
        game_scene = SceneCode::CreateEmptyScene(&current_scene_buffer);
        game_scene_buffer = &game_scene->scene_object_buffer;
    }
};
*/

namespace GamePieceCode
{

    GamePiece game_pieces[32];
    
#define MAX_GAME_PIECE 1
    uint32_t next_open_index;
    uint32_t alive_count;
    float2 dim;
    
//TODO(Ray):Allow to pass in the reference to the sprite in the atlas array
    void Init(GamePiece* piece,float3 start_p,float size_in_pixels,float life_size_in_pixels)
    {
        //Get screen dim from renderer.
        //for(int i = 0;i < MAX_GAME_PIECE;++i)
        {
            GamePiece* a = piece;
            a->ot.p = start_p;
            a->ot.r = quaternion::identity();
            a->ot.s = float3(size_in_pixels);
            a->v = float3(0);
            a->is_alive = false;
            a->is_visible = false;
            a->life_bars_count = 1;
            a->life_ot[0].p = start_p;
            a->life_ot[0].r = quaternion::identity();
            a->life_ot[0].s = float3(life_size_in_pixels);
        }
    }
    
/*    
    void SetNextIndex()
    {
        uint32_t wrapped_index = next_open_index;
        for(int i = 0;i < MAX_GAME_PIECE;++i)
        {
            wrapped_index = (wrapped_index + 1) % MAX_GAME_PIECE;
            GamePiece* a = &game_pieces[wrapped_index];
            if(!a->is_alive)
            {
                next_open_index = wrapped_index;
                break;
            }
        }
        //TODO(Ray):If we didnt find any open define wether should we wrap around or reject adding?
    }
    void Spawn()
    {
        GamePiece* a = &game_pieces[next_open_index];
        a->is_alive = true;
        a->sp->is_visible = true;
//        float screen_ratio = half_dim.x()/half_dim.y();//Not exact but close enough to make sure they spawn of screen.
//        float3 rand_dir_speed = normalize(float3(sin(radians(spawn_angle)),cos(radians(spawn_angle)),0)) * screen_ratio;
//        spawn_angle += 360/100;
        float3 d = float3(0);//float3(rand_dir_speed.xy() * half_dim,0);
        a->sp->so->ot.p = a->sp->so->ot.p + d;
        float Speed = 10;
        a->v = float3(0);//-rand_dir_speed * 100;
        alive_count++;
        SetNextIndex();
    }
*/
    
    bool InBoundsCheck(float3 p,float2 dim)
    {
        if(p.x() < dim.x() &&
           p.y() < dim.y() &&
           p.x() > 0 &&
           p.y() > 0)
        {
            return true;
        }
        return false;
    }

    bool OutBoundsCheck(float3 p,float2 dim)
    {
        if(p.x() > dim.x() ||
           p.y() > dim.y() ||
           p.x() < 0 ||
           p.y() < 0)
        {
            return true;
        }
        return false;
    }
    
    void Update(GamePiece* pieces,float d_time)
    {
        for(int i = 0;i < MAX_GAME_PIECE;++i)
        {
            GamePiece* a = &pieces[i];
            if(a->is_alive)
            {
//                SceneObject* so = a->sp->so;
                float3 final_p = a->ot.p;
                float3 final_s = a->ot.s;
                quaternion final_r = a->ot.r;
                if(OutBoundsCheck(a->ot.p,dim) &&
                   a->is_spawn_complete)
                {
                    //wrap around
                    //distance from screen center
                    float2 dx = abso(final_p.xy() - dim);
                    final_p = float3(dx,0);
                }
                else if(!a->is_spawn_complete &&
                        InBoundsCheck(final_p,dim))
                {
                    a->is_spawn_complete = true;
                }
                //assuming 60fps since we dont have timing info in this teset app.
                //this will obviously look like shit if you dont have 60fps
                final_p += a->v * d_time;
                final_r = quaternion::identity();
//                SceneObjectCode::UpdateSceneObject(so,final_p,final_r,final_s);
            }
        }        
    }

    void SetPieces(PhysicsScene scene,GLProgram program,LoadedTexture lt,LoadedTexture life_lt,PhysicsMaterial material,float2 dim,bool init = false)
    {
        //Initialize gfraphics for the pieces
        float piece_width_in_pixels = 110;
        float2 bottom_left_p = float2(piece_width_in_pixels,piece_width_in_pixels);
        float2 top_left_p = float2(piece_width_in_pixels,dim.y() - piece_width_in_pixels);
    
        float2 center_p = RendererCode::dim * 0.5f;
        int c_i = 0;

        for(int o = 0;o < 2;++o)
        {
            //black pieces
            float2 start_pos = float2(0.0f);
            if(o == 0)
                start_pos = bottom_left_p;
            else
                start_pos = top_left_p;
        
            float2 current_pos = start_pos;
            for(int i = 0;i < 16;++i,++c_i)
            {
//Initialize physics for the piece
                float2 next_p = current_pos;
                if(init)
                {
                    GamePieceCode::Init(&GamePieceCode::game_pieces[c_i],float3(next_p,0),lt.dim.x(),life_lt.dim.x());
                    PhysicsShapeSphere sphere_shape = PhysicsCode::CreateSphere(GamePieceCode::game_pieces[c_i].ot.s.x() / 2,material);
                    RigidBodyDynamic rbd = PhysicsCode::CreateDynamicRigidbody(float3(next_p,0), sphere_shape.shape, false);
                    PhysicsCode::AddActorToScene(scene, rbd);
                    PhysicsCode::DisableGravity(rbd.state,true);
                    GamePieceCode::game_pieces[c_i].program = program;
                    
                    GamePieceCode::game_pieces[c_i].texture = OpenGLEmu::TexImage2D(lt.texels, lt.dim, PixelFormatRGBA8Unorm, OpenGLEmu::GetDefaultDescriptor(), TextureUsageShaderRead);
                    GamePieceCode::game_pieces[c_i].life_bar_texture = OpenGLEmu::TexImage2D(life_lt.texels, life_lt.dim, PixelFormatRGBA8Unorm, OpenGLEmu::GetDefaultDescriptor(), TextureUsageShaderRead);
                    
                    GamePieceCode::game_pieces[c_i].texture.sampler = OpenGLEmu::GetDefaultSampler();
                    GamePieceCode::game_pieces[c_i].rbd = rbd;
                    PhysicsCode::UpdateRigidBodyMassAndInertia(GamePieceCode::game_pieces[c_i].rbd,1);
                    PhysicsCode::SetMass(GamePieceCode::game_pieces[c_i].rbd,1);

                }
                else
                {
                    RigidBodyDynamic rbd = GamePieceCode::game_pieces[c_i].rbd;
                }

            
                if(0 == (i + 1) % 8)
                {
                    if(o == 0)
                        current_pos += float2(0,piece_width_in_pixels);
                    else
                        current_pos -= float2(0,piece_width_in_pixels);
                    current_pos = float2(start_pos.x(),current_pos.y());
                }
                else
                {
                    current_pos += float2(piece_width_in_pixels,0);
                }
            }
        }

    
    }
    
};//ENd name space

