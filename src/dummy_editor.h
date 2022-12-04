#pragma once

struct editor_state
{
    u32 CurrentGizmoOperation;

    memory_arena Arena;

    vec3 Size;
    b32 RootMotion;
    vec3 Color;
    light_attenuation Attenuation;
    vec4 ParticleColor;
    u32 ParticleCount;

#if 0
    char Search[256];
    i32 AssetType;
    i32 SelectedAssetIndex;
    model *SelectedModel;
#endif
};
