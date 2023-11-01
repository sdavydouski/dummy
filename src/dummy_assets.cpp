#include "dummy.h"

dummy_internal u32
ReadAnimationGraph(animation_graph_asset *GraphAsset, u64 Offset, u8 *Buffer, memory_arena *Arena)
{
    model_asset_animation_graph_header *AnimationGraphHeader = GET_DATA_AT(Buffer, Offset, model_asset_animation_graph_header);
    CopyString(AnimationGraphHeader->Name, GraphAsset->Name);
    CopyString(AnimationGraphHeader->Entry, GraphAsset->Entry);
    CopyString(AnimationGraphHeader->Animator, GraphAsset->Animator);
    GraphAsset->NodeCount = AnimationGraphHeader->NodeCount;
    GraphAsset->Nodes = PushArray(Arena, GraphAsset->NodeCount, animation_node_asset);

    u32 TotalPrevNodeSize = 0;
    for (u32 NodeIndex = 0; NodeIndex < GraphAsset->NodeCount; ++NodeIndex)
    {
        model_asset_animation_node_header *NodeHeader = GET_DATA_AT(Buffer, AnimationGraphHeader->NodesOffset + TotalPrevNodeSize, model_asset_animation_node_header);
        animation_node_asset *NodeAsset = GraphAsset->Nodes + NodeIndex;

        NodeAsset->Type = NodeHeader->Type;
        CopyString(NodeHeader->Name, NodeAsset->Name);

        NodeAsset->TransitionCount = NodeHeader->TransitionCount;
        NodeAsset->Transitions = GET_DATA_AT(Buffer, NodeHeader->TransitionsOffset, animation_transition_asset);

        NodeAsset->AdditiveAnimationCount = NodeHeader->AdditiveAnimationCount;
        NodeAsset->AdditiveAnimations = GET_DATA_AT(Buffer, NodeHeader->AdditiveAnimationsOffset, additive_animation_asset);

        switch (NodeAsset->Type)
        {
            case AnimationNodeType_Clip:
            {
                model_asset_animation_state_header *AnimationStateHeader = GET_DATA_AT(Buffer, NodeHeader->Offset, model_asset_animation_state_header);

                NodeAsset->Animation = PushType(Arena, animation_state_asset);
                CopyString(AnimationStateHeader->AnimationClipName, NodeAsset->Animation->AnimationClipName);
                NodeAsset->Animation->IsLooping = AnimationStateHeader->IsLooping;
                NodeAsset->Animation->EnableRootMotion = AnimationStateHeader->EnableRootMotion;

                TotalPrevNodeSize += sizeof(model_asset_animation_state_header);
                break;
            }
            case AnimationNodeType_BlendSpace:
            {
                model_asset_blend_space_1d_header *BlendSpaceHeader = GET_DATA_AT(Buffer, NodeHeader->Offset, model_asset_blend_space_1d_header);

                NodeAsset->Blendspace = PushType(Arena, blend_space_1d_asset);
                NodeAsset->Blendspace->ValueCount = BlendSpaceHeader->ValueCount;
                NodeAsset->Blendspace->Values = GET_DATA_AT(Buffer, BlendSpaceHeader->ValuesOffset, blend_space_1d_value_asset);

                TotalPrevNodeSize += sizeof(model_asset_blend_space_1d_header) + BlendSpaceHeader->ValueCount * sizeof(blend_space_1d_value_asset);
                break;
            }
            case AnimationNodeType_Reference:
            {
                model_asset_animation_reference_header *AnimationAdditiveHeader = GET_DATA_AT(Buffer, NodeHeader->Offset, model_asset_animation_reference_header);

                NodeAsset->Reference = PushType(Arena, animation_reference_asset);
                CopyString(AnimationAdditiveHeader->NodeName, NodeAsset->Reference->NodeName);

                TotalPrevNodeSize += sizeof(model_asset_animation_reference_header);

                break;
            }
            case AnimationNodeType_Graph:
            {
                NodeAsset->Graph = PushType(Arena, animation_graph_asset);

                u32 NodeSize = ReadAnimationGraph(NodeAsset->Graph, NodeHeader->Offset, Buffer, Arena);

                TotalPrevNodeSize += NodeSize + sizeof(model_asset_animation_graph_header);
                break;
            }
        }

        TotalPrevNodeSize += sizeof(model_asset_animation_node_header) + NodeHeader->TransitionCount * sizeof(animation_transition_asset) + NodeHeader->AdditiveAnimationCount * sizeof(additive_animation_asset);
    }

    return TotalPrevNodeSize;
}

dummy_internal model_asset *
LoadModelAsset(platform_api *Platform, char *FileName, memory_arena *Arena)
{
    model_asset *Result = PushType(Arena, model_asset);

    read_file_result AssetFile = Platform->ReadFile(FileName, Arena, ReadBinary());
    u8 *Buffer = (u8 *)AssetFile.Contents;

    asset_header *Header = GET_DATA_AT(Buffer, 0, asset_header);

    Assert(Header->Type == AssetType_Model);

    model_asset_header *ModelHeader = GET_DATA_AT(Buffer, Header->DataOffset, model_asset_header);

    Result->Bounds = ModelHeader->Bounds;
    Result->BoundsOBB = ModelHeader->BoundsOBB;

    // Skeleton
    model_asset_skeleton_header *SkeletonHeader = GET_DATA_AT(Buffer, ModelHeader->SkeletonHeaderOffset, model_asset_skeleton_header);
    Result->Skeleton.JointCount = SkeletonHeader->JointCount;
#if 0
    Result->Skeleton.Joints = (joint *)(Buffer + SkeletonHeader->JointsOffset);
#else
    joint *Joints = GET_DATA_AT(Buffer, SkeletonHeader->JointsOffset, joint);

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
    Result->BindPose.LocalJointPoses = GET_DATA_AT(Buffer, SkeletonPoseHeader->LocalJointPosesOffset, transform);
#if 0
    Result->BindPose.GlobalJointPoses = (mat4 *)(Buffer + SkeletonPoseHeader->GlobalJointPosesOffset);
#else
    Result->BindPose.GlobalJointPoses = PushArray(Arena, Result->Skeleton.JointCount, mat4, Align(16));
#endif

    // Animation Graph
    ReadAnimationGraph(&Result->AnimationGraph, ModelHeader->AnimationGraphHeaderOffset, Buffer, Arena);

    // Meshes
    model_asset_meshes_header *MeshesHeader = GET_DATA_AT(Buffer, ModelHeader->MeshesHeaderOffset, model_asset_meshes_header);
    Result->MeshCount = MeshesHeader->MeshCount;
    Result->Meshes = PushArray(Arena, Result->MeshCount, mesh);

    u64 NextMeshHeaderOffset = 0;
    for (u32 MeshIndex = 0; MeshIndex < MeshesHeader->MeshCount; ++MeshIndex)
    {
        model_asset_mesh_header *MeshHeader = GET_DATA_AT(Buffer, MeshesHeader->MeshesOffset + NextMeshHeaderOffset, model_asset_mesh_header);
        
        mesh *Mesh = Result->Meshes + MeshIndex;

        Mesh->MaterialIndex = MeshHeader->MaterialIndex;
        Mesh->VertexCount = MeshHeader->VertexCount;
        Mesh->IndexCount = MeshHeader->IndexCount;

        u32 VerticesOffset = 0;

        if (MeshHeader->HasPositions)
        {
            Mesh->Positions = GET_DATA_AT(Buffer, MeshHeader->VerticesOffset + VerticesOffset, vec3);
            VerticesOffset += sizeof(vec3) * MeshHeader->VertexCount;
        }

        if (MeshHeader->HasNormals)
        {
            Mesh->Normals = GET_DATA_AT(Buffer, MeshHeader->VerticesOffset + VerticesOffset, vec3);
            VerticesOffset += sizeof(vec3) * MeshHeader->VertexCount;
        }

        if (MeshHeader->HasTangents)
        {
            Mesh->Tangents = GET_DATA_AT(Buffer, MeshHeader->VerticesOffset + VerticesOffset, vec3);
            VerticesOffset += sizeof(vec3) * MeshHeader->VertexCount;
        }

        if (MeshHeader->HasBitangets)
        {
            Mesh->Bitangents = GET_DATA_AT(Buffer, MeshHeader->VerticesOffset + VerticesOffset, vec3);
            VerticesOffset += sizeof(vec3) * MeshHeader->VertexCount;
        }

        if (MeshHeader->HasTextureCoords)
        {
            Mesh->TextureCoords = GET_DATA_AT(Buffer, MeshHeader->VerticesOffset + VerticesOffset, vec2);
            VerticesOffset += sizeof(vec2) * MeshHeader->VertexCount;
        }

        if (MeshHeader->HasWeights)
        {
            Mesh->Weights = GET_DATA_AT(Buffer, MeshHeader->VerticesOffset + VerticesOffset, vec4);
            VerticesOffset += sizeof(vec4) * MeshHeader->VertexCount;
        }

        if (MeshHeader->HasJointIndices)
        {
            Mesh->JointIndices = GET_DATA_AT(Buffer, MeshHeader->VerticesOffset + VerticesOffset, ivec4);
            VerticesOffset += sizeof(ivec4) * MeshHeader->VertexCount;
        }

        Mesh->Indices = GET_DATA_AT(Buffer, MeshHeader->IndicesOffset, u32);

        NextMeshHeaderOffset += sizeof(model_asset_mesh_header) + VerticesOffset + MeshHeader->IndexCount * sizeof(u32);
    }

    // Materials
    model_asset_materials_header *MaterialsHeader = GET_DATA_AT(Buffer, ModelHeader->MaterialsHeaderOffset, model_asset_materials_header);

    Result->MaterialCount = MaterialsHeader->MaterialCount;
    Result->Materials = PushArray(Arena, Result->MaterialCount, mesh_material, Align(16));

    u64 NextMaterialHeaderOffset = 0;
    for (u32 MaterialIndex = 0; MaterialIndex < MaterialsHeader->MaterialCount; ++MaterialIndex)
    {
        model_asset_material_header *MaterialHeader = GET_DATA_AT(Buffer, MaterialsHeader->MaterialsOffset + NextMaterialHeaderOffset, model_asset_material_header);
        mesh_material *Material = Result->Materials + MaterialIndex;
        Material->ShadingMode = MaterialHeader->ShadingMode;
        Material->PropertyCount = MaterialHeader->PropertyCount;
        Material->Properties = PushArray(Arena, Material->PropertyCount, material_property);

        u64 NextMaterialPropertyHeaderOffset = 0;
        for (u32 MaterialPropertyIndex = 0; MaterialPropertyIndex < MaterialHeader->PropertyCount; ++MaterialPropertyIndex)
        {
            model_asset_material_property_header *MaterialPropertyHeader = GET_DATA_AT(Buffer, MaterialHeader->PropertiesOffset + NextMaterialPropertyHeaderOffset, model_asset_material_property_header);

            material_property *MaterialProperty = Material->Properties + MaterialPropertyIndex;
            MaterialProperty->Type = MaterialPropertyHeader->Type;

            switch (MaterialProperty->Type)
            {
                case MaterialProperty_Float_Shininess:
                case MaterialProperty_Float_Metalness:
                case MaterialProperty_Float_Roughness:
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
                case MaterialProperty_Texture_Albedo:
                case MaterialProperty_Texture_Metalness:
                case MaterialProperty_Texture_Roughness:
                case MaterialProperty_Texture_Normal:
                {
                    MaterialProperty->Bitmap = MaterialPropertyHeader->Bitmap;
                    MaterialProperty->Bitmap.Pixels = GET_DATA_AT(Buffer, MaterialPropertyHeader->BitmapOffset, void);

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
    model_asset_animations_header *AnimationsHeader = GET_DATA_AT(Buffer, ModelHeader->AnimationsHeaderOffset, model_asset_animations_header);
    Result->AnimationCount = AnimationsHeader->AnimationCount;
    Result->Animations = PushArray(Arena, Result->AnimationCount, animation_clip);

    u64 NextAnimationHeaderOffset = 0;
    for (u32 AnimationIndex = 0; AnimationIndex < AnimationsHeader->AnimationCount; ++AnimationIndex)
    {
        model_asset_animation_header *AnimationHeader = GET_DATA_AT(Buffer, AnimationsHeader->AnimationsOffset + NextAnimationHeaderOffset, model_asset_animation_header);

        animation_clip *Animation = Result->Animations + AnimationIndex;

        CopyString(AnimationHeader->Name, Animation->Name);
        Animation->Duration = AnimationHeader->Duration;
        Animation->PoseSampleCount = AnimationHeader->PoseSampleCount;
        Animation->PoseSamples = PushArray(Arena, Animation->PoseSampleCount, animation_sample);
        Animation->EventCount = AnimationHeader->EventCount;
        Animation->Events = GET_DATA_AT(Buffer, AnimationHeader->EventsOffset, animation_event);

        u64 NextAnimationSampleHeaderOffset = 0;
        for (u32 AnimationPoseIndex = 0; AnimationPoseIndex < AnimationHeader->PoseSampleCount; ++AnimationPoseIndex)
        {
            model_asset_animation_sample_header *AnimationSampleHeader = GET_DATA_AT(Buffer, AnimationHeader->PoseSamplesOffset + NextAnimationSampleHeaderOffset, model_asset_animation_sample_header);

            animation_sample *AnimationSample = Animation->PoseSamples + AnimationPoseIndex;

            AnimationSample->JointIndex = AnimationSampleHeader->JointIndex;
            AnimationSample->KeyFrameCount = AnimationSampleHeader->KeyFrameCount;
            AnimationSample->KeyFrames = GET_DATA_AT(Buffer, AnimationSampleHeader->KeyFramesOffset, key_frame);

            NextAnimationSampleHeaderOffset += sizeof(model_asset_animation_sample_header) + AnimationSampleHeader->KeyFrameCount * sizeof(key_frame);
        }

        NextAnimationHeaderOffset += sizeof(model_asset_animation_header) + NextAnimationSampleHeaderOffset + AnimationHeader->EventCount * sizeof(animation_event);
    }

    return Result;
}

dummy_internal font_asset *
LoadFontAsset(platform_api *Platform, char *FileName, memory_arena *Arena)
{
    font_asset *Result = PushType(Arena, font_asset);

    read_file_result AssetFile = Platform->ReadFile(FileName, Arena, ReadBinary());
    u8 *Buffer = (u8 *) AssetFile.Contents;

    asset_header *Header = GET_DATA_AT(Buffer, 0, asset_header);

    Assert(Header->Type == AssetType_Font);

    font_asset_header *FontHeader = GET_DATA_AT(Buffer, Header->DataOffset, font_asset_header);

    Result->TextureAtlas.Width = FontHeader->TextureAtlasWidth;
    Result->TextureAtlas.Height = FontHeader->TextureAtlasHeight;
    Result->TextureAtlas.Channels = FontHeader->TextureAtlasChannels;
    Result->TextureAtlas.Pixels = GET_DATA_AT(Buffer, FontHeader->TextureAtlasOffset, u8);

    Result->VerticalAdvance = FontHeader->VerticalAdvance;
    Result->Ascent = FontHeader->Ascent;
    Result->Descent = FontHeader->Descent;

    Result->CodepointsRangeCount = FontHeader->CodepointsRangeCount;
    Result->CodepointsRanges = GET_DATA_AT(Buffer, FontHeader->CodepointsRangesOffset, codepoints_range);

    Result->HorizontalAdvanceTableCount = FontHeader->HorizontalAdvanceTableCount;
    Result->HorizontalAdvanceTable = GET_DATA_AT(Buffer, FontHeader->HorizontalAdvanceTableOffset, f32);

    Result->GlyphCount = FontHeader->GlyphCount;
    Result->Glyphs = GET_DATA_AT(Buffer, FontHeader->GlyphsOffset, glyph);

    return Result;
}

dummy_internal audio_clip_asset *
LoadAudioClipAsset(platform_api *Platform, char *FileName, memory_arena *Arena)
{
    audio_clip_asset *Result = PushType(Arena, audio_clip_asset);

    read_file_result AssetFile = Platform->ReadFile(FileName, Arena, ReadBinary());
    u8 *Buffer = (u8 *) AssetFile.Contents;

    asset_header *Header = GET_DATA_AT(Buffer, 0, asset_header);

    Assert(Header->Type == AssetType_AudioClip);

    audio_clip_asset_header *AudioHeader = GET_DATA_AT(Buffer, Header->DataOffset, audio_clip_asset_header);

    Result->Format = AudioHeader->Format;
    Result->Channels = AudioHeader->Channels;
    Result->SamplesPerSecond = AudioHeader->SamplesPerSecond;
    Result->BitsPerSample = AudioHeader->BitsPerSample;
    Result->AudioBytes = AudioHeader->AudioBytes;
    Result->AudioData = GET_DATA_AT(Buffer, AudioHeader->AudioDataOffset, u8);

    return Result;
}

dummy_internal texture_asset *
LoadTextureAsset(platform_api *Platform, char *FileName, memory_arena *Arena)
{
    texture_asset *Result = PushType(Arena, texture_asset);

    read_file_result AssetFile = Platform->ReadFile(FileName, Arena, ReadBinary());
    u8 *Buffer = (u8 *)AssetFile.Contents;

    asset_header *Header = GET_DATA_AT(Buffer, 0, asset_header);

    Assert(Header->Type == AssetType_Texture);

    texture_asset_header *TextureHeader = GET_DATA_AT(Buffer, Header->DataOffset, texture_asset_header);

    Result->Bitmap.Width = TextureHeader->Width;
    Result->Bitmap.Height = TextureHeader->Height;
    Result->Bitmap.Channels = TextureHeader->Channels;
    Result->Bitmap.IsHDR = TextureHeader->IsHDR;

    if (Result->Bitmap.IsHDR)
    {
        Result->Bitmap.Pixels = GET_DATA_AT(Buffer, TextureHeader->PixelsOffset, f32);
    }
    else
    {
        Result->Bitmap.Pixels = GET_DATA_AT(Buffer, TextureHeader->PixelsOffset, u8);
    }

    return Result;
}

dummy_internal model *
GetModelAsset(game_assets *Assets, const char *Name)
{
    Assert(Assets->State != GameAssetsState_Unloaded)

    model *Result = HashTableLookup(&Assets->Models, (char *) Name);
    return Result;
}

dummy_internal font *
GetFontAsset(game_assets *Assets, const char *Name)
{
    font *Result = HashTableLookup(&Assets->Fonts, (char *) Name);
    return Result;
}

dummy_internal audio_clip *
GetAudioClipAsset(game_assets *Assets, const char *Name)
{
    audio_clip *Result = HashTableLookup(&Assets->AudioClips, (char *) Name);
    return Result;
}

dummy_internal texture *
GetTextureAsset(game_assets *Assets, const char *Name)
{
    texture *Result = HashTableLookup(&Assets->Textures, (char *) Name);
    return Result;
}
