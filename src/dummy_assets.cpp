internal model_asset *
LoadModelAsset(platform_api *Platform, char *FileName, memory_arena *Arena)
{
    model_asset *Result = PushType(Arena, model_asset);

    read_file_result AssetFile = Platform->ReadFile(FileName, Arena, false);
    void *Buffer = AssetFile.Contents;

    model_asset_header *Header = (model_asset_header *)Buffer;

    // Skeleton
    model_asset_skeleton_header *SkeletonHeader = (model_asset_skeleton_header *)((u8 *)Buffer + Header->SkeletonHeaderOffset);
    Result->Skeleton.JointCount = SkeletonHeader->JointCount;
    Result->Skeleton.Joints = (joint *)((u8 *)Buffer + SkeletonHeader->JointsOffset);

    // Skeleton Bind Pose
    model_asset_skeleton_pose_header *SkeletonPoseHeader = (model_asset_skeleton_pose_header *)((u8 *)Buffer + Header->SkeletonPoseHeaderOffset);
    Result->BindPose.Skeleton = &Result->Skeleton;
    Result->BindPose.LocalJointPoses = (joint_pose *)((u8 *)Buffer + SkeletonPoseHeader->LocalJointPosesOffset);
    Result->BindPose.GlobalJointPoses = (mat4 *)((u8 *)Buffer + SkeletonPoseHeader->GlobalJointPosesOffset);

    // Meshes
    model_asset_meshes_header *MeshesHeader = (model_asset_meshes_header *)((u8 *)Buffer + Header->MeshesHeaderOffset);
    Result->MeshCount = MeshesHeader->MeshCount;
    Result->Meshes = PushArray(Arena, Result->MeshCount, mesh);

    u64 NextMeshHeaderOffset = 0;
    for (u32 MeshIndex = 0; MeshIndex < MeshesHeader->MeshCount; ++MeshIndex)
    {
        model_asset_mesh_header *MeshHeader = (model_asset_mesh_header *)((u8 *)Buffer +
            MeshesHeader->MeshesOffset + NextMeshHeaderOffset);
        
        mesh *Mesh = Result->Meshes + MeshIndex;

        Mesh->MaterialIndex = MeshHeader->MaterialIndex;
        Mesh->VertexCount = MeshHeader->VertexCount;
        Mesh->IndexCount = MeshHeader->IndexCount;

        u32 VerticesOffset = 0;

        if (MeshHeader->HasPositions)
        {
            Mesh->Positions = (vec3 *)((u8 *)Buffer + MeshHeader->VerticesOffset + VerticesOffset);
            VerticesOffset += sizeof(vec3) * MeshHeader->VertexCount;
        }

        if (MeshHeader->HasNormals)
        {
            Mesh->Normals = (vec3 *)((u8 *)Buffer + MeshHeader->VerticesOffset + VerticesOffset);
            VerticesOffset += sizeof(vec3) * MeshHeader->VertexCount;
        }

        if (MeshHeader->HasTangents)
        {
            Mesh->Tangents = (vec3 *)((u8 *)Buffer + MeshHeader->VerticesOffset + VerticesOffset);
            VerticesOffset += sizeof(vec3) * MeshHeader->VertexCount;
        }

        if (MeshHeader->HasBitangets)
        {
            Mesh->Bitangents = (vec3 *)((u8 *)Buffer + MeshHeader->VerticesOffset + VerticesOffset);
            VerticesOffset += sizeof(vec3) * MeshHeader->VertexCount;
        }

        if (MeshHeader->HasTextureCoords)
        {
            Mesh->TextureCoords = (vec2 *)((u8 *)Buffer + MeshHeader->VerticesOffset + VerticesOffset);
            VerticesOffset += sizeof(vec2) * MeshHeader->VertexCount;
        }

        if (MeshHeader->HasWeights)
        {
            Mesh->Weights = (vec4 *)((u8 *)Buffer + MeshHeader->VerticesOffset + VerticesOffset);
            VerticesOffset += sizeof(vec4) * MeshHeader->VertexCount;
        }

        if (MeshHeader->HasJointIndices)
        {
            Mesh->JointIndices = (i32 *)((u8 *)Buffer + MeshHeader->VerticesOffset + VerticesOffset);
            VerticesOffset += sizeof(i32) * 4 * MeshHeader->VertexCount;
        }

        Mesh->Indices = (u32 *)((u8 *)Buffer + MeshHeader->IndicesOffset);

        NextMeshHeaderOffset += sizeof(model_asset_mesh_header) + VerticesOffset + MeshHeader->IndexCount * sizeof(u32);
    }

    // Materials
    model_asset_materials_header *MaterialsHeader = (model_asset_materials_header *)((u8 *)Buffer + Header->MaterialsHeaderOffset);

    Result->MaterialCount = MaterialsHeader->MaterialCount;
    Result->Materials = PushArray(Arena, Result->MaterialCount, mesh_material);

    u64 NextMaterialHeaderOffset = 0;
    for (u32 MaterialIndex = 0; MaterialIndex < MaterialsHeader->MaterialCount; ++MaterialIndex)
    {
        model_asset_material_header *MaterialHeader = (model_asset_material_header *)((u8 *)Buffer +
            MaterialsHeader->MaterialsOffset + NextMaterialHeaderOffset);
        mesh_material *Material = Result->Materials + MaterialIndex;
        Material->PropertyCount = MaterialHeader->PropertyCount;
        Material->Properties = PushArray(Arena, Material->PropertyCount, material_property);

        u64 NextMaterialPropertyHeaderOffset = 0;
        for (u32 MaterialPropertyIndex = 0; MaterialPropertyIndex < MaterialHeader->PropertyCount; ++MaterialPropertyIndex)
        {
            model_asset_material_property_header *MaterialPropertyHeader = (model_asset_material_property_header *)
                ((u8 *)Buffer + MaterialHeader->PropertiesOffset + NextMaterialPropertyHeaderOffset);

            material_property *MaterialProperty = Material->Properties + MaterialPropertyIndex;
            MaterialProperty->Type = MaterialPropertyHeader->Type;

            switch (MaterialProperty->Type)
            {
                case MaterialProperty_Float_Shininess:
                {
                    MaterialProperty->Value = MaterialPropertyHeader->Value;

                    NextMaterialPropertyHeaderOffset += sizeof(model_asset_material_property_header);

                    break;
                }
                case MaterialProperty_Color_Ambient:
                case MaterialProperty_Color_Diffuse:
                case MaterialProperty_Color_Specular:
                {
                    MaterialProperty->Color = MaterialPropertyHeader->Color;

                    NextMaterialPropertyHeaderOffset += sizeof(model_asset_material_property_header);

                    break;
                }
                case MaterialProperty_Texture_Diffuse:
                case MaterialProperty_Texture_Specular:
                case MaterialProperty_Texture_Shininess:
                case MaterialProperty_Texture_Normal:
                {
                    MaterialProperty->Bitmap = MaterialPropertyHeader->Bitmap;
                    MaterialProperty->Bitmap.Pixels = (void *)((u8 *)Buffer + MaterialPropertyHeader->BitmapOffset);

                    u32 BitmapSize = MaterialProperty->Bitmap.Width * MaterialProperty->Bitmap.Height * MaterialProperty->Bitmap.Channels;

                    NextMaterialPropertyHeaderOffset += sizeof(model_asset_material_property_header) + BitmapSize;

                    break;
                }
                default:
                {
                    Assert(!"Invalid material property");
                }
            }
        }

        NextMaterialHeaderOffset += sizeof(model_asset_material_header) + NextMaterialPropertyHeaderOffset;
    }

    // Animations
    model_asset_animations_header *AnimationsHeader = (model_asset_animations_header *)
        ((u8 *)Buffer + Header->AnimationsHeaderOffset);
    Result->AnimationCount = AnimationsHeader->AnimationCount;
    Result->Animations = PushArray(Arena, Result->AnimationCount, animation_clip);

    u64 NextAnimationHeaderOffset = 0;
    for (u32 AnimationIndex = 0; AnimationIndex < AnimationsHeader->AnimationCount; ++AnimationIndex)
    {
        model_asset_animation_header *AnimationHeader = (model_asset_animation_header *)
            ((u8 *)Buffer +AnimationsHeader->AnimationsOffset + NextAnimationHeaderOffset);

        animation_clip *Animation = Result->Animations + AnimationIndex;

        CopyString(AnimationHeader->Name, Animation->Name, MAX_ANIMATION_NAME_LENGTH);
        Animation->Duration = AnimationHeader->Duration;
        Animation->IsLooping = AnimationHeader->IsLooping;
        Animation->InPlace = AnimationHeader->InPlace;
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