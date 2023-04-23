#pragma once

#define GAME_PROCESS_ON_UPDATE(Name) void Name(struct game_state *State, struct game_process *Process, f32 Delta)
typedef GAME_PROCESS_ON_UPDATE(game_process_on_update);

struct game_process_params
{
    f32 InterpolationTime;
    game_entity *Entity;
};

struct game_process
{
    game_process_on_update *OnUpdatePerFrame;
    game_process *Child;

    game_process_params Params;

    char Key[256];
    game_process *Prev;
    game_process *Next;
};
