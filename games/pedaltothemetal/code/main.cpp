
#include "engine.h"

bool game_log = true;
    
extern "C" void gameInit()
{
    PlatformOutput(game_log,"Game Init Start\n");
//Init things    
    PlatformOutput(game_log,"Game Init Complete\n");
}

extern "C" void gameUpdate()
{
    PlatformOutput(game_log,"Game Update start\n");
//Update things
    PlatformOutput(game_log,"Game update complete\n");
}

