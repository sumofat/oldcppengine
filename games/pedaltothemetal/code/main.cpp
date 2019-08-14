
#include "engineapi.h"
#include "GamePiece.cpp"
bool game_log = false;

//TestAssets
static SoundClip bgm_soundclip;

//Threading
TicketMutex ticket_mutex;

//Game Specific
SpriteBatchBuffer sprite_batch_buffer;

//GLEMU TEST STUFF
GLTexture tex;
GLProgram gl_program;
uint64_t gl_program_handle;
RenderCamera ortho_cam;

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
        
    OpenGLEmu::CreateBufferAtBinding(0);
    Yostr tpath = AssetSystem::GetDataPath("ship.png",StringsHandler::transient_string_memory);
    Resource::GetImageFromDisk(tpath.String, &lt,4);
    tex = OpenGLEmu::TexImage2D(lt.texels, lt.dim, PixelFormatRGBA8Unorm, OpenGLEmu::GetDefaultDescriptor(), TextureUsageShaderRead);
    tex.sampler = OpenGLEmu::GetDefaultSampler();
    
    VertexDescriptor vd = SpriteBatchCode::CreateDefaultSpriteBatchDescriptor();
    GLProgram program = OpenGLEmu::AddProgramFromMainLibrary("spritebatch_vs","spritebatch_fs_single_sampler",vd);
    gl_program = program;
    gl_program_handle = program.id;

    //ENDGLEMU TEST
    
    PlatformOutput(game_log,"Game Init Complete\n");
}


float3 ship_p;
quaternion ship_r;
float angle = 0;
//Frame Update
extern "C" void gameUpdate()
{
    PlatformOutput(game_log,"Game Update start\n");
//Update things
    angle += 2;
    float2 center_p = RendererCode::dim * 0.5f;
    ship_p = float3(center_p,0);
    ship_r = axis_angle(float3(0,0,1),angle);    

//Graphics    
    //TESTGLEMU
//    OpenGLEmu::ClearColor(float4(0.25f,0.02f,0.6f,1.0f));
//    OpenGLEmu::ClearBuffer(CLEAR_COLOR_BIT | CLEAR_STENCIL_BIT);
    OpenGLEmu::UseProgram(gl_program);
		
    uint32_t bi = OpenGLEmu::current_buffer_index;
        
    TripleGPUBuffer* v_buffer = OpenGLEmu::GetBufferAtBinding(0);
        
    float2 uvs[4];
    uvs[0] = float2(0.0f,0.0f);
    uvs[1] = float2(1.0f,0.0f);
    uvs[2] = float2(1.0f,1.0f);
    uvs[3] = float2(0.0f,1.0f);
    float matrix[16] = {1,0,0,0, 0,1,0,0 ,0,0,1,0, 0,0,0,1};
    float4 temp_color = float4(1);
    uint32_t current_count = 1; 
    SpriteBatchCode::AddSpriteToBatchAtBuffer(OpenGLEmu::current_buffer_index, ship_p, ship_r, float2(100), float4(1) ,uvs,matrix, v_buffer);

    SpriteUniforms* uniforms = SetUniformsVertex(SpriteUniforms);

    uniforms->pcm_mat = ortho_cam.projection_matrix;//float4x4::identity();//Camera::main.projection_matrix;

    OpenGLEmu::AddFragTextureBinding(tex,0);
    float from_bytes = v_buffer->from_to_bytes.y();
    float to_bytes = from_bytes + current_count * SIZE_OF_SPRITE_IN_BYTES;
    v_buffer->from_to_bytes = float2(from_bytes,to_bytes);
    OpenGLEmu::AddBufferBinding(v_buffer->buffer[bi],0,v_buffer->from_to_bytes.x());
    OpenGLEmu::DrawArrays(current_count * 6,SIZE_OF_SPRITE_IN_BYTES);

//Play bgm
    SoundCode::ContinousPlay(&bgm_soundclip);
    PlatformOutput(game_log,"Game update complete\n");
}

