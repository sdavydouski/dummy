#include "dummy.h"

inline game_process *
GetGameProcess(game_state *State, char *Name)
{
    game_process *Result = HashTableLookup(&State->Processes, Name);
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
    CopyString(ProcessName, Process->Key);
    Process->OnUpdatePerFrame = OnUpdatePerFrame;
}

dummy_internal void
StartGameProcess_(game_state *State, const char *ProcessName, game_process_on_update *OnUpdatePerFrame)
{
    game_process *Process = GetGameProcess(State, (char *)ProcessName);

    if (IsSlotEmpty(Process->Key))
    {
        InitGameProcess(Process, (char *)ProcessName, OnUpdatePerFrame);
        AddGameProcess(State, Process);
    }
}

dummy_internal void
EndGameProcess(game_state *State, const char *ProcessName)
{
    game_process *Process = GetGameProcess(State, (char *)ProcessName);

    // if exists
    if (StringEquals(Process->Key, ProcessName))
    {
        RemoveGameProcess(Process);

        if (Process->Child)
        {
            AddGameProcess(State, Process->Child);
        }

        RemoveFromHashTable(Process->Key);
    }
}

#define StartGameProcess(State, OnUpdatePerFrame) StartGameProcess_(State, #OnUpdatePerFrame, OnUpdatePerFrame)

inline void
AttachChildGameProcess(game_state *State, char *ParentProcessName, char *ChildProcessName, game_process_on_update *ChildOnUpdatePerFrame)
{
    game_process *ParentProcess = GetGameProcess(State, ParentProcessName);
    game_process *ChildProcess = GetGameProcess(State, ChildProcessName);

    if (IsSlotEmpty(ChildProcess->Key))
    {
        InitGameProcess(ChildProcess, ChildProcessName, ChildOnUpdatePerFrame);
    }

    ParentProcess->Child = ChildProcess;
}

#if 0
DLLExport GAME_PROCESS_ON_UPDATE(DelayProcess)
{
    State->DelayTime += Delta;

    if (State->DelayTime >= State->DelayDuration)
    {
        State->DelayTime = 0.f;

        EndGameProcess(State, Process->Key);
    }
}
#endif

DLLExport GAME_PROCESS_ON_UPDATE(PlayerOrientationLerpProcess)
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
            EndGameProcess(State, Process->Key);
        }
    }
}

#if 0
DLLExport GAME_PROCESS_ON_UPDATE(PlayerPositionLerpProcess)
{
    rigid_body *PlayerBody = State->Player->Body;

    if (PlayerBody->PositionLerp.Duration > 0.f)
    {
        PlayerBody->PositionLerp.Time += Delta;

        f32 t = PlayerBody->PositionLerp.Time / PlayerBody->PositionLerp.Duration;

        if (t <= 1.f)
        {
            PlayerBody->Position = Lerp(PlayerBody->PositionLerp.From, t, PlayerBody->PositionLerp.To);
        }
        else
        {
            EndGameProcess(State, Process->Key);
        }
    }
}
#endif

DLLExport GAME_PROCESS_ON_UPDATE(SmoothInputMoveProcess)
{
    f32 InterpolationTime = 0.2f;

    vec2 dMove = (State->TargetMove - State->CurrentMove) / InterpolationTime;
    State->CurrentMove += dMove * Delta;
}
