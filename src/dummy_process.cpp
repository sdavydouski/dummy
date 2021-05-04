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
    if (State->Player->Body->OrientationLerp.Duration > 0.f)
    {
        State->Player->Body->OrientationLerp.Time += Delta;

        f32 t = State->Player->Body->OrientationLerp.Time / State->Player->Body->OrientationLerp.Duration;

        if (t <= 1.f)
        {
            State->Player->Body->Orientation = Slerp(State->Player->Body->OrientationLerp.From, t, State->Player->Body->OrientationLerp.To);
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