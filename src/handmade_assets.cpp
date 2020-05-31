#include "handmade_assets.h"

internal model_asset
ReadModelAsset(platform_api *Platform, char *FileName, memory_arena *Arena)
{
    model_asset Result = {};

    read_file_result AssetFile = Platform->ReadFile(Platform->PlatformHandle, FileName, Arena);
    void *Buffer = AssetFile.Contents;

    model_asset_header *Header = (model_asset_header *)Buffer;

    model_asset_skeleton_header *SkeletonHeader = (model_asset_skeleton_header *)((u8 *)Buffer + Header->SkeletonHeaderOffset);
    Result.Skeleton.JointCount = SkeletonHeader->JointCount;
    Result.Skeleton.Joints = (joint *)((u8 *)Buffer + SkeletonHeader->JointsOffset);
    Result.Skeleton.LocalJointPoses = (joint_pose *)((u8 *)Buffer + SkeletonHeader->LocalJointPosesOffset);
    Result.Skeleton.GlobalJointPoses = (mat4 *)((u8 *)Buffer + SkeletonHeader->GlobalJointPosesOffset);

    model_asset_meshes_header *MeshesHeader = (model_asset_meshes_header *)((u8 *)Buffer + Header->MeshesHeaderOffset);
    Result.MeshCount = MeshesHeader->MeshCount;
    Result.Meshes = PushArray(Arena, Result.MeshCount, mesh);

    u32 NextMeshHeaderOffset = 0;
    for (u32 MeshIndex = 0; MeshIndex < MeshesHeader->MeshCount; ++MeshIndex)
    {
        model_asset_mesh_header *MeshHeader = (model_asset_mesh_header *)((u8 *)Buffer +
            MeshesHeader->MeshesOffset + NextMeshHeaderOffset);
        
        mesh *Mesh = Result.Meshes + MeshIndex;

        Mesh->VertexCount = MeshHeader->VertexCount;
        Mesh->IndexCount = MeshHeader->IndexCount;
        Mesh->Vertices = (vertex *)((u8 *)Buffer + MeshHeader->VerticesOffset);
        Mesh->Indices = (u32 *)((u8 *)Buffer + MeshHeader->IndicesOffset);

        NextMeshHeaderOffset += sizeof(model_asset_mesh_header) +
            MeshHeader->VertexCount * sizeof(vertex) + MeshHeader->IndexCount * sizeof(u32);
    }

    return Result;
}