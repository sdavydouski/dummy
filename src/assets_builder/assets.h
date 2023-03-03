#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using std::string;

template <typename TValue>
using dynamic_array = std::vector<TValue>;

template <typename TKey, typename TValue>
using hashtable = std::unordered_map<TKey, TValue>;

namespace fs = std::filesystem;

template <typename T>
inline T *
AllocateMemory(umm Count = 1)
{
    T *Result = (T *)calloc(Count, sizeof(T));

    Assert(Result);

    return Result;
}

inline u32
GetFileSize(FILE *File)
{
    u32 FileSize = 0;

    fseek(File, 0, SEEK_END);
    FileSize = ftell(File);
    fseek(File, 0, SEEK_SET);

    return FileSize;
}

dummy_internal char *
ReadTextFile(const char *FileName)
{
    char *Result = 0;
    FILE *File = fopen(FileName, "rb");

    if (File)
    {
        u32 Length = GetFileSize(File);

        Result = AllocateMemory<char>(Length + 1);

        fread(Result, 1, Length, File);
        Result[Length] = 0;

        fclose(File);
    }

    return Result;
}
