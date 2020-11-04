#pragma once

#include <cstring>

#define Kilobytes(Bytes) (Bytes * 1024LL)
#define Megabytes(Bytes) (Kilobytes(Bytes) * 1024LL)
#define Gigabytes(Bytes) (Megabytes(Bytes) * 1024LL)
#define Terabytes(Bytes) (Gigabytes(Bytes) * 1024LL)

// todo: alignment

inline void
ClearMemory(void *Memory, umm Size)
{
    memset(Memory, 0, Size);
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

inline void *
PushSize(memory_arena *Arena, umm Size)
{
    Assert(Arena->Used + Size < Arena->Size);

    void *Result = (u8 *)Arena->Base + Arena->Used;
    Arena->Used += Size;

    // todo: control flags
    ClearMemory(Result, Size);

    return Result;
}

#define PushType(Arena, Type) (Type *)PushSize(Arena, sizeof(Type))
#define PushArray(Arena, Count, Type) (Type *)PushSize(Arena, Count * sizeof(Type))
#define PushString(Arena, Count) (char *)PushSize(Arena, Count * sizeof(char))
