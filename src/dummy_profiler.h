#pragma once

#define PLATFORM_GET_TIMESTAMP(name) u64 name(void)
typedef PLATFORM_GET_TIMESTAMP(platform_get_timestamp);

struct profiler_sample
{
    char Name[64];
    u64 ElapsedTicks;
    f32 ElapsedMilliseconds;
};

struct profiler_frame_samples
{
    u32 SampleCount;
    profiler_sample Samples[64];
};

struct platform_profiler
{
    u64 TicksPerSecond;
    i32 CurrentFrameSampleIndex;
    u32 MaxFrameSampleCount;
    profiler_frame_samples *FrameSamples;

    platform_get_timestamp *GetTimestamp;
};

inline profiler_frame_samples *
ProfilerGetCurrentFrameSamples(platform_profiler *Profiler)
{
    profiler_frame_samples *Result = Profiler->FrameSamples + Profiler->CurrentFrameSampleIndex;
    return Result;
}

inline profiler_frame_samples *
ProfilerGetPreviousFrameSamples(platform_profiler *Profiler)
{
    i32 PreviousFrameSampleIndex = Profiler->CurrentFrameSampleIndex - 1;
    if (PreviousFrameSampleIndex < 0)
    {
        PreviousFrameSampleIndex = Profiler->MaxFrameSampleCount - 1;
    }

    profiler_frame_samples *Result = Profiler->FrameSamples + PreviousFrameSampleIndex;
    return Result;
}

inline void
StoreProfileSample(platform_profiler *Profiler, char *Name, u64 ElapsedTicks)
{
    profiler_sample Sample = {};
    CopyString(Name, Sample.Name);
    Sample.ElapsedTicks = ElapsedTicks;
    Sample.ElapsedMilliseconds = ((f32)ElapsedTicks / (f32)Profiler->TicksPerSecond) * 1000.f;

    profiler_frame_samples *FrameSamples = ProfilerGetCurrentFrameSamples(Profiler);

    FrameSamples->Samples[FrameSamples->SampleCount++] = Sample;
    Assert(FrameSamples->SampleCount < ArrayCount(FrameSamples->Samples));
}

inline void
ProfilerStartFrame(platform_profiler *Profiler)
{
    Profiler->CurrentFrameSampleIndex = (Profiler->CurrentFrameSampleIndex + 1) % Profiler->MaxFrameSampleCount;
    profiler_frame_samples *FrameSamples = ProfilerGetCurrentFrameSamples(Profiler);
    FrameSamples->SampleCount = 0;
}

struct auto_profiler
{
    platform_profiler *Profiler;
    char *Name;
    u64 StartTime;

    auto_profiler(platform_profiler *Profiler, char *Name) : Profiler(Profiler), Name(Name)
    {
        StartTime = Profiler->GetTimestamp();
    }

    ~auto_profiler()
    {
        u64 EndTime = Profiler->GetTimestamp();
        u64 ElapsedTicks = EndTime - StartTime;

        StoreProfileSample(Profiler, Name, ElapsedTicks);
    }
};

#if PROFILER
#define PROFILE(Profiler, Name) auto_profiler Profile(Profiler, (char *) Name)
#define PROFILER_START_FRAME(Profiler) ProfilerStartFrame(Profiler)
#else
#define PROFILE(...) 
#define PROFILER_START_FRAME(...) 
#endif
