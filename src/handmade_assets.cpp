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

    u64 NextMeshHeaderOffset = 0;
    for (u32 MeshIndex = 0; MeshIndex < MeshesHeader->MeshCount; ++MeshIndex)
    {
        model_asset_mesh_header *MeshHeader = (model_asset_mesh_header *)((u8 *)Buffer +
            MeshesHeader->MeshesOffset + NextMeshHeaderOffset);
        
        mesh *Mesh = Result.Meshes + MeshIndex;

        Mesh->PrimitiveType = MeshHeader->PrimitiveType;
        Mesh->VertexCount = MeshHeader->VertexCount;
        Mesh->IndexCount = MeshHeader->IndexCount;
        Mesh->Vertices = (skinned_vertex *)((u8 *)Buffer + MeshHeader->VerticesOffset);
        Mesh->Indices = (u32 *)((u8 *)Buffer + MeshHeader->IndicesOffset);

        NextMeshHeaderOffset += sizeof(model_asset_mesh_header) +
            MeshHeader->VertexCount * sizeof(skinned_vertex) + MeshHeader->IndexCount * sizeof(u32);
    }

    model_asset_animations_header *AnimationsHeader = (model_asset_animations_header *)
        ((u8 *)Buffer + Header->AnimationsHeaderOffset);
    Result.AnimationCount = AnimationsHeader->AnimationCount;
    Result.Animations = PushArray(Arena, Result.AnimationCount, animation_clip);

    u64 NextAnimationHeaderOffset = 0;
    for (u32 AnimationIndex = 0; AnimationIndex < AnimationsHeader->AnimationCount; ++AnimationIndex)
    {
        model_asset_animation_header *AnimationHeader = (model_asset_animation_header *)
            ((u8 *)Buffer +AnimationsHeader->AnimationsOffset + NextAnimationHeaderOffset);

        animation_clip *Animation = Result.Animations + AnimationIndex;

        CopyString(AnimationHeader->Name, Animation->Name, MAX_ANIMATION_NAME_LENGTH);
        Animation->Duration = AnimationHeader->Duration;
        Animation->PoseSampleCount = AnimationHeader->PoseSampleCount;
        Animation->PoseSamples = PushArray(Arena, Animation->PoseSampleCount, animation_sample);

        u64 NextAnimationSampleHeaderOffset = 0;
        for (u32 AnimationPoseIndex = 0; AnimationPoseIndex < AnimationHeader->PoseSampleCount; ++AnimationPoseIndex)
        {
            model_asset_animation_sample_header *AnimationSampleHeader = (model_asset_animation_sample_header *)
                ((u8 *)Buffer + AnimationHeader->PoseSamplesOffset + NextAnimationSampleHeaderOffset);

            animation_sample *AnimationSample = Animation->PoseSamples + AnimationPoseIndex;

            AnimationSample->JointIndex = AnimationSampleHeader->JointIndex;
            AnimationSample->KeyFrameCount = AnimationSampleHeader->KeyFrameCount;
            AnimationSample->KeyFrames = (key_frame *)((u8 *)Buffer + AnimationSampleHeader->KeyFramesOffset);

            NextAnimationSampleHeaderOffset += sizeof(model_asset_animation_sample_header) +
                AnimationSampleHeader->KeyFrameCount * sizeof(key_frame);
        }

        NextAnimationHeaderOffset += sizeof(model_asset_animation_header) + NextAnimationSampleHeaderOffset;
    }

    return Result;
}