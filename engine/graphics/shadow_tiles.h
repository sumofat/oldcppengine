#if !defined(SHADOW_TILES)

struct tile_entry
{
    float2 dim;
    float2 tile_count;
    float2 offset;
    float2 absolute_offset;
    bool occupied;
};

struct block_entry
{
    float2 offset;
    u32 tile_count;
    tile_entry* first_open;
    tile_entry* tiles;
};

struct block_texture
{
    f32 tile_size = 128;
    u32 texture_size = 2048;
    u32 max_block_size = 1024;
    u32 max_block_count;
    u32 entry_count;
    u32 max_blocks_per_texture_one_dim;
    u32 max_tile_count_per_block;
    u32 max_tiles_per_block_one_dim;
    block_entry* first_open;
    block_entry* blocks;
};

struct block_tile_entry_result
{
    float4 tile;
    float2 tile_count_used;
    float2 block;
    float2 abs;
};

//void InitSquareBlockTexture(block_texture* block,u32 size,u32 max_block_size,u32 min_tile_size);
//block_tile_entry_result AddTileEntry_(block_texture* block_texture);
//block_tile_entry_result AddTileEntry(block_texture* block_texture,float2 tile_count_to_use);
//void ResetBlocks(block_texture* block_texture);

#define SHADOW_TILES
#endif
