#pragma once

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
    u32 CurrentGizmoOperation;

    memory_arena Arena;

    editor_add_entity AddEntity;
};
