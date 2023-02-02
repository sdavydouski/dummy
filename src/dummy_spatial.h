#pragma once

struct game_entity;

struct spatial_hash_grid_cell
{
    u32 EntityCount;
    game_entity *Entities[256];
};

struct spatial_hash_grid
{
    bounds Bounds;
    vec3 CellSize;
    ivec3 CellCount;

    u32 TotalCellCount;
    spatial_hash_grid_cell *Cells;
};
