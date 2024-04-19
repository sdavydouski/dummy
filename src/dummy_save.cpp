#include "dummy.h"

inline void
Model2Spec(model *Model, model_spec *Spec)
{
    Spec->Has = true;
    CopyString(Model->Key, Spec->ModelRef);
}

inline void
Collider2Spec(collider *Collider, collider_spec *Spec)
{
    Spec->Has = true;
    Spec->Type = Collider->Type;

    switch (Collider->Type)
    {
        case Collider_Box:
        {
            Spec->Box = Collider->Box;

            break;
        }
        default:
        {
            NotImplemented;
        }
    }
}

inline void
RigidBody2Spec(rigid_body *Body, rigid_body_spec *Spec)
{
    Spec->Has = true;
}

inline void
PointLight2Spec(point_light *PointLight, point_light_spec *Spec)
{
    Spec->Has = true;
    Spec->Color = PointLight->Color;
    Spec->Attenuation = PointLight->Attenuation;
}

inline void
ParticleEmitter2Spec(particle_emitter *ParticleEmitter, particle_emitter_spec *Spec)
{
    Spec->Has = true;
    Spec->ParticleCount = ParticleEmitter->ParticleCount;
    Spec->ParticlesSpawn = ParticleEmitter->ParticlesSpawn;
    Spec->Color = ParticleEmitter->Color;
    Spec->Size = ParticleEmitter->Size;
}

inline void
AudioSource2Spec(audio_source *AudioSource, audio_source_spec *Spec)
{
    Spec->Has = true;
    CopyString(AudioSource->AudioClip->Key, Spec->AudioClipRef);
    Spec->Volume = AudioSource->Volume;
    Spec->MinDistance = AudioSource->MinDistance;
    Spec->MaxDistance = AudioSource->MaxDistance;
}
