#pragma once

struct stream_chunk
{
    u32 Size;
    u8 *Contents;

    stream_chunk *Next;
};

struct stream
{
    stream_chunk *First;
    stream_chunk *Last;

    memory_arena Arena;
};

inline stream_chunk *
AppendChunk(stream *Stream, u32 Size, u8 *Contents)
{
    stream_chunk *Chunk = PushType(&Stream->Arena, stream_chunk);
    Chunk->Size = Size;
    Chunk->Contents = Contents;
    Chunk->Next = 0;

    if (Stream->Last)
    {
        Stream->Last->Next = Chunk;
        Stream->Last = Stream->Last->Next;
    }
    else
    {
        Stream->First = Chunk;
        Stream->Last = Stream->First;
    }

    return Chunk;
}

inline void
ClearStream(stream *Stream)
{
    ClearMemoryArena(&Stream->Arena);
    Stream->First = 0;
    Stream->Last = 0;
}

inline stream
CreateStream(memory_arena Arena)
{
    stream Result = {};

    Result.Arena = Arena;

    return Result;
}

//#define Out(...) Out_((__FILE__, __LINE__, __VA_ARGS__)

dummy_internal void
Out(stream *Dest, const char *Format, ...)
{
    va_list Args;

    char String[512];

    va_start(Args, Format);
    u32 Size = FormatStringArgs(String, ArrayCount(String), Format, Args);
    va_end(Args);

    u8 *Contents = PushArray(&Dest->Arena, Size, u8);
    CopyMemory(String, Contents, Size);

    AppendChunk(Dest, Size, Contents);
}
