
#include "engineapi.h"
#include "GamePiece.cpp"
bool game_log = false;

//TestRender settings
RenderCamera ortho_cam;


//TestAssets
static SoundClip bgm_soundclip;

//Threading
TicketMutex ticket_mutex;

//Game Specific
SpriteBatchBuffer sprite_batch_buffer;

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
#define TEST_FILE_COUNT 3
    s32 test_file_count = 3;
    Yostr test_files[TEST_FILE_COUNT];
    test_files[0] = CreateStringFromLiteral("spaceship.1.png",&StringsHandler::transient_string_memory);
    test_files[1] = CreateStringFromLiteral("spaceship.1.png",&StringsHandler::transient_string_memory);
    test_files[2] = CreateStringFromLiteral("spaceship.1.png",&StringsHandler::transient_string_memory);
    
    YoyoSpriteBatchRenderer::CreateAtlasFromTextureArray(&sprite_batch_buffer, test_files, 3, &StringsHandler::transient_string_memory);
    YoyoSpriteBatchRenderer::Init(&sprite_batch_buffer,dim,LoadActionClear);

    PlatformOutput(game_log,"Game Init Complete\n");
}




//Frame Update
extern "C" void gameUpdate()
{
    PlatformOutput(game_log,"Game Update start\n");
//Update things
//Play bgm
    SoundCode::ContinousPlay(&bgm_soundclip);
    PlatformOutput(game_log,"Game update complete\n");
}

