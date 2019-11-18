
#include "engineapi.h"
#include "GamePiece.cpp"
#include "GamePiecePhysics.cpp"

bool game_log = false;

//TestAssets
static SoundClip bgm_soundclip;

//Threading
TicketMutex ticket_mutex;

//Game Specific
SpriteBatchBuffer sprite_batch_buffer;
//GLEMU TEST STUFF
//GLProgram gl_program;
//uint64_t gl_program_handle;
RenderCamera ortho_cam;
PhysicsScene scene;
PhysicsMaterial material;

void* LoadTextureThread(void* data)
{
    BeginTicketMutex(&ticket_mutex);
   
    //if(!OpenGLEmu::GLIsValidTexture(tex))
    {
        //tex = OpenGLEmu::TexImage2D(lt.texels, lt.dim, PixelFormatRGBA8Unorm, OpenGLEmu::GetDefaultDescriptor(), TextureUsageShaderRead);
        
        //tex.sampler = OpenGLEmu::GetDefaultSampler();
        //YoyoStretchPushBack(&pushed_textures,tex);
    }

    EndTicketMutex(&ticket_mutex);
    return 0;
}

void GameDebugMenuCallbackIMGUI()
{
//Inisde a Begin menu call
    if(ImGui::MenuItem("Game Debug"))
    {
//Select piece change its material properties
    }
    ImGui::EndMenu();
    
    PlatformOutput(true,"GAMEDEBUGCALLBCK");
}

extern "C" void gameInit()
{
    PlatformOutput(game_log,"Game Init Start\n");
//Init things    
    Yostr base_path_to_data = BuildPathToAssets(&StringsHandler::string_memory, Directory_None);
    ticket_mutex = {};

//INit Audio
    char* file_name = "Master.strings.bank";
    Yostr mat_final_path = AppendString(base_path_to_data, CreateStringFromLiteral(file_name,&StringsHandler::transient_string_memory), &StringsHandler::transient_string_memory);
        
    SoundAssetCode::CreateSoundBank(mat_final_path.String);
        
    char* bsfile_name = "Master.bank";
    mat_final_path = AppendString(base_path_to_data, CreateStringFromLiteral(bsfile_name,&StringsHandler::transient_string_memory), &StringsHandler::transient_string_memory);
    SoundAssetCode::CreateSoundBank(mat_final_path.String);
        
    char* afile_name = "bgm.bank";
    mat_final_path = AppendString(base_path_to_data, CreateStringFromLiteral(afile_name,&StringsHandler::transient_string_memory), &StringsHandler::transient_string_memory);
    SoundAssetCode::CreateSoundBank(mat_final_path.String);
    SoundAssetCode::LoadBankSampleData();
    SoundAssetCode::GetBus("bus:/bgmgroup");
    SoundAssetCode::GetEvent("event:/bgm",&bgm_soundclip);

    //Init Graphics
    float2 dim = RendererCode::dim;
    ortho_cam.projection_type = screen_space;
    ortho_cam.matrix = float4x4::identity();
    ortho_cam.projection_matrix = init_screen_space_matrix(dim);
    ortho_cam.near_far_planes = float2(0.1f, 1000.0f);

    //TEST GLEMU
    LoadedTexture lt;
    LoadedTexture life_lt;
        
    OpenGLEmu::CreateBufferAtBinding(0);
    Yostr tpath     = AssetSystem::GetDataPath("koma.png",&StringsHandler::transient_string_memory);
    Yostr life_path = AssetSystem::GetDataPath("life.png",&StringsHandler::transient_string_memory);
    
    Resource::GetImageFromDisk(tpath.String, &lt,4);
    Resource::GetImageFromDisk(life_path.String, &life_lt,4);
    
    VertexDescriptor vd = SpriteBatchCode::CreateDefaultSpriteBatchDescriptor();
    GLProgram program = OpenGLEmu::AddProgramFromMainLibrary("spritebatch_vs","spritebatch_fs_single_sampler",vd);
    //ENDGLEMU TEST

//Initialize physics for the scene.
    material = PhysicsCode::CreateMaterial(0.0f, 0.0f, 1.0f);
    PhysicsCode::SetRestitutionCombineMode(material,physx::PxCombineMode::eMIN);
    PhysicsCode::SetFrictionCombineMode(material,physx::PxCombineMode::eMIN);
    scene = PhysicsCode::CreateScene(PhysicsCode::DefaultFilterShader);
    GamePiecePhysicsCallback* e = new GamePiecePhysicsCallback();
    PhysicsCode::SetSceneCallback(&scene, e);
    GamePieceCode::SetPieces(scene, program, lt,life_lt, material, dim,true);

    EditorGUI::game_callback = GameDebugMenuCallbackIMGUI;
//Init the audio assets for the pieces.
    //bounce wall
    //bounce piece
    //push
    //sliding
    //damamage
    //win
    //lose
     
    PlatformOutput(game_log,"Game Init Complete\n");
}

bool prev_lmb_state = false;
struct FingerPull
{
    float2 start_pull_p;
    float2 end_pull_p;
    float pull_strength;
    bool pull_begin = false;
    GamePiece* gp;
};

static FingerPull finger_pull = {};

bool CheckValidFingerPull(FingerPull* fp)
{
    Assert(fp);
    fp->pull_strength = abs(length(fp->start_pull_p - fp->end_pull_p));
    if(fp->pull_strength > 2)
    {
        return true;
    }
    fp->pull_strength = 0;
    return false;
}

float ground_friction = 0.991f;
//Frame Update
extern "C" void gameUpdate()
{
    PlatformOutput(game_log,"Game Update start\n");
//Update things
        float2 window_dim = RendererCode::dim;
//GetInput
    Input input = EngineInput::GetInput();
    if(input.mouse.lmb.down)
    {
        //Begin the pull
        finger_pull.pull_begin = true;
        if(!finger_pull.gp)
        {
            //find the hovered unit
            PxScene* cs = scene.state;
            //PxVec3 origin = screen_to_world_point(ortho_cam.projection_matrix,ortho_cam.matrix,window_dim,input.mouse.p,0).toPhysx();
            PxVec3 origin = PxVec3(input.mouse.p.x(),window_dim.y() - input.mouse.p.y(),0);
            PxVec3 unitDir = float3(0,0,-1).toPhysx();// [in] Normalized ray direction
            PxReal maxDistance = 5.0f;                // [in] Raycast max distance
            PxRaycastBuffer hit;                      // [out] Raycast results
            // Raycast against all static & dynamic objects (no filtering)
            // The main result from this call is the closest hit, stored in the 'hit.block' structure
            bool status = cs->raycast(origin, unitDir, maxDistance, hit);
            if(status)
            {
                for (int i = 0; i < 32; ++i)
                {
                    //For each piece check inside sphere radius if start touch is in side piece radius
                    GamePiece* gp = &GamePieceCode::game_pieces[i];
                   if( hit.block.actor == gp->rbd.state)
                   {
                       int a =0;
                       finger_pull.gp = gp;
                       finger_pull.start_pull_p = input.mouse.p;
                       break;
                   }
                    //set gp
                }
            }
        }
        //if no unit hovered ignore pull
        //else show pull graphic over piece
    }
    else
    {

        if(finger_pull.pull_begin)
        {
            finger_pull.end_pull_p  = input.mouse.p;
            if(!CheckValidFingerPull(&finger_pull))
            {
                finger_pull.pull_begin = false;
            }
            if(finger_pull.pull_begin && finger_pull.gp)
            {
                //Pull was valid
                float3 flick_dir = float3(finger_pull.end_pull_p - finger_pull.start_pull_p,0);
                flick_dir = float3(-flick_dir.x(),flick_dir.y(),0);
                finger_pull.gp->rbd.state->setLinearVelocity(flick_dir.toPhysx());
                finger_pull.gp = nullptr;
                finger_pull.start_pull_p = float2(0,0);
                finger_pull.end_pull_p = float2(0,0);
                finger_pull.pull_begin = false;
            }
        }
    }
    //Set friction on moving bodies
    
    prev_lmb_state = input.mouse.lmb.down;
//Graphics
    uint32_t current_count = 1; 
    TripleGPUBuffer* v_buffer = OpenGLEmu::GetBufferAtBinding(0);
    uint32_t bi = OpenGLEmu::current_buffer_index;
    for(int i = 0;i < 32;++i)
    {
        //Physics
        PxVec3 lv = GamePieceCode::game_pieces[i].rbd.state->getLinearVelocity();
        lv *= ground_friction;
        GamePieceCode::game_pieces[i].rbd.state->setLinearVelocity(lv);
        //Graphics
        OpenGLEmu::UseProgram(GamePieceCode::game_pieces[i].program);
        float2 uvs[4];
        uvs[0] = float2(0.0f,0.0f);
        uvs[1] = float2(1.0f,0.0f);
        uvs[2] = float2(1.0f,1.0f);
        uvs[3] = float2(0.0f,1.0f);
        float matrix[16] = {1,0,0,0, 0,1,0,0 ,0,0,1,0, 0,0,0,1};
        float4 temp_color = float4(1);
  
        PxTransform pxt = GamePieceCode::game_pieces[i].rbd.state->getGlobalPose();
        GamePieceCode::game_pieces[i].ot.p = float3(pxt.p.x,pxt.p.y,pxt.p.z);
        SpriteBatchCode::AddSpriteToBatchAtBuffer(OpenGLEmu::current_buffer_index, GamePieceCode::game_pieces[i].ot.p, GamePieceCode::game_pieces[i].ot.r, GamePieceCode::game_pieces[i].ot.s.xy(), float4(1) ,uvs,matrix, v_buffer);
        current_count++;        
    }
    
    SpriteUniforms* uniforms = SetUniformsVertex(SpriteUniforms);
    uniforms->pcm_mat = ortho_cam.projection_matrix;//float4x4::identity();//Camera::main.projection_matrix;
    OpenGLEmu::AddFragTextureBinding(GamePieceCode::game_pieces[0].texture,0);
    float from_bytes = v_buffer->from_to_bytes.y();
    float to_bytes = from_bytes + current_count * SIZE_OF_SPRITE_IN_BYTES;
    v_buffer->from_to_bytes = float2(from_bytes,to_bytes);
    OpenGLEmu::AddBufferBinding(v_buffer->buffer[bi],0,v_buffer->from_to_bytes.x());
    OpenGLEmu::DrawArrays(current_count * 6,SIZE_OF_SPRITE_IN_BYTES);
    
    current_count = 1;
    v_buffer = OpenGLEmu::GetBufferAtBinding(0);
    bi = OpenGLEmu::current_buffer_index;
    for(int i = 0;i < 32;++i)
    {
        //Graphics
        OpenGLEmu::UseProgram(GamePieceCode::game_pieces[i].program);
        float2 uvs[4];
        uvs[0] = float2(0.0f,0.0f);
        uvs[1] = float2(1.0f,0.0f);
        uvs[2] = float2(1.0f,1.0f);
        uvs[3] = float2(0.0f,1.0f);
        float matrix[16] = {1,0,0,0, 0,1,0,0 ,0,0,1,0, 0,0,0,1};
        float4 temp_color = float4(1);
        
        //GamePieceCode::game_pieces[i].ot.p = float3(pxt.p.x,pxt.p.y,pxt.p.z);
        SpriteBatchCode::AddSpriteToBatchAtBuffer(OpenGLEmu::current_buffer_index, GamePieceCode::game_pieces[i].ot.p, GamePieceCode::game_pieces[i].ot.r, GamePieceCode::game_pieces[i].life_ot[0].s.xy(), float4(1) ,uvs,matrix, v_buffer);
        
        current_count++;
    }
    
    SpriteUniforms* uuniforms = SetUniformsVertex(SpriteUniforms);
    uuniforms->pcm_mat = ortho_cam.projection_matrix;
    OpenGLEmu::AddFragTextureBinding(GamePieceCode::game_pieces[0].life_bar_texture,0);
    from_bytes = v_buffer->from_to_bytes.y();
    to_bytes = from_bytes + current_count * SIZE_OF_SPRITE_IN_BYTES;
    v_buffer->from_to_bytes = float2(from_bytes,to_bytes);
    OpenGLEmu::AddBufferBinding(v_buffer->buffer[bi],0,v_buffer->from_to_bytes.x());
    OpenGLEmu::DrawArrays(current_count * 6,SIZE_OF_SPRITE_IN_BYTES);
    
    PhysicsCode::Update(&scene,1,0.016f);        
//Play bgm
    //SoundCode::ContinousPlay(&bgm_soundclip);

    PlatformOutput(game_log,"Game update complete\n");
}

