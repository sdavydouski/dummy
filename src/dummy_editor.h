#pragma once

struct editor_state
{
    u32 CurrentGizmoOperation;

    memory_arena Arena;

    vec3 Size;
    b32 RootMotion;

#if 0
    char Search[256];
    i32 AssetType;
    i32 SelectedAssetIndex;
    model *SelectedModel;
#endif
};
