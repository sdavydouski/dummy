#pragma once

#define EDITOR 1
#define PROFILER 1
#define ASSERT 1

#define SID(String) Hash(String)
#define MAX_ENTITY_NAME 256

#include "dummy_defs.h"
#include "dummy_math.h"
#include "dummy_random.h"
#include "dummy_memory.h"
#include "dummy_string.h"
#include "dummy_stream.h"
#include "dummy_container.h"
#include "dummy_input.h"
#include "dummy_events.h"
#include "dummy_bounds.h"
#include "dummy_body.h"
#include "dummy_collision.h"
#include "dummy_contact.h"
#include "dummy_visibility.h"
#include "dummy_spatial.h"
#include "dummy_camera.h"
#include "dummy_process.h"
#include "dummy_particles.h"
#include "dummy_animation.h"
#include "dummy_animator.h"
#include "dummy_assets.h"
#include "dummy_audio.h"
#include "dummy_renderer.h"
#include "dummy_text.h"
#include "dummy_save.h"
#include "dummy_job.h"
#include "dummy_profiler.h"
#include "dummy_platform.h"

struct game_assets;

//
u32 GenerateGameProcessId(game_state *State);
game_entity *GetGameEntity(game_state *State, u32 EntityId);
u32 FindNearbyEntities(spatial_hash_grid *Grid, game_entity *Entity, aabb Bounds, game_entity **Entities, u32 MaxEntityCount);
void PublishEvent(game_event_list *EventList, const char *EventName, void *Params);
void TransitionToNode(animation_graph *Graph, const char *NodeName);
animation_node *GetAnimationNode(animation_graph *Graph, const char *NodeName);
additive_animation *GetAdditiveAnimation(animation_node *Node, const char *AnimationClipName);
bool32 AnimationClipFinished(animation_state Animation);
bool32 AdditiveAnimationsFinished(animation_node *Node);
audio_clip *GetAudioClipAsset(game_assets *Assets, const char *Name);
aabb GetEntityBounds(game_entity *Entity);
aabb CreateAABBMinMax(vec3 Min, vec3 Max);
aabb CreateAABBCenterHalfExtent(vec3 Center, vec3 HalfExtent);
aabb CalculateAxisAlignedBoundingBox(obb Box);
void CalculateVertices(aabb Box, vec3 *Vertices);
aabb UpdateBounds(aabb Bounds, mat4 M);
aabb UpdateBounds(aabb Bounds, transform T);
bool32 TestAABBAABB(aabb a, aabb b);
bool32 IntersectRayAABB(ray Ray, aabb Box, vec3 &Coord);
audio_play_options SetVolume(f32 Volume);
void CalculateRigidBodyState(rigid_body *Body);
f32 TransformToAxis(collider_box *Box, vec3 Axis);
void CalculateVertices(collider_box *Box, vec3 *Vertices);
void CalculateNormalizedVertices(collider_box *Box, vec3 *Vertices);
bool32 TestBoxPlane(collider_box *Box, plane Plane);
//

enum game_mode
{
    GameMode_World,
    GameMode_Menu,
    GameMode_Editor
};

struct game_entity
{
    u32 Id;
    char Name[MAX_ENTITY_NAME];
    ivec3 GridCellCoords[2];
    transform Transform;
    vec3 DebugColor;

#if 1
    model *Model;
    skinning_data *Skinning;
    animation_graph *Animation;
    collider *Collider;
    rigid_body *Body;
    point_light *PointLight;
    particle_emitter *ParticleEmitter;
    audio_source *AudioSource;
#else
    u32 ModelIndex;
    u32 SkinningIndex;
    u32 AnimationIndex;
    u32 ColliderIndex;
    u32 BodyIndex;
    u32 PointLightIndex;
    u32 ParticleEmitterIndex;
    u32 AudioSourceIndex;
#endif

    // ?
    bool32 Visible;
    bool32 Destroyed;
    bool32 IsGrounded;
    bool32 IsManipulated;
};

struct world_area
{
    u32 MaxEntityCount;
    u32 EntityCount;
    game_entity *Entities;
    spatial_hash_grid SpatialGrid;
    memory_arena Arena;

#if 0
    model *Models[1024];
    skinning_data Skins[256];
    animation_graph AnimationGraphs[256];
    collider Colliders[1024];
    rigid_body Bodies[512];
    point_light PointLight[64];
    particle_emitter ParticleEmitters[64];
    audio_source AudioSources[64];
#endif
};

struct entity_render_batch
{
    char Key[256];
    u32 EntityCount;
    u32 MaxEntityCount;
    game_entity **Entities;

    model *Model;
    skinning_data *Skinning;

    union
    {
        mesh_instance *MeshInstances;
        skinned_mesh_instance *SkinnedMeshInstances;
    };
};

struct game_asset
{
    char Name[128];
    char Path[128];
};

struct game_asset_model
{
    game_asset GameAsset;
    model_asset *ModelAsset;
};

struct game_asset_font
{
    game_asset GameAsset;
    font_asset *FontAsset;
};

struct game_asset_texture
{
    game_asset GameAsset;
    texture_asset *TextureAsset;
};

struct game_asset_audio_clip
{
    game_asset GameAsset;
    audio_clip_asset *AudioAsset;
};

enum game_assets_state
{
    GameAssetsState_Unloaded,
    GameAssetsState_Loaded,
    GameAssetsState_Ready,
};

struct game_assets
{
    hash_table<model> Models;
    hash_table<font> Fonts;
    hash_table<texture> Textures;
    hash_table<audio_clip> AudioClips;

    u32 ModelAssetCount;
    game_asset_model *ModelAssets;

    u32 FontAssetCount;
    game_asset_font *FontAssets;

    u32 TextureAssetCount;
    game_asset_texture *TextureAssets;

    u32 AudioClipAssetCount;
    game_asset_audio_clip *AudioClipAssets;

    game_assets_state State;

    memory_arena Arena;
};

struct game_options
{
    bool32 EnableShadows;
    bool32 ShowCascades;
    bool32 ShowBoundingVolumes;
    bool32 ShowSkeletons;
    bool32 ShowCamera;
    bool32 ShowGrid;
    bool32 ShowSkybox;
    bool32 ShowSpatialGrid;
    bool32 WireframeMode;
};

struct game_menu_quad
{
    vec4 Color;
    u32 Corner;
    f32 Move;
};

struct game_state
{
    memory_arena PermanentArena;
    memory_arena FrameArena;

    job_queue *JobQueue;
    stream Stream;

    world_area WorldArea;

    game_mode Mode;

    game_camera PlayerCamera;
    game_camera EditorCamera;

    vec2 ViewFrustrumSize;
    polyhedron Frustrum;
    plane Ground;

    game_assets Assets;
    animator Animator;

    u32 NextFreeEntityId;
    u32 NextFreeMeshId;
    u32 NextFreeTextureId;
    u32 NextFreeSkinningId;
    u32 NextFreeProcessId;
    u32 NextFreeAudioSourceId;

    u32 RenderableEntityCount;
    u32 ActiveEntitiesCount;

    game_entity *Player;
    game_entity *SelectedEntity;

    hash_table<entity_render_batch> EntityBatches;

    directional_light DirectionalLight;

    hash_table<game_process> Processes;
    // linked-list (for efficient adding/removal and traversing)
    game_process ProcessSentinel;

    random_sequence GeneralEntropy;
    random_sequence ParticleEntropy;

    game_event_list EventList;

    f32 MasterVolume;

    vec3 BackgroundColor;

    vec2 TargetMove;
    vec2 CurrentMove;

    game_options Options;
    game_menu_quad MenuQuads[4];

    value_state<bool32> DanceMode;

    // todo:
    contact_resolver ContactResolver;

    // todo: temp
    u32 SkyboxId;
};
