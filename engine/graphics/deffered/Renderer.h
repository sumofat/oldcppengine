#if !defined(RENDERER_H)

struct IsVisibleTable
{
    memory_index model_index;
};

namespace Renderer
{
    extern vector is_model_visible_table;
};

#define RENDERER_H
#endif 

#ifdef ENGINEIMPL

namespace Renderer
{
    vector is_model_visible_table;
    bool is_int;
    void Init()
    {
        is_model_visible_table = YoyoInitVector(1,IsVisibleTable,false);
    }

    memory_index AddVisibleModel(ModelAsset* m)
    {
        YoyoStretchPushBack(&is_model_visible_table,m);
        return 
    }

    void DeleteVisibleModel(memory_index handle)
    {
        YoyoFreeIndex(&is_model_visible_table,handle);
    }
    
};



#endif


