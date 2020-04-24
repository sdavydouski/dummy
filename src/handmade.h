#pragma once

#include "handmade_renderer.h"

enum game_mode
{
	GameMode_None,
	GameMode_World,
	GameMode_Menu
};

struct game_camera
{
	vec3 Position;
	vec3 Direction;
	vec3 Up;

	f32 Pitch;
	f32 Yaw;
	f32 FovY;
};

#if 0
struct octree
{
	aabb Region;

	u32 ObjectCount;
	rigid_body *Objects;
	
	octree *Children[8];
	octree *Parent;
};

// https://www.gamedev.net/articles/programming/general-and-gameplay-programming/introduction-to-octrees-r3529/
internal void
BuildOctree(octree *Tree, vec3 MinOctantSize, memory_arena *Arena)
{
	if (Tree->ObjectCount <= 1)
	{
		return;
	}

	vec3 Dimension = Tree->Region.Max - Tree->Region.Min;

	if (Dimension.x <= MinOctantSize.x && Dimension.y <= MinOctantSize.y && Dimension.z <= MinOctantSize.z)
	{
		return;
	}

	vec3 Center = Tree->Region.Min + Dimension / 2.f;

	aabb SubdividedRegions[8];
	SubdividedRegions[0] = CreateAABB(Tree->Region.Min, Center);
	SubdividedRegions[1] = CreateAABB(vec3(Center.x, Tree->Region.Min.y, Tree->Region.Min.z), vec3(Tree->Region.Max.x, Center.y, Center.z));
	SubdividedRegions[2] = CreateAABB(vec3(Center.x, Tree->Region.Min.y, Center.z), vec3(Tree->Region.Max.x, Center.y, Tree->Region.Max.z));
	SubdividedRegions[3] = CreateAABB(vec3(Tree->Region.Min.x, Tree->Region.Min.y, Center.z), vec3(Center.x, Center.y, Tree->Region.Max.z));
	SubdividedRegions[4] = CreateAABB(vec3(Tree->Region.Min.x, Center.y, Tree->Region.Min.z), vec3(Center.x, Tree->Region.Max.y, Center.z));
	SubdividedRegions[5] = CreateAABB(vec3(Center.x, Center.y, Tree->Region.Min.z), vec3(Tree->Region.Max.x, Tree->Region.Max.y, Center.z));
	SubdividedRegions[6] = CreateAABB(Center, Tree->Region.Max);
	SubdividedRegions[7] = CreateAABB(vec3(Tree->Region.Min.x, Center.y, Center.z), vec3(Center.x, Tree->Region.Max.y, Tree->Region.Max.z));

	u32 CurrentObjectIndexToDelete = 0;
	u32 *ObjectIndicesToDelete = PushArray(Arena, Tree->ObjectCount, u32);

	for (u32 ObjectIndex = 0; ObjectIndex < Tree->ObjectCount; ++ObjectIndex)
	{
		rigid_body *Object = Tree->Objects + ObjectIndex;
		aabb ObjectAABB = GetRigidBodyAABB(Object);

		for (u32 SubdividedRegionIndex = 0; SubdividedRegionIndex < 8; ++SubdividedRegionIndex)
		{
			if (Contains(SubdividedRegions[SubdividedRegionIndex], ObjectAABB))
			{
				/*
					octList[a].Add(obj);
				*/
				u32 *ObjectIndexToDelete = ObjectIndicesToDelete + CurrentObjectIndexToDelete++;
				*ObjectIndexToDelete = ObjectIndex;
				break;
			}
		}
	}

	// delete every moved object from this node.
	u32 ObjectsToStayCount = Tree->ObjectCount - CurrentObjectIndexToDelete;
	u32 ObjectsToStayCurrentIndex = 0;
	rigid_body *ObjectsToStay = PushArray(Arena, ObjectsToStayCount, rigid_body);

	CurrentObjectIndexToDelete = 0;
	for (u32 ObjectIndex = 0; ObjectIndex < Tree->ObjectCount; ++ObjectIndex)
	{
		if (ObjectIndex == CurrentObjectIndexToDelete)
		{
			++CurrentObjectIndexToDelete;
		}
		else
		{
			rigid_body *Object = ObjectsToStay + ObjectsToStayCurrentIndex++;
			*Object = *(Tree->Objects + ObjectIndex);
		}
	}

	Tree->ObjectCount = ObjectsToStayCount;
	Tree->Objects = ObjectsToStay;
	//



	for (u32 SubdividedRegionIndex = 0; SubdividedRegionIndex < 8; ++SubdividedRegionIndex)
	{

	}
}

#endif

struct game_state
{
	memory_arena WorldArena;

	game_mode Mode;
	game_camera PlayerCamera;
	game_camera DebugCamera;

	u32 RigidBodiesCount;
	rigid_body *RigidBodies;

	plane Ground;

	u32 GridCount;
	b32 IsBackgroundHighlighted;
	b32 Advance;
};
