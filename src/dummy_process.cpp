#include "dummy.h"

inline game_process *
GetGameProcess(game_state *State, u32 Id)
{
    game_process *Result = HashTableLookup(&State->Processes, Id);
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
InitGameProcess(game_process *Process, const char *ProcessName, u32 Id, game_process_on_update *OnUpdatePerFrame, game_process_params Params)
{
    CopyString(ProcessName, Process->Name);
    Process->Key = Id;
    Process->OnUpdatePerFrame = OnUpdatePerFrame;
    Process->Params = Params;
}

dummy_internal u32
StartGameProcess_(game_state *State, const char *ProcessName, game_process_on_update *OnUpdatePerFrame, game_process_params Params)
{
    u32 ProcessId = GenerateGameProcessId(State);
    game_process *Process = GetGameProcess(State, ProcessId);

    InitGameProcess(Process, ProcessName, ProcessId, OnUpdatePerFrame, Params);
    AddGameProcess(State, Process);

    return ProcessId;
}

dummy_internal void
EndGameProcess(game_state *State, u32 Id)
{
    game_process *Process = GetGameProcess(State, Id);

    Assert(Process->Key == Id);

    RemoveGameProcess(Process);

    if (Process->Child)
    {
        AddGameProcess(State, Process->Child);
    }

    RemoveFromHashTable(&Process->Key);
}

inline void
AttachChildGameProcess_(game_state *State, u32 ParentProcessId, const char *ChildProcessName, game_process_on_update *ChildOnUpdatePerFrame, game_process_params ChildParams)
{
    u32 ChildProcessId = GenerateGameProcessId(State);

    game_process *ParentProcess = GetGameProcess(State, ParentProcessId);
    game_process *ChildProcess = GetGameProcess(State, ChildProcessId);

    InitGameProcess(ChildProcess, ChildProcessName, ChildProcessId, ChildOnUpdatePerFrame, ChildParams);

    ParentProcess->Child = ChildProcess;
}

#define StartGameProcess(State, OnUpdatePerFrame, Params) StartGameProcess_(State, #OnUpdatePerFrame, OnUpdatePerFrame, Params)
#define AttachChildGameProcess(State, ParentProcessId, ChildOnUpdatePerFrame, ChildParams) AttachChildGameProcess_(State, ParentProcessId, #ChildOnUpdatePerFrame, ChildOnUpdatePerFrame, ChildParams)

// Reloadable game processes

DLLExport GAME_PROCESS_ON_UPDATE(RigidBodyPositionLerpProcess)
{
    rigid_body *Body = Process->Params.Entity->Body;

    Body->PositionLerp.Time += Params->Delta;

    f32 t = Body->PositionLerp.Time / Body->PositionLerp.Duration;

    if (t <= 1.f)
    {
        Body->Position = Lerp(Body->PositionLerp.From, t, Body->PositionLerp.To);
    }
    else
    {
        Body->PositionLerp = {};
        EndGameProcess(State, Process->Key);
    }
}

DLLExport GAME_PROCESS_ON_UPDATE(RigidBodyOrientationLerpProcess)
{
    rigid_body *Body = Process->Params.Entity->Body;

    Body->OrientationLerp.Time += Params->Delta;

    f32 t = Body->OrientationLerp.Time / Body->OrientationLerp.Duration;

    if (t <= 1.f)
    {
        Body->Orientation = Slerp(Body->OrientationLerp.From, t, Body->OrientationLerp.To);
    }
    else
    {
        Body->OrientationLerp = {};
        State->PlayerOrientationLerpProcessId = 0;
        EndGameProcess(State, Process->Key);
    }
}

DLLExport GAME_PROCESS_ON_UPDATE(SmoothInputMoveProcess)
{
    f32 InterpolationTime = Process->Params.InterpolationTime;

    if (State->TargetMove != State->CurrentMove)
    {
        vec2 dMove = (State->TargetMove - State->CurrentMove) / InterpolationTime;
        State->CurrentMove += dMove * Params->Delta;
    }
}

DLLExport GAME_PROCESS_ON_UPDATE(ChangeEntityColorProcess)
{
    game_entity *Entity = Process->Params.Entity;

    Entity->DebugColor.r = Entity->TestColor.r * (Sin(Params->Time * 4.f) + 1.5f) * 0.5f;
    Entity->DebugColor.g = Entity->TestColor.g * (Cos(Params->Time * 4.f) + 1.5f) * 0.5f;
    Entity->DebugColor.b = Entity->TestColor.b * (Sin(Params->Time * 4.f) + 1.5f) * 0.5f;
}
