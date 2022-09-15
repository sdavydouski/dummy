internal u32
ReadAnimationGraph(animation_graph_asset *GraphAsset, u64 Offset, u8 *Buffer)
{
    model_asset_animation_graph_header *AnimationGraphHeader = (model_asset_animation_graph_header *)(Buffer + Offset);
    CopyString(AnimationGraphHeader->Name, GraphAsset->Name);
    CopyString(AnimationGraphHeader->Entry, GraphAsset->Entry);
    GraphAsset->NodeCount = AnimationGraphHeader->NodeCount;
    GraphAsset->Nodes = (animation_node_asset *)malloc(sizeof(animation_node_asset) * GraphAsset->NodeCount);

    u32 TotalPrevNodeSize = 0;
    for (u32 NodeIndex = 0; NodeIndex < GraphAsset->NodeCount; ++NodeIndex)
    {
        model_asset_animation_node_header *NodeHeader = (model_asset_animation_node_header *)(Buffer + AnimationGraphHeader->NodesOffset + TotalPrevNodeSize);
        animation_node_asset *NodeAsset = GraphAsset->Nodes + NodeIndex;

        NodeAsset->Type = NodeHeader->Type;
        CopyString(NodeHeader->Name, NodeAsset->Name);
        NodeAsset->TransitionCount = NodeHeader->TransitionCount;
        NodeAsset->Transitions = (animation_transition_asset *)(Buffer + NodeHeader->TransitionsOffset);

        switch (NodeAsset->Type)
        {
            case AnimationNodeType_Clip:
            {
                model_asset_animation_state_header *AnimationStateHeader = (model_asset_animation_state_header *)(Buffer + NodeHeader->Offset);

                NodeAsset->Animation = (animation_state_asset *)malloc(sizeof(animation_state_asset) * 1);
                CopyString(AnimationStateHeader->AnimationClipName, NodeAsset->Animation->AnimationClipName);
                NodeAsset->Animation->IsLooping = AnimationStateHeader->IsLooping;

                TotalPrevNodeSize += sizeof(model_asset_animation_state_header);
                break;
            }
            case AnimationNodeType_BlendSpace:
            {
                model_asset_blend_space_1d_header *BlendSpaceHeader = (model_asset_blend_space_1d_header *)(Buffer + NodeHeader->Offset);

                NodeAsset->Blendspace = (blend_space_1d_asset *)malloc(sizeof(blend_space_1d_asset) * 1);
                NodeAsset->Blendspace->ValueCount = BlendSpaceHeader->ValueCount;
                NodeAsset->Blendspace->Values = (blend_space_1d_value_asset *)(Buffer + BlendSpaceHeader->ValuesOffset);

                TotalPrevNodeSize += sizeof(model_asset_blend_space_1d_header) + BlendSpaceHeader->ValueCount * sizeof(blend_space_1d_value_asset);
                break;
            }
            case AnimationNodeType_Graph:
            {
                NodeAsset->Graph = (animation_graph_asset *)malloc(sizeof(animation_graph_asset) * 1);

                u32 NodeSize = ReadAnimationGraph(NodeAsset->Graph, NodeHeader->Offset, Buffer);

                TotalPrevNodeSize += NodeSize + sizeof(model_asset_animation_graph_header);
                break;
            }
        }

        TotalPrevNodeSize += sizeof(model_asset_animation_node_header) + NodeHeader->TransitionCount * sizeof(animation_transition_asset);
    }

    return TotalPrevNodeSize;
}

internal model_asset *
LoadModelAsset(platform_api *Platform, char *FileName, memory_arena *Arena)
{
    model_asset *Result = PushType(Arena, model_asset);

    read_file_result AssetFile = Platform->ReadFile(FileName, Arena, false);
    u8 *Buffer = (u8 *)AssetFile.Contents;

    asset_header *Header = (asset_header *) Buffer;

    Assert(Header->Type == AssetType_Model);

    model_asset_header *ModelHeader = (model_asset_header *) (Buffer + Header->DataOffset);

    // Skeleton
    model_asset_skeleton_header *SkeletonHeader = (model_asset_skeleton_header *)(Buffer + ModelHeader->SkeletonHeaderOffset);
    Result->Skeleton.JointCount = SkeletonHeader->JointCount;
#if 0
    Result->Skeleton.Joints = (joint *)(Buffer + SkeletonHeader->JointsOffset);
#else
    joint *Joints = (joint *)(Buffer + SkeletonHeader->JointsOffset);

    Result->Skeleton.Joints = PushArray(Arena, Result->Skeleton.JointCount, joint, Align(16));

    for (u32 JointIndex = 0; JointIndex < Result->Skeleton.JointCount; ++JointIndex)
    {
        joint *SourceJoint = Joints + JointIndex;
        joint *DestJoint = Result->Skeleton.Joints + JointIndex;

        *DestJoint = *SourceJoint;
    }
#endif

    // Skeleton Bind Pose
    model_asset_skeleton_pose_header *SkeletonPoseHeader = (model_asset_skeleton_pose_header *)(Buffer + ModelHeader->SkeletonPoseHeaderOffset);
    Result->BindPose.Skeleton = &Result->Skeleton;
    Result->BindPose.LocalJointPoses = (joint_pose *)(Buffer + SkeletonPoseHeader->LocalJointPosesOffset);
#if 0
    Result->BindPose.GlobalJointPoses = (mat4 *)(Buffer + SkeletonPoseHeader->GlobalJointPosesOffset);
#else
    Result->BindPose.GlobalJointPoses = PushArray(Arena, Result->Skeleton.JointCount, mat4, Align(16));
#endif

    // Animation Graph
    ReadAnimationGraph(&Result->AnimationGraph, ModelHeader->AnimationGraphHeaderOffset, Buffer);

    // Meshes
    model_asset_meshes_header *MeshesHeader = (model_asset_meshes_header *)(Buffer + ModelHeader->MeshesHeaderOffset);
    Result->MeshCount = MeshesHeader->MeshCount;
    Result->Meshes = PushArray(Arena, Result->MeshCount, mesh);

    u64 NextMeshHeaderOffset = 0;
    for (u32 MeshIndex = 0; MeshIndex < MeshesHeader->MeshCount; ++MeshIndex)
    {
        model_asset_mesh_header *MeshHeader = (model_asset_mesh_header *)(Buffer +
            MeshesHeader->MeshesOffset + NextMeshHeaderOffset);
        
        mesh *Mesh = Result->Meshes + MeshIndex;

        Mesh->MaterialIndex = MeshHeader->MaterialIndex;
        Mesh->VertexCount = MeshHeader->VertexCount;
        Mesh->IndexCount = MeshHeader->IndexCount;

        u32 VerticesOffset = 0;

        if (MeshHeader->HasPositions)
        {
            Mesh->Positions = (vec3 *)(Buffer + MeshHeader->VerticesOffset + VerticesOffset);
            VerticesOffset += sizeof(vec3) * MeshHeader->VertexCount;
        }

        if (MeshHeader->HasNormals)
        {
            Mesh->Normals = (vec3 *)(Buffer + MeshHeader->VerticesOffset + VerticesOffset);
            VerticesOffset += sizeof(vec3) * MeshHeader->VertexCount;
        }

        if (MeshHeader->HasTangents)
        {
            Mesh->Tangents = (vec3 *)(Buffer + MeshHeader->VerticesOffset + VerticesOffset);
            VerticesOffset += sizeof(vec3) * MeshHeader->VertexCount;
        }

        if (MeshHeader->HasBitangets)
        {
            Mesh->Bitangents = (vec3 *)(Buffer + MeshHeader->VerticesOffset + VerticesOffset);
            VerticesOffset += sizeof(vec3) * MeshHeader->VertexCount;
        }

        if (MeshHeader->HasTextureCoords)
        {
            Mesh->TextureCoords = (vec2 *)(Buffer + MeshHeader->VerticesOffset + VerticesOffset);
            VerticesOffset += sizeof(vec2) * MeshHeader->VertexCount;
        }

        if (MeshHeader->HasWeights)
        {
            Mesh->Weights = (vec4 *)(Buffer + MeshHeader->VerticesOffset + VerticesOffset);
            VerticesOffset += sizeof(vec4) * MeshHeader->VertexCount;
        }

        if (MeshHeader->HasJointIndices)
        {
            Mesh->JointIndices = (i32 *)(Buffer + MeshHeader->VerticesOffset + VerticesOffset);
            VerticesOffset += sizeof(i32) * 4 * MeshHeader->VertexCount;
        }

        Mesh->Indices = (u32 *)(Buffer + MeshHeader->IndicesOffset);

        NextMeshHeaderOffset += sizeof(model_asset_mesh_header) + VerticesOffset + MeshHeader->IndexCount * sizeof(u32);
    }

    // Materials
    model_asset_materials_header *MaterialsHeader = (model_asset_materials_header *)(Buffer + ModelHeader->MaterialsHeaderOffset);

    Result->MaterialCount = MaterialsHeader->MaterialCount;
    Result->Materials = PushArray(Arena, Result->MaterialCount, mesh_material);

    u64 NextMaterialHeaderOffset = 0;
    for (u32 MaterialIndex = 0; MaterialIndex < MaterialsHeader->MaterialCount; ++MaterialIndex)
    {
        model_asset_material_header *MaterialHeader = (model_asset_material_header *)(Buffer +
            MaterialsHeader->MaterialsOffset + NextMaterialHeaderOffset);
        mesh_material *Material = Result->Materials + MaterialIndex;
        Material->PropertyCount = MaterialHeader->PropertyCount;
        Material->Properties = PushArray(Arena, Material->PropertyCount, material_property);

        u64 NextMaterialPropertyHeaderOffset = 0;
        for (u32 MaterialPropertyIndex = 0; MaterialPropertyIndex < MaterialHeader->PropertyCount; ++MaterialPropertyIndex)
        {
            model_asset_material_property_header *MaterialPropertyHeader = (model_asset_material_property_header *)
                (Buffer + MaterialHeader->PropertiesOffset + NextMaterialPropertyHeaderOffset);

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
                    MaterialProperty->Bitmap.Pixels = (void *)(Buffer + MaterialPropertyHeader->BitmapOffset);

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
        (Buffer + ModelHeader->AnimationsHeaderOffset);
    Result->AnimationCount = AnimationsHeader->AnimationCount;
    Result->Animations = PushArray(Arena, Result->AnimationCount, animation_clip);

    u64 NextAnimationHeaderOffset = 0;
    for (u32 AnimationIndex = 0; AnimationIndex < AnimationsHeader->AnimationCount; ++AnimationIndex)
    {
        model_asset_animation_header *AnimationHeader = (model_asset_animation_header *)
            (Buffer +AnimationsHeader->AnimationsOffset + NextAnimationHeaderOffset);

        animation_clip *Animation = Result->Animations + AnimationIndex;

        CopyString(AnimationHeader->Name, Animation->Name);
        Animation->Duration = AnimationHeader->Duration;
        Animation->PoseSampleCount = AnimationHeader->PoseSampleCount;
        Animation->PoseSamples = PushArray(Arena, Animation->PoseSampleCount, animation_sample);

        u64 NextAnimationSampleHeaderOffset = 0;
        for (u32 AnimationPoseIndex = 0; AnimationPoseIndex < AnimationHeader->PoseSampleCount; ++AnimationPoseIndex)
        {
            model_asset_animation_sample_header *AnimationSampleHeader = (model_asset_animation_sample_header *)
                (Buffer + AnimationHeader->PoseSamplesOffset + NextAnimationSampleHeaderOffset);

            animation_sample *AnimationSample = Animation->PoseSamples + AnimationPoseIndex;

            AnimationSample->JointIndex = AnimationSampleHeader->JointIndex;
            AnimationSample->KeyFrameCount = AnimationSampleHeader->KeyFrameCount;
            AnimationSample->KeyFrames = (key_frame *)(Buffer + AnimationSampleHeader->KeyFramesOffset);

            NextAnimationSampleHeaderOffset += sizeof(model_asset_animation_sample_header) +
                AnimationSampleHeader->KeyFrameCount * sizeof(key_frame);
        }

        NextAnimationHeaderOffset += sizeof(model_asset_animation_header) + NextAnimationSampleHeaderOffset;
    }

    return Result;
}

internal font_asset *
LoadFontAsset(platform_api *Platform, char *FileName, memory_arena *Arena)
{
    font_asset *Result = PushType(Arena, font_asset);

    read_file_result AssetFile = Platform->ReadFile(FileName, Arena, false);
    u8 *Buffer = (u8 *) AssetFile.Contents;

    asset_header *Header = (asset_header *) Buffer;

    Assert(Header->Type == AssetType_Font);

    font_asset_header *FontHeader = (font_asset_header *) (Buffer + Header->DataOffset);

    Result->TextureAtlas.Width = FontHeader->TextureAtlasWidth;
    Result->TextureAtlas.Height = FontHeader->TextureAtlasHeight;
    Result->TextureAtlas.Channels = FontHeader->TextureAtlasChannels;
    Result->TextureAtlas.Pixels = (u8 *) (Buffer + FontHeader->TextureAtlasOffset);

    Result->VerticalAdvance = FontHeader->VerticalAdvance;
    Result->Ascent = FontHeader->Ascent;
    Result->Descent = FontHeader->Descent;

    Result->CodepointsRangeCount = FontHeader->CodepointsRangeCount;
    Result->CodepointsRanges = (codepoints_range *) (Buffer + FontHeader->CodepointsRangesOffset);

    Result->HorizontalAdvanceTableCount = FontHeader->HorizontalAdvanceTableCount;
    Result->HorizontalAdvanceTable = (f32 *) (Buffer + FontHeader->HorizontalAdvanceTableOffset);

    Result->GlyphCount = FontHeader->GlyphCount;
    Result->Glyphs = (glyph *) (Buffer + FontHeader->GlyphsOffset);

    return Result;
}
