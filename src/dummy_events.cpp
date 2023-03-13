#include "dummy.h"

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

                switch (SID(Entity->Model->Key))
                {
                    case SID("xbot"):
                    case SID("ybot"):
                    case SID("pelegrini"):
                    {
                        AudioClip = GetAudioClipAsset(Assets, "step_cloth1");
                        break;
                    }
                    case SID("paladin"):
                    {
                        AudioClip = GetAudioClipAsset(Assets, "step_metal");
                        break;
                    }
                    case SID("warrok"):
                    case SID("maw"):
                    {
                        AudioClip = GetAudioClipAsset(Assets, "step_lth4");
                        break;
                    }
                }

                Assert(AudioClip);

                Play3D(AudioCommands, AudioClip, Position, SetVolume(Weight));
            }
        }
    }

    EventList->EventCount = 0;
    ClearMemoryArena(&EventList->Arena);
}
