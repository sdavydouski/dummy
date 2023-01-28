#pragma once

#include <memory.h>

#undef CopyMemory

#define Kilobytes(Bytes) (Bytes * 1024LL)
#define Megabytes(Bytes) (Kilobytes(Bytes) * 1024LL)
#define Gigabytes(Bytes) (Megabytes(Bytes) * 1024LL)
#define Terabytes(Bytes) (Gigabytes(Bytes) * 1024LL)

inline void
ClearMemory(void *Memory, umm Size)
{
    // todo: abysmally slow
    memset(Memory, 0, Size);
}

inline void
CopyMemory(void *From, void *To, umm Size)
{
    // todo: use smth better?
    memcpy(To, From, Size);
}

inline umm
AlignAddress(umm Address, umm Alignment)
{
    umm AlignmentMask = Alignment - 1;
    umm AlignmentOffset = 0;

    // Alignment must be a power of 2
    Assert((Alignment & AlignmentMask) == 0); 

    if (Address & AlignmentMask)
    {
        AlignmentOffset = Alignment - (Address & AlignmentMask);
    }

    umm Result = Address + AlignmentOffset;

    return Result;
}

struct memory_arena
{
    umm Size;
    umm Used;
    void *Base;
};

struct memory_arena_push_options
{
    umm Alignment;
    bool32 Clear;
};

struct scoped_memory
{
    memory_arena *Arena;
    umm Used;

    scoped_memory(memory_arena *Arena) : Arena(Arena), Used(Arena->Used) {}
    ~scoped_memory()
    {
        Arena->Used = Used;
    }
};

inline memory_arena_push_options
DefaultArenaPushOptions()
{
    memory_arena_push_options Result;
    
    Result.Alignment = 4;
    Result.Clear = true;

    return Result;
}

inline memory_arena_push_options
AlignNoClear(umm Alignment)
{
    memory_arena_push_options Result = DefaultArenaPushOptions();

    Result.Alignment = Alignment;
    Result.Clear = false;

    return Result;
}

inline memory_arena_push_options
AlignClear(umm Alignment)
{
    memory_arena_push_options Result = DefaultArenaPushOptions();

    Result.Alignment = Alignment;
    Result.Clear = true;

    return Result;
}

inline memory_arena_push_options
Align(umm Alignment)
{
    memory_arena_push_options Result = DefaultArenaPushOptions();

    Result.Alignment = Alignment;

    return Result;
}

inline memory_arena_push_options
NoClear()
{
    memory_arena_push_options Result = DefaultArenaPushOptions();

    Result.Clear = false;

    return Result;
}

inline void
InitMemoryArena(memory_arena *Arena, void *Memory, umm Size)
{
    Arena->Base = Memory;
    Arena->Size = Size;
    Arena->Used = 0;
}

inline void
ClearMemoryArena(memory_arena *Arena)
{
    Arena->Used = 0;
}

inline void *
PushSize(memory_arena *Arena, umm Size, memory_arena_push_options Options = DefaultArenaPushOptions())
{
    umm Alignment = Options.Alignment;
    bool32 Clear = Options.Clear;

    umm RawAddress = (umm)Arena->Base + Arena->Used;
    umm AlignedAddress = AlignAddress(RawAddress, Alignment);
    umm AlignmentOffset = AlignedAddress - RawAddress;

    umm ActualSize = Size + AlignmentOffset;

    Assert(Arena->Used + ActualSize < Arena->Size);

    void *Result = (void *)AlignedAddress;
    Arena->Used += ActualSize;

    if (Clear)
    {
        ClearMemory(Result, ActualSize);
    }

    return Result;
}

inline memory_arena
SubMemoryArena(memory_arena *Arena, umm Size, memory_arena_push_options Options = DefaultArenaPushOptions())
{
    memory_arena Result = {};

    Result.Size = Size;
    Result.Base = PushSize(Arena, Size, Options);

    return Result;
}

#define PushType(Arena, Type, ...) (Type *)PushSize(Arena, sizeof(Type), __VA_ARGS__)
#define PushArray(Arena, Count, Type, ...) (Type *)PushSize(Arena, Count * sizeof(Type), __VA_ARGS__)
#define PushString(Arena, Count, ...) (char *)PushArray(Arena, Count, char, __VA_ARGS__)
