#pragma once

struct particle
{
    vec3 Position;
    vec3 Velocity;
    vec3 Acceleration;
    vec4 Color;
    vec4 dColor;
    vec2 Size;
    vec2 dSize;
    f32 CameraDistanceSquared;
};

struct particle_emitter
{
    u32 NextParticleIndex;

    u32 ParticleCount;
    particle *Particles;

    u32 ParticlesSpawn;
    vec4 Color;
    vec2 Size;
};
