#pragma once

#include <stdint.h>

#define internal static
#define global static
#define persist static

#define DLLExport __declspec(dllexport)

#define ArrayCount(Array) (sizeof(Array) / sizeof(Array[0]))
#define StructOffset(StructType, StructMember) ((u64)(&(((StructType *)0)->StructMember)))

#define Assert(Expression) if (!(Expression)) { *(volatile int *)0 = 0; }
#define InvalidCodePath Assert(!"Invalid code path")

#define i8 int8_t
#define u8 uint8_t
#define i16 int16_t
#define u16 uint16_t
#define i32 int32_t
#define u32 uint32_t
#define i64 uint64_t
#define u64 uint64_t

#define f32 float
#define f64 double

#define b32 u32

#define umm uintptr_t

#define wchar wchar_t

#define U32_MAX UINT32_MAX;