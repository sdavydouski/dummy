#pragma once

#define GAME_PROCESS_ON_UPDATE(Name) void Name(struct game_state *State, struct game_process *Process, f32 Delta)
typedef GAME_PROCESS_ON_UPDATE(game_process_on_update);
