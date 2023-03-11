#pragma once

struct platform_api;

struct game_event
{
    char Name[64];
    void *Params;
};

struct game_event_list
{
    u32 MaxEventCount;
    u32 volatile EventCount;
    game_event *Events;

    platform_api *Platform;
    memory_arena Arena;
};

struct animation_event_data
{
    u32 EntityId;
    f32 Weight;
};
