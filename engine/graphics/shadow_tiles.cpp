
#ifdef ENGINEIMPL
#include "shadow_tiles.h"

namespace ShadowTilesCode
{
    MemoryArena lights_memory;
    MemoryArena lights_properties_memory;
    u32 light_index = 0;

    void InitSquareBlockTexture(block_texture* block,u32 size,u32 max_block_size,u32 min_tile_size)
    {
        if(block)
        {
            block->texture_size = size;
            block->max_block_size = max_block_size;
            block->max_block_count = size / max_block_size * 2;//total
            block->max_blocks_per_texture_one_dim = size / max_block_size;//one dimension
            block->max_tile_count_per_block = power(max_block_size / min_tile_size,2);
            block->max_tiles_per_block_one_dim = max_block_size / min_tile_size;
            //TODO(Ray):make this take an arena.        
            block->blocks = (block_entry*)PlatformAllocateMemory(sizeof(block_entry) * block->max_block_count);
            u32 x = 0;
            u32 y = 0;
            float2 abs_offset = float2(0,0);
            for(u32 i = 0;i < block->max_block_count;++i)
            {
                block_entry* entry =  block->blocks + i;
                entry->tiles = (tile_entry*)PlatformAllocateMemory(sizeof(tile_entry) * block->max_tile_count_per_block);
                entry->offset = float2(x++,y);

                if(i == 0)block->first_open = entry;
                u32 t_x = 0;
                u32 t_y = 0;
                for(u32 j = 0;j < block->max_tile_count_per_block;++j)
                {
                    tile_entry* tile = entry->tiles + j;
                    if(j == 0)
                    {
                        entry->first_open = tile;                    
                    }

                    tile->dim = float2(1,1) * min_tile_size;
                    tile->tile_count = float2(1,1);
                    tile->offset = float2(t_x++,t_y);
                    float2 a = 
                        tile->absolute_offset = entry->offset * block->max_tiles_per_block_one_dim + tile->offset;
                    //end of tiles row in block
                    if(t_x > block->max_tiles_per_block_one_dim - 1)
                    {
                        abs_offset.setY( t_y);
                        t_y++;

                        t_x = 0;
                    }   
                }
            
                //end of block row in texture
                if(x > block->max_blocks_per_texture_one_dim - 1)
                {
                    y++;
                    x = 0;
                }
            }
        }
    }
    block_tile_entry_result AddTileEntry_(block_texture* block_texture);
//NOTE(Ray):Here using recursion thought you were clever but now your cluster fucked haha.
//Rewrite this to make it easier to understand and more efficient!  What was I thinking.
    block_tile_entry_result AddTileEntry(block_texture* block_texture,float2 tile_count_to_use)
    {
        block_tile_entry_result result_offsets;

        //TODO(Ray):I promise I will remove this :P
        block_entry* block = block_texture->first_open;
        //tile_count_to_use = tile_count_to_use - 1;
        //are we over block size//will we go over block size on this tile//do we have enough contiguous blocks?
        if(block_texture->entry_count < block_texture->max_block_count - 1 &&
           block)
        {
            if(block->tile_count < block_texture->max_tile_count_per_block - 1 &&
               block->tile_count + mul(tile_count_to_use) < block_texture->max_tile_count_per_block - 1 )
            {
                tile_entry* new_tile = block->first_open;
                u32 new_block_count = mul(tile_count_to_use);
                //find next possible tile if greater than min tile size
                if(new_block_count > 1)
                {
                    s32 remaining_tile_count = block_texture->max_tile_count_per_block - block->tile_count;
                    for(int i = 0;i < remaining_tile_count;++i)
                    {
                        //TODO(Ray):Should I be executed for this abomination?
                        if(block->first_open->offset.x() + tile_count_to_use.x() > block_texture->max_tiles_per_block_one_dim - 1 ||
                           block->first_open->offset.y() + tile_count_to_use.y() > block_texture->max_tiles_per_block_one_dim - 1)
                        {
                            //Cant fit here try next tile
                            block->first_open++;
                            new_tile = block->first_open;
                        }
                        else
                        {
                            //can fit set as offset
                            tile_entry* at_tile = new_tile;
                            s32 block_count_to_mark = new_block_count;
                            tile_entry* next_open_tile = nullptr;
                            //mark the tiles we are using as occupied
                            while(block_count_to_mark != 0)
                            {
                                if(at_tile->absolute_offset.x() < new_tile->offset.x() + tile_count_to_use.x() && 
                                   at_tile->absolute_offset.y() < new_tile->offset.y() + tile_count_to_use.y())
                                {
                                    at_tile->occupied = true;
                                    block_count_to_mark--;
                                }
                                else if(!next_open_tile && !at_tile->occupied)
                                {
                                    next_open_tile = at_tile;
                                }
                                at_tile++;
                                //TODO(Ray):Assert here we didnt go out of bounds of the block we should have already determined that we have room here
                            }
                        
                            block->first_open = next_open_tile;
                            block->tile_count += new_block_count;
                        
                            //We need to do a check first to make sure we ae not going out of the block bounds.
                            result_offsets.tile = float4(new_tile->dim.x() * tile_count_to_use.x() , new_tile->dim.y() * tile_count_to_use.y(), new_tile->offset.x(), new_tile->offset.y() );
                            result_offsets.tile_count_used = tile_count_to_use;
                            result_offsets.block = block->offset;
                            result_offsets.abs = new_tile->absolute_offset;
                            break;
                        }
                    }                    
                }
                else if(new_tile->occupied)
                {
                    //If we are occupied look for next open tile if we dont find any try next block and if still no good exit out and log it.
                    //new_tile->occupied = true;
                
                    block->first_open++;// = new_tile + 1;
                    //block->tile_count++;
                    AddTileEntry_(block_texture);
                    /*
                      result_offsets.tile = V4(new_tile->dim.x , new_tile->dim.y, new_tile->offset.x , new_tile->offset.y );
                      result_offsets.tile_count_used = V2(1,1);
                      result_offsets.block = block->offset;
                      result_offsets.abs = new_tile->absolute_offset;
                    */
                }
                else
                {
                
                    new_tile->occupied = true;
                
                    block->first_open++;// = new_tile + 1;
                    block->tile_count++;
                
                    result_offsets.tile = float4(new_tile->dim, new_tile->offset );
                    result_offsets.tile_count_used = float2(1,1);
                    result_offsets.block = block->offset;
                    result_offsets.abs = new_tile->absolute_offset;
                }
            

            }
            else
            {
                block_texture->first_open = block_texture->first_open + 1;
                result_offsets = AddTileEntry(block_texture,tile_count_to_use);
                //TODO(RaY):No more tiles for this block
                //look for another open block.
//            Assert(false);
            }        
        }
        else
        {
            //TODO(Ray):We have no more blocks left 
            Assert(false);
        }
        return result_offsets;
    }

    block_tile_entry_result AddTileEntry_(block_texture* block_texture)
    {
        return AddTileEntry(block_texture,float2(1,1));
    }
    
    void ResetBlocks(block_texture* block_texture)
    {
        block_texture->first_open = block_texture->blocks;
        for(u32 i = 0;i < block_texture->max_block_count;++i)
        {
            block_entry* block = block_texture->blocks + i;
            block->tile_count = 0;
            block->first_open = block->tiles;
            for(int ti = 0;ti < (int)block_texture->max_tile_count_per_block;++ti)
            {
                tile_entry* tile = block->first_open + ti;
                tile->occupied = false;
            }
        }
    }
};

#endif
