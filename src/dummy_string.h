#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

inline bool32
StringEquals(const char *Str1, const char *Str2) {
    bool32 Result = strcmp(Str1, Str2) == 0;
    return Result;
}

inline bool32
StringEquals(const wchar *Str1, const wchar *Str2) {
    bool32 Result = wcscmp(Str1, Str2) == 0;
    return Result;
}

inline u32
StringLength(const char *String) {
    u32 Result = (u32)strlen(String);
    return Result;
}

inline u32
StringLength(const wchar *String) {
    u32 Result = (u32)wcslen(String);
    return Result;
}

inline void
ConcatenateString(char *Dest, char *Source, u32 DestLength)
{
    strcat_s(Dest, DestLength, Source);
}

inline void
ConcatenateString(wchar *Dest, wchar *Source, u32 DestLength)
{
    wcscat_s(Dest, DestLength, Source);
}

inline void
CopyString_(const char *Source, char *Dest, u32 DestLength)
{
    strcpy_s(Dest, DestLength, Source);
}

inline void
CopyString_(const wchar *Source, wchar *Dest, u32 DestLength)
{
    wcscpy_s(Dest, DestLength, Source);
}

#define CopyString(Source, Dest) CopyString_(Source, Dest, sizeof(Dest))

inline char *
GetLastAfterDelimiter(char *String, const char Delimiter) {
    char *Result = String;

    for (char *Scan = String; *Scan; ++Scan) {
        if (*Scan == Delimiter) {
            Result = Scan + 1;
        }
    }

    return Result;
}

inline wchar *
GetLastAfterDelimiter(wchar *String, const wchar Delimiter) {
    wchar *Result = String;

    for (wchar *Scan = String; *Scan; ++Scan) {
        if (*Scan == Delimiter) {
            Result = Scan + 1;
        }
    }

    return Result;
}

inline void
CopyStringUnsafe(const char *Source, char *Dest, u32 DestLength)
{
    for (u32 Index = 0; Index < DestLength; ++Index)
    {
        Dest[Index] = Source[Index];
    }

    Dest[DestLength] = 0;
}

inline u32
FormatString_(char *String, u32 Size, const char *Format, ...)
{
    va_list ArgPtr;

    va_start(ArgPtr, Format);
    u32 StringSize = vsnprintf(String, Size, Format, ArgPtr);
    va_end(ArgPtr);

    return StringSize;
}

inline u32
FormatString_(wchar *String, u32 Size, const wchar *Format, ...)
{
    va_list ArgPtr;

    va_start(ArgPtr, Format);
    u32 StringSize = _vsnwprintf_s(String, Size, Size, Format, ArgPtr);
    va_end(ArgPtr);

    return StringSize;
}

#define FormatString(String, Format, ...) FormatString_(String, ArrayCount(String), Format, __VA_ARGS__)

inline u32
FormatStringArgs(char *String, u32 Size, const char *Format, va_list Args)
{
    u32 StringSize = vsnprintf(String, Size, Format, Args);
    return StringSize;
}

inline void
GetDirectoryPath(const char *FilePath, char *DirectoryPath)
{
    const char *LastDelimiter = FilePath;

    for (const char *Scan = FilePath; *Scan; ++Scan)
    {
        if (*Scan == '/')
        {
            LastDelimiter = Scan + 1;
        }
    }

    CopyStringUnsafe(FilePath, DirectoryPath, (u32)(LastDelimiter - FilePath));
}

inline void
ConvertToWideString(const char *Source, wchar *Dest)
{
    u32 SourceLength = StringLength(Source);
    u32 DestLength = StringLength(Dest);

    mbstowcs_s(0, Dest, SourceLength + 1, Source, SourceLength);
}

inline void
ConvertToString(const wchar *Source, char *Dest)
{
    u32 SourceLength = StringLength(Source);
    u32 DestLength = StringLength(Dest);

    wcstombs_s(0, Dest, SourceLength + 1, Source, SourceLength);
}

inline bool32
StringIncludes(char *Str1, char *Str2)
{
    if (strstr(Str1, Str2))
    {
        return true;
    }

    return false;
}

inline char *
SplitString(char *String, const char *Delimiter, char **Context)
{
    char *Token = strtok_s(String, Delimiter, Context);
    return Token;
}
