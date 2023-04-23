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
InitGameProcess(game_process *Process, char *ProcessName, game_process_on_update *OnUpdatePerFrame, game_process_params Params)
{
    CopyString(ProcessName, Process->Key);
    Process->OnUpdatePerFrame = OnUpdatePerFrame;
    Process->Params = Params;
}

dummy_internal void
StartGameProcess_(game_state *State, const char *ProcessName, game_process_on_update *OnUpdatePerFrame, game_process_params Params)
{
    game_process *Process = GetGameProcess(State, (char *)ProcessName);

    if (IsSlotEmpty(Process->Key))
    {
        InitGameProcess(Process, (char *)ProcessName, OnUpdatePerFrame, Params);
        AddGameProcess(State, Process);
    }
}

dummy_internal void
EndGameProcess_(game_state *State, const char *ProcessName)
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

#define StartGameProcess(State, OnUpdatePerFrame, Params) StartGameProcess_(State, #OnUpdatePerFrame, OnUpdatePerFrame, Params)
#define EndGameProcess(State, OnUpdatePerFrame) EndGameProcess_(State, #OnUpdatePerFrame)

inline void
AttachChildGameProcess(game_state *State, char *ParentProcessName, char *ChildProcessName, game_process_on_update *ChildOnUpdatePerFrame, game_process_params ChildParams)
{
    game_process *ParentProcess = GetGameProcess(State, ParentProcessName);
    game_process *ChildProcess = GetGameProcess(State, ChildProcessName);

    if (IsSlotEmpty(ChildProcess->Key))
    {
        InitGameProcess(ChildProcess, ChildProcessName, ChildOnUpdatePerFrame, ChildParams);
    }

    ParentProcess->Child = ChildProcess;
}

DLLExport GAME_PROCESS_ON_UPDATE(RigidBodyOrientationLerpProcess)
{
    rigid_body *Body = Process->Params.Entity->Body;

    if (Body->OrientationLerp.Duration > 0.f)
    {
        Body->OrientationLerp.Time += Delta;

        f32 t = Body->OrientationLerp.Time / Body->OrientationLerp.Duration;

        if (t <= 1.f)
        {
            Body->Orientation = Slerp(Body->OrientationLerp.From, t, Body->OrientationLerp.To);
        }
        else
        {
            EndGameProcess_(State, Process->Key);
        }
    }
}

DLLExport GAME_PROCESS_ON_UPDATE(RigidBodyPositionLerpProcess)
{
    rigid_body *Body = Process->Params.Entity->Body;

    if (Body->PositionLerp.Duration > 0.f)
    {
        Body->PositionLerp.Time += Delta;

        f32 t = Body->PositionLerp.Time / Body->PositionLerp.Duration;

        if (t <= 1.f)
        {
            Body->Position = Lerp(Body->PositionLerp.From, t, Body->PositionLerp.To);
        }
        else
        {
            EndGameProcess_(State, Process->Key);
        }
    }
}

DLLExport GAME_PROCESS_ON_UPDATE(SmoothInputMoveProcess)
{
    f32 InterpolationTime = Process->Params.InterpolationTime;

    vec2 dMove = (State->TargetMove - State->CurrentMove) / InterpolationTime;
    State->CurrentMove += dMove * Delta;
}
