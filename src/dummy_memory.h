#pragma once

#include <cstring>

#define Kilobytes(Bytes) (Bytes * 1024LL)
#define Megabytes(Bytes) (Kilobytes(Bytes) * 1024LL)
#define Gigabytes(Bytes) (Megabytes(Bytes) * 1024LL)
#define Terabytes(Bytes) (Gigabytes(Bytes) * 1024LL)

inline void
ClearMemory(void *Memory, umm Size)
{
    memset(Memory, 0, Size);
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
PushSize(memory_arena *Arena, umm Size, umm Alignment = 4)
{
    umm RawAddress = (umm)Arena->Base + Arena->Used;
    umm AlignedAddress = AlignAddress(RawAddress, Alignment);
    umm AlignmentOffset = AlignedAddress - RawAddress;

    umm ActualSize = Size + AlignmentOffset;

    Assert(Arena->Used + ActualSize < Arena->Size);

    void *Result = (void *)AlignedAddress;
    Arena->Used += ActualSize;

    // todo: control flags
    ClearMemory(Result, ActualSize);

    return Result;
}

#define PushType(Arena, Type, ...) (Type *)PushSize(Arena, sizeof(Type), __VA_ARGS__)
#define PushArray(Arena, Count, Type, ...) (Type *)PushSize(Arena, Count * sizeof(Type), __VA_ARGS__)
#define PushString(Arena, Count, ...) (char *)PushArray(Arena, Count, char, __VA_ARGS__)
