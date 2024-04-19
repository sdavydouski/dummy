#pragma once

// todo: update spec structs
struct model_spec
{
    bool32 Has;
    char ModelRef[64];
};

struct collider_spec
{
    bool32 Has;

    collider_type Type;
    union
    {
        collider_box Box;
        collider_sphere Sphere;
    };

    vec3 Translation;
    quat Rotation;
};

struct rigid_body_spec
{
    bool32 Has;
};

struct point_light_spec
{
    bool32 Has;
    vec3 Color;
    light_attenuation Attenuation;
};

struct particle_emitter_spec
{
    bool32 Has;
    u32 ParticleCount;
    u32 ParticlesSpawn;
    vec4 Color;
    vec2 Size;
};

struct audio_source_spec
{
    bool32 Has;
    char AudioClipRef[64];
    f32 Volume;
    f32 MinDistance;
    f32 MaxDistance;
};

struct game_entity_spec
{
    char Name[MAX_ENTITY_NAME];
    transform Transform;
    model_spec ModelSpec;
    collider_spec ColliderSpec;
    rigid_body_spec RigidBodySpec;
    point_light_spec PointLightSpec;
    particle_emitter_spec ParticleEmitterSpec;
    audio_source_spec AudioSourceSpec;

    vec3 DebugColor;
};

#pragma pack(push, 1)
enum game_file_type
{
    GameFile_Area,
    GameFile_Entity
};

struct game_file_header
{
    u32 MagicValue;
    u32 Version;
    u32 EntityCount;
    game_file_type Type;
};
#pragma pack(pop)
