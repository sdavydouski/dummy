#include "dummy.h"

inline game_entity * GetGameEntity(game_state *State, u32 EntityId);

inline void
PublishEvent(game_event_list *EventList, const char *EventName, void *Params)
{
    Assert(EventList->EventCount < EventList->MaxEventCount);

    platform_api *Platform = EventList->Platform;

    Platform->EnterCriticalSection(Platform->PlatformHandle);

    game_event *Event = EventList->Events + EventList->EventCount++;

    Platform->LeaveCriticalSection(Platform->PlatformHandle);

    CopyString(EventName, Event->Name);
    Event->Params = Params;
}

dummy_internal void
ProcessEvents(game_state *State, audio_commands *AudioCommands, render_commands *RenderCommands)
{
    game_event_list *EventList = &State->EventList;
    game_assets *Assets = &State->Assets;

    for (u32 EventIndex = 0; EventIndex < EventList->EventCount; ++EventIndex)
    {
        game_event *Event = EventList->Events + EventIndex;

        if (StringEquals(Event->Name, "footstep"))
        {
            animation_event_data *Data = (animation_event_data *) Event->Params;
            f32 Weight = Data->Weight;

            if (Weight >= 0.5f)
            {
                game_entity *Entity = GetGameEntity(State, Data->EntityId);
                vec3 Position = Entity->Transform.Translation;

                audio_clip *AudioClip = 0;
                char *ModelName = Entity->Model->Key;

                if (StringEquals(ModelName, "paladin"))
                {
                    AudioClip = GetAudioClipAsset(Assets, "step_metal");
                }
                else if (StringEquals(ModelName, "pelegrini"))
                {
                    AudioClip = GetAudioClipAsset(Assets, "step_lth1");
                }
                else if (StringEquals(ModelName, "xbot") || StringEquals(ModelName, "ybot"))
                {
                    AudioClip = GetAudioClipAsset(Assets, "step_cloth1");
                }
                else if (StringEquals(ModelName, "warrok") || StringEquals(ModelName, "maw"))
                {
                    AudioClip = GetAudioClipAsset(Assets, "step_lth4");
                }

                Assert(AudioClip);

                Play3D(AudioCommands, AudioClip, Position, SetVolume(Weight));
            }
        }
    }

    EventList->EventCount = 0;
    ClearMemoryArena(&EventList->Arena);
}
