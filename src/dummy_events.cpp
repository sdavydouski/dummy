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

internal void
ProcessEvents(game_state *State, audio_commands *AudioCommands, render_commands *RenderCommands)
{
    game_event_list *EventList = &State->EventList;
    game_assets *Assets = &State->Assets;

    for (u32 EventIndex = 0; EventIndex < EventList->EventCount; ++EventIndex)
    {
        game_event *Event = EventList->Events + EventIndex;

        if (StringEquals(Event->Name, "footstep"))
        {
            animation_footstep_event *Data = (animation_footstep_event *) Event->Params;

            {
                game_entity *Entity = GetGameEntity(State, Data->EntityId);
                vec3 Position = Entity->Transform.Translation;

                audio_clip *AudioClip = 0;
                char *ModelName = Entity->Model->Key;

                if (StringEquals(ModelName, "Paladin"))
                {
                    AudioClip = GetAudioClipAsset(Assets, "step_metal");
                }
                else if (StringEquals(ModelName, "Pelegrini") || StringEquals(ModelName, "xBot") || StringEquals(ModelName, "yBot"))
                {
                    AudioClip = GetAudioClipAsset(Assets, "step_cloth1");
                }
                else if (StringEquals(ModelName, "Warrok") || StringEquals(ModelName, "Maw"))
                {
                    AudioClip = GetAudioClipAsset(Assets, "step2");
                }

                Assert(AudioClip);

                Play3D(AudioCommands, AudioClip, Position);
            }
        }
    }

    EventList->EventCount = 0;
    ClearMemoryArena(&EventList->Arena);
}
