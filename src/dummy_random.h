#pragma once

struct random_sequence
{
    u32 State;
};

inline random_sequence
RandomSequence(u32 Seed)
{
    random_sequence Result = {};
    Result.State = Seed;

    return Result;
}

inline u32
RandomNextU32(random_sequence *Sequence)
{
    // Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs"
    u32 Result = Sequence->State;
    Result ^= Result << 13;
    Result ^= Result >> 17;
    Result ^= Result << 5;
    Sequence->State = Result;

    return Result;
}

inline u32
RandomChoice(random_sequence *Sequence, u32 ChoiceCount)
{
    u32 Result = RandomNextU32(Sequence) % ChoiceCount;
    return Result;
}

inline f32 Random01(random_sequence *Sequence)
{
    f32 Result = (f32)RandomNextU32(Sequence) / U32_MAX;
    return Result;
}

inline f32
RandomBetween(random_sequence *Sequence, f32 Min, f32 Max)
{
    f32 Result = Lerp(Min, Random01(Sequence), Max);
    return Result;
}

inline i32
RandomBetween(random_sequence *Sequence, i32 Min, i32 Max)
{
    i32 Result = Min + (i32)(RandomNextU32(Sequence) % ((Max + 1) - Min));
    return Result;
}
