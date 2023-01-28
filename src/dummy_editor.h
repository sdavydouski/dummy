#pragma once

struct editor_state
{
    u32 CurrentGizmoOperation;

    memory_arena Arena;

    // todo:
    vec3 Size;
    bool32 RootMotion;
    vec3 Color;
    light_attenuation Attenuation;
    vec4 ParticleColor;
    vec2 ParticleSize;
    u32 ParticleCount;
    u32 ParticlesSpawn;

#if 0
    char Search[256];
    i32 AssetType;
    i32 SelectedAssetIndex;
    model *SelectedModel;
#endif
};
