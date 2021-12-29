internal game_process *
GetGameProcess(game_state *State, char *Name)
{
    game_process *Result = HashTableLookup(State->MaxProcessCount, State->Processes, Name);
    return Result;
}

inline void
AddGameProcess(game_state *State, game_process *Process)
{
    AddToLinkedList(&State->ProcessSentinel, Process);
}

inline void
RemoveGameProcess(game_process *Process)
{
    RemoveFromLinkedList(Process);
}

inline void
InitGameProcess(game_process *Process, char *ProcessName, game_process_on_update *OnUpdatePerFrame)
{
    CopyString(ProcessName, Process->Name);
    Process->OnUpdatePerFrame = OnUpdatePerFrame;
}

internal void
StartGameProcess(game_state *State, const char *ProcessName, game_process_on_update *OnUpdatePerFrame)
{
    game_process *Process = GetGameProcess(State, (char *)ProcessName);

    // todo: do a better job, for christ's sake
    // if empty
    if (StringEquals(Process->Name, ""))
    {
        InitGameProcess(Process, (char *)ProcessName, OnUpdatePerFrame);
        AddGameProcess(State, Process);
    }
}

internal void
EndGameProcess(game_state *State, const char *ProcessName)
{
    game_process *Process = GetGameProcess(State, (char *)ProcessName);

    // if exists
    if (StringEquals(Process->Name, ProcessName))
    {
        RemoveGameProcess(Process);

        if (Process->Child)
        {
            AddGameProcess(State, Process->Child);
        }

        // todo: removing element from hashtable
        CopyString("", Process->Name);
    }
}

inline void
AttachChildGameProcess(game_state *State, char *ParentProcessName, char *ChildProcessName, game_process_on_update *ChildOnUpdatePerFrame)
{
    game_process *ParentProcess = GetGameProcess(State, ParentProcessName);
    game_process *ChildProcess = GetGameProcess(State, ChildProcessName);

    // todo: IsEmpty?
    if (StringEquals(ChildProcess->Name, ""))
    {
        InitGameProcess(ChildProcess, ChildProcessName, ChildOnUpdatePerFrame);
    }

    ParentProcess->Child = ChildProcess;
}

inline GAME_PROCESS_ON_UPDATE(DelayProcess)
{
    State->DelayTime += Delta;

    if (State->DelayTime >= State->DelayDuration)
    {
        State->DelayTime = 0.f;

        EndGameProcess(State, Process->Name);
    }
}

inline GAME_PROCESS_ON_UPDATE(ChangeBackgroundProcess)
{
    f32 Red = Random01(&State->RNG);
    f32 Green = Random01(&State->RNG);
    f32 Blue = Random01(&State->RNG);

    State->DirectionalColor = vec3(Red, Green, Blue);

    EndGameProcess(State, Process->Name);
}

inline GAME_PROCESS_ON_UPDATE(PlayerOrientationLerpProcess)
{
    rigid_body *PlayerBody = State->Player->Body;

    if (PlayerBody->OrientationLerp.Duration > 0.f)
    {
        PlayerBody->OrientationLerp.Time += Delta;

        f32 t = PlayerBody->OrientationLerp.Time / PlayerBody->OrientationLerp.Duration;

        if (t <= 1.f)
        {
            PlayerBody->Orientation = Slerp(PlayerBody->OrientationLerp.From, t, PlayerBody->OrientationLerp.To);
        }
        else
        {
            EndGameProcess(State, Process->Name);
        }
    }
}

inline GAME_PROCESS_ON_UPDATE(CameraLerpProcess)
{
    if (State->PlayerCamera.PositionLerp.Duration > 0.f)
    {
        State->PlayerCamera.PositionLerp.Time += Delta;

        f32 t = State->PlayerCamera.PositionLerp.Time / State->PlayerCamera.PositionLerp.Duration;

        if (t <= 1.f)
        {
            State->PlayerCamera.Pivot = Lerp(State->PlayerCamera.PositionLerp.From, t, State->PlayerCamera.PositionLerp.To);
        }
        else
        {
            EndGameProcess(State, Process->Name);
        }
    }
}