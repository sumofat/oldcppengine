//
//  Header.h
//  pedaltothemetal
//
//  Created by Ray Olen Garner on 2019/01/11.
//  Copyright © 2019 DeNA. All rights reserved.
//

//NOTE(Ray):This is a test game for the engine that is relatively complex.
//We would like to try kitlang as a scripting language for its interopablilty with C

#ifndef Main_H
#define Main_H

#include "engine.h"

#define MAX_KARTS 8

//NOTE(Ray):This is for quick starting only we wan this list dynamically
//created from data
enum ItemType
{
    Item_FireShooter,
    Item_Star,
    Item_Turbo
};

enum Property
{
    Property_Speed
};

struct Item
{
    ItemType type;
    Iem
};

struct Kart
{
    Yostr name;
    float prev_speed;    
    float current_speed;
    float v;
    float prev_v;
    Item items[2];//Max 2 items at at time
    Item current_using_item;
    SceneObject* so;
};

struct GameState
{
    Kart karts[MAX_KARTS];
    
};


#endif /* Header_h */
