#pragma once

struct editor_state
{
    u32 CurrentGizmoOperation;

    memory_arena Arena;

    char Search[256];
    i32 AssetType;
    i32 SelectedAssetIndex;
    model *SelectedModel;
};
