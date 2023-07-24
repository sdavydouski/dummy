#include "dummy.h"

dummy_internal void
SwapParticles(particle *A, particle *B)
{
    particle Temp = *A;
    *A = *B;
    *B = Temp;
}

dummy_internal i32
PartitionParticles(particle *Particles, i32 LowIndex, i32 HighIndex)
{
    i32 MiddleIndex = (HighIndex + LowIndex) / 2;
    particle Pivot = Particles[MiddleIndex];

    i32 LeftIndex = LowIndex - 1;
    i32 RightIndex = HighIndex + 1;

    while (true)
    {
        do
        {
            LeftIndex += 1;
        } while (Particles[LeftIndex].CameraDistanceSquared > Pivot.CameraDistanceSquared);

        do
        {
            RightIndex -= 1;
        } while (Particles[RightIndex].CameraDistanceSquared < Pivot.CameraDistanceSquared);

        if (LeftIndex >= RightIndex)
        {
            return RightIndex;
        }

        SwapParticles(Particles + LeftIndex, Particles + RightIndex);
    }
}

dummy_internal void
QuickSortParticles(particle *Particles, i32 LowIndex, i32 HighIndex)
{
    if (LowIndex >= 0 && HighIndex >= 0 && LowIndex < HighIndex)
    {
        i32 PivotIndex = PartitionParticles(Particles, LowIndex, HighIndex);

        QuickSortParticles(Particles, LowIndex, PivotIndex);
        QuickSortParticles(Particles, PivotIndex + 1, HighIndex);
    }
}

dummy_internal void
SortParticles(u32 ParticleCount, particle *Particles)
{
    QuickSortParticles(Particles, 0, ParticleCount - 1);
}
