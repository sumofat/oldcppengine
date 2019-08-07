
struct GamePiece
{
    Sprite* sp;
    float3 v;
    bool is_alive;
    bool is_spawn_complete;
};

Scene* game_scene;
namespace GameSceneHandler
{
#define MAX_SCENES 10
    SceneObjectBuffer* game_scene_buffer;
    SceneBuffer current_scene_buffer;
    int scene_id;
    void Init()
    {
        scene_id = 0;//first scene is always 0
        SceneCode::InitScene(&current_scene_buffer,MAX_SCENES);
        game_scene = SceneCode::CreateEmptyScene(&current_scene_buffer);
        game_scene_buffer = &game_scene->scene_object_buffer;
    }
};


namespace GamePieceCode
{
   
#define MAX_GAME_PIECE 1
    
    GamePiece game_pieces[MAX_GAME_PIECE];
    uint32_t next_open_index;
    uint32_t alive_count;
    float2 dim;
//TODO(Ray):Allow to pass in the reference to the sprite in the atlas array
    void Init(SpriteBatchBuffer* sprite_batch,float3 start_p)
    {
        //Get screen dim from renderer.
        for(int i = 0;i < MAX_GAME_PIECE;++i)
        {
            GamePiece* a = &game_pieces[i];
            YoyoSpriteBatchRenderer::InitSprite(&GameSceneHandler::game_scene_buffer,sprite_batch,&a->sp,start_p,quaternion::identity(),float3(50));
            a->v = float3(0);
            a->is_alive = false;
            a->sp->atlas_entry_id = 1;
            a->sp->is_visible = false;
        }
    }
    
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
    
    void Update(float d_time,SpriteBatchBuffer* sprite_batch)
    {
        for(int i = 0;i < MAX_GAME_PIECE;++i)
        {
            GamePiece* a = &game_pieces[i];
            if(a->is_alive)
            {
                SceneObject* so = a->sp->so;
                float3 final_p = so->ot.p;
                float3 final_s = so->ot.s;
                quaternion final_r = so->ot.r;
                if(OutBoundsCheck(so->ot.p,dim) &&
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
                SceneObjectCode::UpdateSceneObject(so,final_p,final_r,final_s);
            }
        }        
    }
}

