#pragma once

#define MAX_IMNODE_CONTEXT_COUNT 8

struct editor_add_entity
{
    char Name[MAX_ENTITY_NAME];
    model_spec Model;
    collider_spec Collider;
    rigid_body_spec RigidBody;
    point_light_spec PointLight;
    particle_emitter_spec ParticleEmitter;
};

struct editor_state
{
    bool32 GameWindowHovered;

    u32 CurrentGizmoOperation;

    u32 CurrentStreamIndex;

    bool32 LogAutoScroll;
    ImGuiTextFilter LogFilter;

    platform_api *Platform;
    memory_arena Arena;

    editor_add_entity AddEntity;

    stack<animation_graph *> AnimationGraphStack;
    ImNodesEditorContext *ImNodesContexts[MAX_IMNODE_CONTEXT_COUNT];
};
