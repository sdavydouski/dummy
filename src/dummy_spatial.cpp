inline void
InitSpatialHashGrid(spatial_hash_grid *Grid, aabb Bounds, vec3 CellSize, memory_arena *Arena)
{
    Grid->Bounds = Bounds;
    Grid->CellSize = CellSize;

    Grid->CellCount.x = Ceil((Bounds.Max.x - Bounds.Min.x) / CellSize.x);
    Grid->CellCount.y = Ceil((Bounds.Max.y - Bounds.Min.y) / CellSize.y);
    Grid->CellCount.z = Ceil((Bounds.Max.z - Bounds.Min.z) / CellSize.z);

    Grid->TotalCellCount = Grid->CellCount.x * Grid->CellCount.y * Grid->CellCount.z;
    Grid->Cells = PushArray(Arena, Grid->TotalCellCount, spatial_hash_grid_cell);
}

inline u32
Hash(spatial_hash_grid *Grid, i32 CellX, i32 CellY, i32 CellZ)
{
#if 0
    // http://www.beosil.com/download/CollisionDetectionHashing_VMV03.pdf
    u32 Result = (CellX * 73856093) ^ (CellY * 19349663) ^ (CellZ * 83492791);
#else
    // https://developer.nvidia.com/gpugems/gpugems3/part-v-physics-simulation/chapter-32-broad-phase-collision-detection-cuda
    u32 Result = (CellX << 0) | (CellY << 28) | (CellZ << 14);
#endif
    return Result;
}

inline ivec3
GetCellCoordinates(spatial_hash_grid *Grid, vec3 Position)
{
    ivec3 Result;

    f32 NormalizedPositionX = Clamp((Position.x - Grid->Bounds.Min.x) / (Grid->Bounds.Max.x - Grid->Bounds.Min.x), 0.f, 1.f);
    f32 NormalizedPositionY = Clamp((Position.y - Grid->Bounds.Min.y) / (Grid->Bounds.Max.y - Grid->Bounds.Min.y), 0.f, 1.f);
    f32 NormalizedPositionZ = Clamp((Position.z - Grid->Bounds.Min.z) / (Grid->Bounds.Max.z - Grid->Bounds.Min.z), 0.f, 1.f);

    Result.x = Floor(NormalizedPositionX * (Grid->CellCount.x - 1));
    Result.y = Floor(NormalizedPositionY * (Grid->CellCount.y - 1));
    Result.z = Floor(NormalizedPositionZ * (Grid->CellCount.z - 1));

    return Result;
}

inline spatial_hash_grid_cell *
GetGridCell(spatial_hash_grid *Grid, i32 CellX, i32 CellY, i32 CellZ)
{
    u32 HashValue = Hash(Grid, CellX, CellY, CellZ);
    u32 HashSlot = HashValue % Grid->TotalCellCount;

    spatial_hash_grid_cell *Cell = Grid->Cells + HashSlot;

    return Cell;
}

internal void
AddToSpacialGrid(spatial_hash_grid *Grid, game_entity *Entity)
{
    aabb EntityBounds = GetEntityBoundingBox(Entity);

    ivec3 MinCellCoords = GetCellCoordinates(Grid, EntityBounds.Min);
    ivec3 MaxCellCoords = GetCellCoordinates(Grid, EntityBounds.Max);

    Entity->GridCellCoords[0] = MinCellCoords;
    Entity->GridCellCoords[1] = MaxCellCoords;

    // Adding entity to each cell
    for (i32 CellY = MinCellCoords.y; CellY <= MaxCellCoords.y; ++CellY)
    {
        for (i32 CellZ = MinCellCoords.z; CellZ <= MaxCellCoords.z; ++CellZ)
        {
            for (i32 CellX = MinCellCoords.x; CellX <= MaxCellCoords.x; ++CellX)
            {
                spatial_hash_grid_cell *Cell = GetGridCell(Grid, CellX, CellY, CellZ);

                Assert(Cell->EntityCount < ArrayCount(Cell->Entities));

                Cell->Entities[Cell->EntityCount++] = Entity;
            }
        }
    }
}

internal void
RemoveFromSpacialGrid(spatial_hash_grid *Grid, game_entity *Entity)
{
    ivec3 MinCellCoords = Entity->GridCellCoords[0];
    ivec3 MaxCellCoords = Entity->GridCellCoords[1];

    // Removing entity from each cell
    for (i32 CellY = MinCellCoords.y; CellY <= MaxCellCoords.y; ++CellY)
    {
        for (i32 CellZ = MinCellCoords.z; CellZ <= MaxCellCoords.z; ++CellZ)
        {
            for (i32 CellX = MinCellCoords.x; CellX <= MaxCellCoords.x; ++CellX)
            {
                spatial_hash_grid_cell *Cell = GetGridCell(Grid, CellX, CellY, CellZ);

                for (u32 EntityIndex = 0; EntityIndex < Cell->EntityCount; ++EntityIndex)
                {
                    game_entity **CellEntity = Cell->Entities + EntityIndex;

                    if ((*CellEntity)->Id == Entity->Id)
                    {
                        u32 LastCellIndex = Cell->EntityCount - 1;
                        game_entity **LastCellEntity = Cell->Entities + LastCellIndex;

                        *CellEntity = *LastCellEntity;
                        --Cell->EntityCount;

                        Assert(Cell->EntityCount >= 0);
                    }
                }
            }
        }
    }
}

internal void
UpdateInSpacialGrid(spatial_hash_grid *Grid, game_entity *Entity)
{
    aabb EntityBounds = GetEntityBoundingBox(Entity);

    ivec3 MinCellCoords = GetCellCoordinates(Grid, EntityBounds.Min);
    ivec3 MaxCellCoords = GetCellCoordinates(Grid, EntityBounds.Max);

    if (Entity->GridCellCoords[0] != MinCellCoords || Entity->GridCellCoords[1] != MaxCellCoords)
    {
        RemoveFromSpacialGrid(Grid, Entity);
        AddToSpacialGrid(Grid, Entity);
    }
}

internal u32
FindNearbyEntities(spatial_hash_grid *Grid, game_entity *Entity, aabb Bounds, game_entity **Entities, u32 MaxEntityCount)
{
    aabb EntityBounds = GetEntityBoundingBox(Entity);

    aabb AreaBounds;
    AreaBounds.Min = EntityBounds.Min + Bounds.Min;
    AreaBounds.Max = EntityBounds.Max + Bounds.Max;

    ivec3 MinCellCoords = GetCellCoordinates(Grid, AreaBounds.Min);
    ivec3 MaxCellCoords = GetCellCoordinates(Grid, AreaBounds.Max);

    u32 EntityCount = 0;

    for (i32 CellY = MinCellCoords.y; CellY <= MaxCellCoords.y; ++CellY)
    {
        for (i32 CellZ = MinCellCoords.z; CellZ <= MaxCellCoords.z; ++CellZ)
        {
            for (i32 CellX = MinCellCoords.x; CellX <= MaxCellCoords.x; ++CellX)
            {
                spatial_hash_grid_cell *Cell = GetGridCell(Grid, CellX, CellY, CellZ);

                for (u32 CellEntityIndex = 0; CellEntityIndex < Cell->EntityCount; ++CellEntityIndex)
                {
                    game_entity *CellEntity = Cell->Entities[CellEntityIndex];
                    aabb CellEntityBounds = GetEntityBoundingBox(CellEntity);

                    if (CellEntity->Id != Entity->Id && TestAABBAABB(AreaBounds, CellEntityBounds))
                    {
                        b32 ShouldAdd = true;

                        // Filtering duplicates
                        for (u32 EntityIndex = 0; EntityIndex < EntityCount; ++EntityIndex)
                        {
                            game_entity *Entity = Entities[EntityIndex];

                            if (Entity->Id == CellEntity->Id)
                            {
                                ShouldAdd = false;
                                break;
                            }
                        }

                        if (ShouldAdd)
                        {
                            Entities[EntityCount++] = CellEntity;

                            Assert(EntityCount <= MaxEntityCount);

                            if (EntityCount == MaxEntityCount)
                            {
                                return EntityCount;
                            }
                        }
                    }
                }
            }
        }
    }

    return EntityCount;
}
