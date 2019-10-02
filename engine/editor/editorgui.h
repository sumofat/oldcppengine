#if !defined(EDITORGUI_H)
typedef void (*RunGameImgui)();

//NOTE(Ray):In order the editor to ahem edit we need references to rest of the engine
//and systems which creates entanglement... 
namespace EditorGUI
{
    extern RunGameImgui game_callback;
    void Init();    
    void Update();
}

#define EDITORGUI_H
#endif

