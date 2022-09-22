// For testing
internal u32
ReadAnimationGraph(animation_graph_asset *GraphAsset, u64 Offset, u8 *Buffer)
{
    model_asset_animation_graph_header *AnimationGraphHeader = (model_asset_animation_graph_header *) (Buffer + Offset);
    CopyString(AnimationGraphHeader->Name, GraphAsset->Name);
    CopyString(AnimationGraphHeader->Entry, GraphAsset->Entry);
    GraphAsset->NodeCount = AnimationGraphHeader->NodeCount;
    GraphAsset->Nodes = AllocateMemory<animation_node_asset>(GraphAsset->NodeCount);

    u32 TotalPrevNodeSize = 0;
    for (u32 NodeIndex = 0; NodeIndex < GraphAsset->NodeCount; ++NodeIndex)
    {
        model_asset_animation_node_header *NodeHeader = (model_asset_animation_node_header *) (Buffer + AnimationGraphHeader->NodesOffset + TotalPrevNodeSize);
        animation_node_asset *NodeAsset = GraphAsset->Nodes + NodeIndex;

        NodeAsset->Type = NodeHeader->Type;
        CopyString(NodeHeader->Name, NodeAsset->Name);
        NodeAsset->TransitionCount = NodeHeader->TransitionCount;
        NodeAsset->Transitions = (animation_transition_asset *) (Buffer + NodeHeader->TransitionsOffset);

        switch (NodeAsset->Type)
        {
            case AnimationNodeType_Clip:
            {
                model_asset_animation_state_header *AnimationStateHeader = (model_asset_animation_state_header *) (Buffer + NodeHeader->Offset);

                NodeAsset->Animation = AllocateMemory<animation_state_asset>();
                CopyString(AnimationStateHeader->AnimationClipName, NodeAsset->Animation->AnimationClipName);
                NodeAsset->Animation->IsLooping = AnimationStateHeader->IsLooping;

                TotalPrevNodeSize += sizeof(model_asset_animation_state_header);
                break;
            }
            case AnimationNodeType_BlendSpace:
            {
                model_asset_blend_space_1d_header *BlendSpaceHeader = (model_asset_blend_space_1d_header *) (Buffer + NodeHeader->Offset);

                NodeAsset->Blendspace = AllocateMemory<blend_space_1d_asset>();
                NodeAsset->Blendspace->ValueCount = BlendSpaceHeader->ValueCount;
                NodeAsset->Blendspace->Values = (blend_space_1d_value_asset *) (Buffer + BlendSpaceHeader->ValuesOffset);

                TotalPrevNodeSize += sizeof(model_asset_blend_space_1d_header) + BlendSpaceHeader->ValueCount * sizeof(blend_space_1d_value_asset);
                break;
            }
            case AnimationNodeType_Graph:
            {
                NodeAsset->Graph = AllocateMemory<animation_graph_asset>();

                u32 NodeSize = ReadAnimationGraph(NodeAsset->Graph, NodeHeader->Offset, Buffer);

                TotalPrevNodeSize += NodeSize + sizeof(model_asset_animation_graph_header);
                break;
            }
        }

        TotalPrevNodeSize += sizeof(model_asset_animation_node_header) + NodeHeader->TransitionCount * sizeof(animation_transition_asset);
    }

    return TotalPrevNodeSize;
}

// For testing
internal void
ReadModelAsset(const char *FilePath, model_asset *Asset, model_asset *OriginalAsset)
{
    FILE *AssetFile = fopen(FilePath, "rb");

    u32 FileSize = GetFileSize(AssetFile);

    u8 *Buffer = AllocateMemory<u8>(FileSize);

    fread(Buffer, FileSize, 1, AssetFile);

    asset_header *Header = (asset_header *) Buffer;

    model_asset_header *ModelHeader = (model_asset_header *) (Buffer + Header->DataOffset);

    // Read skeleton
    model_asset_skeleton_header *SkeletonHeader = (model_asset_skeleton_header *) (Buffer + ModelHeader->SkeletonHeaderOffset);
    skeleton Skeleton = {};
    Skeleton.JointCount = SkeletonHeader->JointCount;
    Skeleton.Joints = (joint *) ((u8 *) Buffer + SkeletonHeader->JointsOffset);

    // Read skeleton bind pose
    model_asset_skeleton_pose_header *SkeletonPoseHeader = (model_asset_skeleton_pose_header *) (Buffer + ModelHeader->SkeletonPoseHeaderOffset);
    skeleton_pose BindPose = {};
    BindPose.LocalJointPoses = (joint_pose *) ((u8 *) Buffer + SkeletonPoseHeader->LocalJointPosesOffset);
    BindPose.GlobalJointPoses = (mat4 *) ((u8 *) Buffer + SkeletonPoseHeader->GlobalJointPosesOffset);

    // Read animation graph
    animation_graph_asset GraphAsset = {};
    ReadAnimationGraph(&GraphAsset, ModelHeader->AnimationGraphHeaderOffset, Buffer);

    // Read meshes
    model_asset_meshes_header *MeshesHeader = (model_asset_meshes_header *) (Buffer + ModelHeader->MeshesHeaderOffset);

    u32 NextMeshHeaderOffset = 0;

    for (u32 MeshIndex = 0; MeshIndex < MeshesHeader->MeshCount; ++MeshIndex)
    {
        model_asset_mesh_header *MeshHeader = (model_asset_mesh_header *) (Buffer +
            MeshesHeader->MeshesOffset + NextMeshHeaderOffset);
        mesh Mesh = {};

        Mesh.MaterialIndex = MeshHeader->MaterialIndex;
        Mesh.VertexCount = MeshHeader->VertexCount;
        Mesh.IndexCount = MeshHeader->IndexCount;

        u32 VerticesOffset = 0;

        if (MeshHeader->HasPositions)
        {
            Mesh.Positions = (vec3 *) (Buffer + MeshHeader->VerticesOffset + VerticesOffset);
            VerticesOffset += sizeof(vec3) * MeshHeader->VertexCount;
        }

        if (MeshHeader->HasNormals)
        {
            Mesh.Normals = (vec3 *) (Buffer + MeshHeader->VerticesOffset + VerticesOffset);
            VerticesOffset += sizeof(vec3) * MeshHeader->VertexCount;
        }

        if (MeshHeader->HasTangents)
        {
            Mesh.Tangents = (vec3 *) (Buffer + MeshHeader->VerticesOffset + VerticesOffset);
            VerticesOffset += sizeof(vec3) * MeshHeader->VertexCount;
        }

        if (MeshHeader->HasBitangets)
        {
            Mesh.Bitangents = (vec3 *) (Buffer + MeshHeader->VerticesOffset + VerticesOffset);
            VerticesOffset += sizeof(vec3) * MeshHeader->VertexCount;
        }

        if (MeshHeader->HasTextureCoords)
        {
            Mesh.TextureCoords = (vec2 *) (Buffer + MeshHeader->VerticesOffset + VerticesOffset);
            VerticesOffset += sizeof(vec2) * MeshHeader->VertexCount;
        }

        if (MeshHeader->HasWeights)
        {
            Mesh.Weights = (vec4 *) (Buffer + MeshHeader->VerticesOffset + VerticesOffset);
            VerticesOffset += sizeof(vec4) * MeshHeader->VertexCount;
        }

        if (MeshHeader->HasJointIndices)
        {
            Mesh.JointIndices = (i32 *) (Buffer + MeshHeader->VerticesOffset + VerticesOffset);
            VerticesOffset += sizeof(i32) * 4 * MeshHeader->VertexCount;
        }

        Mesh.Indices = (u32 *) (Buffer + MeshHeader->IndicesOffset);

        NextMeshHeaderOffset += sizeof(model_asset_mesh_header) + VerticesOffset + MeshHeader->IndexCount * sizeof(u32);
    }

    // Read materials
    model_asset_materials_header *MaterialsHeader = (model_asset_materials_header *) (Buffer + ModelHeader->MaterialsHeaderOffset);

    u64 NextMaterialHeaderOffset = 0;
    for (u32 MaterialIndex = 0; MaterialIndex < MaterialsHeader->MaterialCount; ++MaterialIndex)
    {
        model_asset_material_header *MaterialHeader = (model_asset_material_header *) (Buffer +
            MaterialsHeader->MaterialsOffset + NextMaterialHeaderOffset);
        mesh_material Material = {};
        Material.PropertyCount = MaterialHeader->PropertyCount;
        Material.Properties = (material_property *) (Buffer + MaterialHeader->PropertiesOffset);

        u64 NextMaterialPropertyHeaderOffset = 0;
        for (u32 MaterialPropertyIndex = 0; MaterialPropertyIndex < MaterialHeader->PropertyCount; ++MaterialPropertyIndex)
        {
            model_asset_material_property_header *MaterialPropertyHeader = (model_asset_material_property_header *)
                (Buffer + MaterialHeader->PropertiesOffset + NextMaterialPropertyHeaderOffset);

            material_property *MaterialProperty = Material.Properties + MaterialPropertyIndex;

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
                {
                    MaterialProperty->Bitmap = MaterialPropertyHeader->Bitmap;
                    MaterialProperty->Bitmap.Pixels = (void *) (Buffer + MaterialPropertyHeader->BitmapOffset);

#if 0
                    char FileName[64];
                    FormatString(FileName, "Diffuse - %d - %d.bmp", MaterialIndex, MaterialPropertyIndex);

                    stbi_write_bmp(FileName, MaterialProperty->Bitmap.Width, MaterialProperty->Bitmap.Height, MaterialProperty->Bitmap.Channels, MaterialProperty->Bitmap.Pixels);
#endif

                    u32 BitmapSize = MaterialProperty->Bitmap.Width * MaterialProperty->Bitmap.Height * MaterialProperty->Bitmap.Channels;

                    NextMaterialPropertyHeaderOffset += sizeof(model_asset_material_property_header) + BitmapSize;

                    break;
                }
                case MaterialProperty_Texture_Specular:
                {
                    MaterialProperty->Bitmap = MaterialPropertyHeader->Bitmap;
                    MaterialProperty->Bitmap.Pixels = (void *) (Buffer + MaterialPropertyHeader->BitmapOffset);

#if 0
                    char FileName[64];
                    FormatString(FileName, "Specular - %d - %d.bmp", MaterialIndex, MaterialPropertyIndex);

                    stbi_write_bmp(FileName, MaterialProperty->Bitmap.Width, MaterialProperty->Bitmap.Height, MaterialProperty->Bitmap.Channels, MaterialProperty->Bitmap.Pixels);
#endif

                    u32 BitmapSize = MaterialProperty->Bitmap.Width * MaterialProperty->Bitmap.Height * MaterialProperty->Bitmap.Channels;

                    NextMaterialPropertyHeaderOffset += sizeof(model_asset_material_property_header) + BitmapSize;

                    break;
                }
                case MaterialProperty_Texture_Shininess:
                {
                    MaterialProperty->Bitmap = MaterialPropertyHeader->Bitmap;
                    MaterialProperty->Bitmap.Pixels = (void *) (Buffer + MaterialPropertyHeader->BitmapOffset);

#if 0
                    char FileName[64];
                    FormatString(FileName, "Shininess - %d - %d.bmp", MaterialIndex, MaterialPropertyIndex);

                    stbi_write_bmp(FileName, MaterialProperty->Bitmap.Width, MaterialProperty->Bitmap.Height, MaterialProperty->Bitmap.Channels, MaterialProperty->Bitmap.Pixels);
#endif

                    u32 BitmapSize = MaterialProperty->Bitmap.Width * MaterialProperty->Bitmap.Height * MaterialProperty->Bitmap.Channels;

                    NextMaterialPropertyHeaderOffset += sizeof(model_asset_material_property_header) + BitmapSize;

                    break;
                }
                case MaterialProperty_Texture_Normal:
                {
                    MaterialProperty->Bitmap = MaterialPropertyHeader->Bitmap;
                    MaterialProperty->Bitmap.Pixels = (void *) (Buffer + MaterialPropertyHeader->BitmapOffset);

#if 0
                    char FileName[64];
                    FormatString(FileName, "Normal - %d - %d.bmp", MaterialIndex, MaterialPropertyIndex);

                    stbi_write_bmp(FileName, MaterialProperty->Bitmap.Width, MaterialProperty->Bitmap.Height, MaterialProperty->Bitmap.Channels, MaterialProperty->Bitmap.Pixels);
#endif

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

    // Read animation clips
    model_asset_animations_header *AnimationsHeader = (model_asset_animations_header *) (Buffer + ModelHeader->AnimationsHeaderOffset);

    u64 NextAnimationHeaderOffset = 0;

    for (u32 AnimationIndex = 0; AnimationIndex < AnimationsHeader->AnimationCount; ++AnimationIndex)
    {
        model_asset_animation_header *AnimationHeader = (model_asset_animation_header *) (Buffer +
            AnimationsHeader->AnimationsOffset + NextAnimationHeaderOffset);
        animation_clip Animation = {};
        CopyString(AnimationHeader->Name, Animation.Name);
        Animation.Duration = AnimationHeader->Duration;
        Animation.PoseSampleCount = AnimationHeader->PoseSampleCount;
        Animation.PoseSamples = (animation_sample *) (Buffer + AnimationHeader->PoseSamplesOffset);

        u64 NextAnimationSampleHeaderOffset = 0;

        for (u32 AnimationPoseIndex = 0; AnimationPoseIndex < AnimationHeader->PoseSampleCount; ++AnimationPoseIndex)
        {
            model_asset_animation_sample_header *AnimationSampleHeader = (model_asset_animation_sample_header *)
                (Buffer + AnimationHeader->PoseSamplesOffset + NextAnimationSampleHeaderOffset);

            animation_sample *AnimationSample = Animation.PoseSamples + AnimationPoseIndex;

            AnimationSample->KeyFrameCount = AnimationSampleHeader->KeyFrameCount;
            AnimationSample->KeyFrames = (key_frame *) (Buffer + AnimationSampleHeader->KeyFramesOffset);

            NextAnimationSampleHeaderOffset += sizeof(model_asset_animation_sample_header) +
                AnimationSampleHeader->KeyFrameCount * sizeof(key_frame);
        }

        NextAnimationHeaderOffset += sizeof(model_asset_animation_header) + NextAnimationSampleHeaderOffset;
    }

    fclose(AssetFile);
}

internal u32
WriteAnimationGraph(animation_graph_asset *AnimationGraph, u64 Offset, FILE *AssetFile)
{
    model_asset_animation_graph_header AnimationGraphHeader = {};
    CopyString(AnimationGraph->Name, AnimationGraphHeader.Name);
    CopyString(AnimationGraph->Entry, AnimationGraphHeader.Entry);
    AnimationGraphHeader.NodeCount = AnimationGraph->NodeCount;
    AnimationGraphHeader.NodesOffset = Offset + sizeof(model_asset_animation_graph_header);

    fwrite(&AnimationGraphHeader, sizeof(model_asset_animation_graph_header), 1, AssetFile);

    u32 TotalPrevNodeSize = 0;
    for (u32 NodeIndex = 0; NodeIndex < AnimationGraph->NodeCount; ++NodeIndex)
    {
        animation_node_asset *Node = AnimationGraph->Nodes + NodeIndex;

        model_asset_animation_node_header NodeHeader = {};
        NodeHeader.Type = Node->Type;
        CopyString(Node->Name, NodeHeader.Name);
        NodeHeader.TransitionCount = Node->TransitionCount;
        NodeHeader.TransitionsOffset = AnimationGraphHeader.NodesOffset + sizeof(model_asset_animation_node_header) + TotalPrevNodeSize;
        NodeHeader.Offset = NodeHeader.TransitionsOffset + NodeHeader.TransitionCount * sizeof(animation_transition_asset);

        fwrite(&NodeHeader, sizeof(model_asset_animation_node_header), 1, AssetFile);
        fwrite(Node->Transitions, sizeof(animation_transition_asset), Node->TransitionCount, AssetFile);

        switch (NodeHeader.Type)
        {
            case AnimationNodeType_Clip:
            {
                model_asset_animation_state_header AnimationStateHeader = {};
                CopyString(Node->Animation->AnimationClipName, AnimationStateHeader.AnimationClipName);
                AnimationStateHeader.IsLooping = Node->Animation->IsLooping;

                fwrite(&AnimationStateHeader, sizeof(model_asset_animation_state_header), 1, AssetFile);

                TotalPrevNodeSize += sizeof(model_asset_animation_state_header);
                break;
            }
            case AnimationNodeType_BlendSpace:
            {
                model_asset_blend_space_1d_header BlendSpaceHeader = {};
                BlendSpaceHeader.ValueCount = Node->Blendspace->ValueCount;
                BlendSpaceHeader.ValuesOffset = NodeHeader.Offset + sizeof(model_asset_blend_space_1d_header);

                fwrite(&BlendSpaceHeader, sizeof(model_asset_blend_space_1d_header), 1, AssetFile);
                fwrite(Node->Blendspace->Values, sizeof(blend_space_1d_value_asset), Node->Blendspace->ValueCount, AssetFile);

                TotalPrevNodeSize += sizeof(model_asset_blend_space_1d_header) + BlendSpaceHeader.ValueCount * sizeof(blend_space_1d_value_asset);
                break;
            }
            case AnimationNodeType_Graph:
            {
                u32 NodeSize = WriteAnimationGraph(Node->Graph, NodeHeader.Offset, AssetFile);

                TotalPrevNodeSize += NodeSize + sizeof(model_asset_animation_graph_header);
                break;
            }
        }

        TotalPrevNodeSize += sizeof(model_asset_animation_node_header) + NodeHeader.TransitionCount * sizeof(animation_transition_asset);
    }

    return TotalPrevNodeSize;
}

internal void
WriteMeshes(model_asset *Asset, u64 Offset, FILE *AssetFile)
{
    model_asset_meshes_header MeshesHeader = {};
    MeshesHeader.MeshCount = Asset->MeshCount;
    MeshesHeader.MeshesOffset = Offset + sizeof(model_asset_meshes_header);

    fwrite(&MeshesHeader, sizeof(model_asset_meshes_header), 1, AssetFile);

    u32 TotalPrevMeshSize = 0;
    for (u32 MeshIndex = 0; MeshIndex < Asset->MeshCount; ++MeshIndex)
    {
        mesh *Mesh = Asset->Meshes + MeshIndex;

        model_asset_mesh_header MeshHeader = {};
        MeshHeader.MaterialIndex = Mesh->MaterialIndex;
        MeshHeader.VertexCount = Mesh->VertexCount;
        MeshHeader.IndexCount = Mesh->IndexCount;
        MeshHeader.VerticesOffset = MeshesHeader.MeshesOffset + sizeof(model_asset_mesh_header) + TotalPrevMeshSize;
        MeshHeader.IndicesOffset = MeshHeader.VerticesOffset + GetMeshVerticesSize(Mesh);

        MeshHeader.HasPositions = Mesh->Positions != 0;
        MeshHeader.HasNormals = Mesh->Normals != 0;
        MeshHeader.HasTangents = Mesh->Tangents != 0;
        MeshHeader.HasBitangets = Mesh->Bitangents != 0;
        MeshHeader.HasTextureCoords = Mesh->TextureCoords != 0;
        MeshHeader.HasWeights = Mesh->Weights != 0;
        MeshHeader.HasJointIndices = Mesh->JointIndices != 0;

        TotalPrevMeshSize += sizeof(model_asset_mesh_header) + GetMeshVerticesSize(Mesh) + Mesh->IndexCount * sizeof(u32);

        fwrite(&MeshHeader, sizeof(model_asset_mesh_header), 1, AssetFile);

        if (MeshHeader.HasPositions)
        {
            fwrite(Mesh->Positions, sizeof(vec3), Mesh->VertexCount, AssetFile);
        }

        if (MeshHeader.HasNormals)
        {
            fwrite(Mesh->Normals, sizeof(vec3), Mesh->VertexCount, AssetFile);
        }

        if (MeshHeader.HasTangents)
        {
            fwrite(Mesh->Tangents, sizeof(vec3), Mesh->VertexCount, AssetFile);
        }

        if (MeshHeader.HasBitangets)
        {
            fwrite(Mesh->Bitangents, sizeof(vec3), Mesh->VertexCount, AssetFile);
        }

        if (MeshHeader.HasTextureCoords)
        {
            fwrite(Mesh->TextureCoords, sizeof(vec2), Mesh->VertexCount, AssetFile);
        }

        if (MeshHeader.HasWeights)
        {
            fwrite(Mesh->Weights, sizeof(vec4), Mesh->VertexCount, AssetFile);
        }

        if (MeshHeader.HasJointIndices)
        {
            fwrite(Mesh->JointIndices, sizeof(i32) * 4, Mesh->VertexCount, AssetFile);
        }

        fwrite(Mesh->Indices, sizeof(u32), Mesh->IndexCount, AssetFile);
    }
}

internal void
WriteMaterials(model_asset *Asset, u64 Offset, FILE *AssetFile)
{
    model_asset_materials_header MaterialsHeader = {};
    MaterialsHeader.MaterialCount = Asset->MaterialCount;
    MaterialsHeader.MaterialsOffset = Offset + sizeof(model_asset_materials_header);

    fwrite(&MaterialsHeader, sizeof(model_asset_materials_header), 1, AssetFile);

    u32 TotalPrevPropertiesSize = 0;
    for (u32 MaterialIndex = 0; MaterialIndex < MaterialsHeader.MaterialCount; ++MaterialIndex)
    {
        mesh_material *Material = Asset->Materials + MaterialIndex;

        model_asset_material_header MaterialHeader = {};
        MaterialHeader.PropertyCount = Material->PropertyCount;
        MaterialHeader.PropertiesOffset = MaterialsHeader.MaterialsOffset + sizeof(model_asset_material_header) +
            MaterialIndex * sizeof(model_asset_material_header) + TotalPrevPropertiesSize;

        fwrite(&MaterialHeader, sizeof(model_asset_material_header), 1, AssetFile);

        u32 PrevPropertiesSize = 0;
        for (u32 MaterialPropertyIndex = 0; MaterialPropertyIndex < Material->PropertyCount; ++MaterialPropertyIndex)
        {
            material_property *MaterialProperty = Material->Properties + MaterialPropertyIndex;

            model_asset_material_property_header MaterialPropertyHeader = {};
            MaterialPropertyHeader.Type = MaterialProperty->Type;

            switch (MaterialProperty->Type)
            {
                case MaterialProperty_Float_Shininess:
                {
                    MaterialPropertyHeader.Value = MaterialProperty->Value;

                    fwrite(&MaterialPropertyHeader, sizeof(model_asset_material_property_header), 1, AssetFile);

                    PrevPropertiesSize += sizeof(model_asset_material_property_header);

                    break;
                }
                case MaterialProperty_Color_Ambient:
                case MaterialProperty_Color_Diffuse:
                case MaterialProperty_Color_Specular:
                {
                    MaterialPropertyHeader.Color = MaterialProperty->Color;

                    fwrite(&MaterialPropertyHeader, sizeof(model_asset_material_property_header), 1, AssetFile);

                    PrevPropertiesSize += sizeof(model_asset_material_property_header);

                    break;
                }
                case MaterialProperty_Texture_Diffuse:
                case MaterialProperty_Texture_Specular:
                case MaterialProperty_Texture_Shininess:
                case MaterialProperty_Texture_Normal:
                {
                    MaterialPropertyHeader.Bitmap = MaterialProperty->Bitmap;
                    MaterialPropertyHeader.BitmapOffset = MaterialHeader.PropertiesOffset + sizeof(model_asset_material_property_header) + PrevPropertiesSize;

                    fwrite(&MaterialPropertyHeader, sizeof(model_asset_material_property_header), 1, AssetFile);

                    u32 BitmapSize = MaterialProperty->Bitmap.Width * MaterialProperty->Bitmap.Height * MaterialProperty->Bitmap.Channels;
                    fwrite(MaterialPropertyHeader.Bitmap.Pixels, sizeof(u8), BitmapSize, AssetFile);

                    PrevPropertiesSize += sizeof(model_asset_material_property_header) + BitmapSize;

                    break;
                }
                default:
                {
                    Assert(!"Invalid material property");
                }
            }
        }

        TotalPrevPropertiesSize += PrevPropertiesSize;
    }
}

internal void
WriteAnimationClips(model_asset *Asset, u64 Offset, FILE *AssetFile)
{
    model_asset_animations_header AnimationsHeader = {};
    AnimationsHeader.AnimationCount = Asset->AnimationCount;
    AnimationsHeader.AnimationsOffset = Offset + sizeof(model_asset_animations_header);

    fwrite(&AnimationsHeader, sizeof(model_asset_animations_header), 1, AssetFile);

    u64 TotalPrevKeyFrameSize = 0;
    for (u32 AnimationIndex = 0; AnimationIndex < Asset->AnimationCount; ++AnimationIndex)
    {
        animation_clip *Animation = Asset->Animations + AnimationIndex;

        model_asset_animation_header AnimationHeader = {};
        CopyString(Animation->Name, AnimationHeader.Name);
        AnimationHeader.Duration = Animation->Duration;
        AnimationHeader.PoseSampleCount = Animation->PoseSampleCount;
        AnimationHeader.PoseSamplesOffset = AnimationsHeader.AnimationsOffset + sizeof(model_asset_animation_header) +
            AnimationIndex * sizeof(model_asset_animation_header) + TotalPrevKeyFrameSize;

        fwrite(&AnimationHeader, sizeof(model_asset_animation_header), 1, AssetFile);

        u64 PrevKeyFrameSize = 0;
        for (u32 AnimationPoseIndex = 0; AnimationPoseIndex < Animation->PoseSampleCount; ++AnimationPoseIndex)
        {
            animation_sample *AnimationPose = Animation->PoseSamples + AnimationPoseIndex;

            model_asset_animation_sample_header AnimationSampleHeader = {};
            AnimationSampleHeader.JointIndex = AnimationPose->JointIndex;
            AnimationSampleHeader.KeyFrameCount = AnimationPose->KeyFrameCount;
            AnimationSampleHeader.KeyFramesOffset = AnimationHeader.PoseSamplesOffset + sizeof(model_asset_animation_sample_header) +
                PrevKeyFrameSize;

            PrevKeyFrameSize += sizeof(model_asset_animation_sample_header) + AnimationSampleHeader.KeyFrameCount * sizeof(key_frame);

            fwrite(&AnimationSampleHeader, sizeof(model_asset_animation_sample_header), 1, AssetFile);
            fwrite(AnimationPose->KeyFrames, sizeof(key_frame), AnimationPose->KeyFrameCount, AssetFile);
        }

        TotalPrevKeyFrameSize += PrevKeyFrameSize;
    }
}

internal void
WriteModelAsset(const char *FilePath, model_asset *Asset)
{
    FILE *AssetFile = fopen(FilePath, "wb");

    if (!AssetFile)
    {
        errno_t Error;
        _get_errno(&Error);
        Assert(!"Panic");
    }

    asset_header Header = {};
    Header.MagicValue = 0x451;
    Header.Version = 1;
    Header.DataOffset = sizeof(asset_header);
    Header.Type = AssetType_Model;
    CopyString("Dummy model asset file", Header.Description);

    u64 CurrentStreamPosition = 0;

    fwrite(&Header, sizeof(asset_header), 1, AssetFile);

    CurrentStreamPosition = ftell(AssetFile);

    model_asset_header ModelAssetHeader = {};

    fwrite(&ModelAssetHeader, sizeof(model_asset_header), 1, AssetFile);

    CurrentStreamPosition = ftell(AssetFile);
    ModelAssetHeader.SkeletonHeaderOffset = CurrentStreamPosition;

    // Writing skeleton
    model_asset_skeleton_header SkeletonHeader = {};
    SkeletonHeader.JointCount = Asset->Skeleton.JointCount;
    SkeletonHeader.JointsOffset = ModelAssetHeader.SkeletonHeaderOffset + sizeof(SkeletonHeader);

    fwrite(&SkeletonHeader, sizeof(model_asset_skeleton_header), 1, AssetFile);
    fwrite(Asset->Skeleton.Joints, sizeof(joint), Asset->Skeleton.JointCount, AssetFile);

    CurrentStreamPosition = ftell(AssetFile);
    ModelAssetHeader.SkeletonPoseHeaderOffset = CurrentStreamPosition;

    // Writing skeleton bind pose
    model_asset_skeleton_pose_header SkeletonPoseHeader = {};
    SkeletonPoseHeader.LocalJointPosesOffset = ModelAssetHeader.SkeletonPoseHeaderOffset + sizeof(model_asset_skeleton_pose_header);
    SkeletonPoseHeader.GlobalJointPosesOffset = SkeletonPoseHeader.LocalJointPosesOffset + SkeletonHeader.JointCount * sizeof(joint_pose);

    fwrite(&SkeletonPoseHeader, sizeof(model_asset_skeleton_pose_header), 1, AssetFile);
    fwrite(Asset->BindPose.LocalJointPoses, sizeof(joint_pose), Asset->Skeleton.JointCount, AssetFile);
    fwrite(Asset->BindPose.GlobalJointPoses, sizeof(mat4), Asset->Skeleton.JointCount, AssetFile);

    CurrentStreamPosition = ftell(AssetFile);
    ModelAssetHeader.AnimationGraphHeaderOffset = CurrentStreamPosition;

    WriteAnimationGraph(&Asset->AnimationGraph, ModelAssetHeader.AnimationGraphHeaderOffset, AssetFile);

    CurrentStreamPosition = ftell(AssetFile);
    ModelAssetHeader.MeshesHeaderOffset = CurrentStreamPosition;

    WriteMeshes(Asset, ModelAssetHeader.MeshesHeaderOffset, AssetFile);

    CurrentStreamPosition = ftell(AssetFile);
    ModelAssetHeader.MaterialsHeaderOffset = CurrentStreamPosition;

    WriteMaterials(Asset, ModelAssetHeader.MaterialsHeaderOffset, AssetFile);

    CurrentStreamPosition = ftell(AssetFile);
    ModelAssetHeader.AnimationsHeaderOffset = CurrentStreamPosition;

    WriteAnimationClips(Asset, ModelAssetHeader.AnimationsHeaderOffset, AssetFile);

    fseek(AssetFile, (long) Header.DataOffset, SEEK_SET);
    fwrite(&ModelAssetHeader, sizeof(model_asset_header), 1, AssetFile);

    fclose(AssetFile);
}

internal void
LoadModelAsset(const char *FilePath, model_asset *Asset, u32 Flags)
{
    aiPropertyStore *AssimpPropertyStore = aiCreatePropertyStore();
    aiSetImportPropertyInteger(AssimpPropertyStore, AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, 0);

    const aiScene *AssimpScene = aiImportFileExWithProperties(FilePath, Flags, 0, AssimpPropertyStore);

    if (AssimpScene)
    {
        *Asset = {};

        ProcessAssimpScene(AssimpScene, Asset);

        aiReleaseImport(AssimpScene);
    }
    else
    {
        const char *ErrorMessage = aiGetErrorString();
        Assert(!ErrorMessage);
    }

    aiReleasePropertyStore(AssimpPropertyStore);
}

internal void
LoadAnimationClipAsset(const char *FilePath, u32 Flags, model_asset *Asset, const char *AnimationName, u32 AnimationIndex)
{
    const aiScene *AssimpScene = aiImportFile(FilePath, Flags);

    Assert(AssimpScene);
    Assert(AssimpScene->mNumAnimations == 1);

    aiAnimation *AssimpAnimation = AssimpScene->mAnimations[0];
    animation_clip *Animation = Asset->Animations + AnimationIndex;

    ProcessAssimpAnimation(AssimpAnimation, Animation, &Asset->Skeleton);

    CopyString(AnimationName, Animation->Name);

    aiReleaseImport(AssimpScene);
}

internal void
ProcessGraphNodes(animation_graph_asset *GraphAsset, Value &Nodes)
{
    GraphAsset->NodeCount = Nodes.Size();
    GraphAsset->Nodes = AllocateMemory<animation_node_asset>(GraphAsset->NodeCount);

    for (u32 NodeIndex = 0; NodeIndex < GraphAsset->NodeCount; ++NodeIndex)
    {
        animation_node_asset *NodeAsset = GraphAsset->Nodes + NodeIndex;

        Value &Node = Nodes[NodeIndex];

        const char *Name = Node["name"].GetString();
        const char *Type = Node["type"].GetString();

        CopyString(Name, NodeAsset->Name);

        Value &Transitions = Node["transitions"].GetArray();

        NodeAsset->TransitionCount = Transitions.Size();
        NodeAsset->Transitions = AllocateMemory<animation_transition_asset>(NodeAsset->TransitionCount);

        // Transitions
        for (u32 TransitionIndex = 0; TransitionIndex < NodeAsset->TransitionCount; ++TransitionIndex)
        {
            animation_transition_asset *TransitionAsset = NodeAsset->Transitions + TransitionIndex;

            Value &Message = Transitions[TransitionIndex];

            const char *From = Name;
            const char *To = Message["to"].GetString();

            CopyString(From, TransitionAsset->From);
            CopyString(To, TransitionAsset->To);

            const char *TransitionType = Message["type"].GetString();

            if (StringEquals(TransitionType, "transitional"))
            {
                TransitionAsset->Type = AnimationTransitionType_Transitional;

                const char *TransitionNode = Message["through"].GetString();
                CopyString(TransitionNode, TransitionAsset->TransitionNode);
            }
            else if (StringEquals(TransitionType, "crossfade"))
            {
                TransitionAsset->Type = AnimationTransitionType_Crossfade;
                TransitionAsset->Duration = Message["blend"].GetFloat();
            }
            else if (StringEquals(TransitionType, "immediate"))
            {
                TransitionAsset->Type = AnimationTransitionType_Immediate;
            }
            else
            {
                Assert(!"Unknown transition type");
            }
        }

        // Nodes
        if (StringEquals(Type, "Graph"))
        {
            NodeAsset->Type = AnimationNodeType_Graph;
            NodeAsset->Graph = AllocateMemory<animation_graph_asset>();

            const char *Name = Node["name"].GetString();
            const char *Entry = Node["entry"].GetString();
            Value &SubNodes = Node["nodes"].GetArray();

            CopyString(Name, NodeAsset->Graph->Name);
            CopyString(Entry, NodeAsset->Graph->Entry);

            ProcessGraphNodes(NodeAsset->Graph, SubNodes);
        }
        else if (StringEquals(Type, "Blendspace"))
        {
            NodeAsset->Type = AnimationNodeType_BlendSpace;
            NodeAsset->Blendspace = AllocateMemory<blend_space_1d_asset>();

            Value &Values = Node["values"].GetArray();

            NodeAsset->Blendspace->ValueCount = Values.Size();
            NodeAsset->Blendspace->Values = AllocateMemory<blend_space_1d_value_asset>(NodeAsset->Blendspace->ValueCount);

            for (u32 ValueIndex = 0; ValueIndex < NodeAsset->Blendspace->ValueCount; ++ValueIndex)
            {
                blend_space_1d_value_asset *ValueAsset = NodeAsset->Blendspace->Values + ValueIndex;

                Value &BlendspaceValue = Values[ValueIndex];

                const char *Clip = BlendspaceValue["clip"].GetString();

                CopyString(Clip, ValueAsset->AnimationClipName);
                ValueAsset->Value = BlendspaceValue["value"].GetFloat();
            }
        }
        else if (StringEquals(Type, "Animation"))
        {
            NodeAsset->Type = AnimationNodeType_Clip;
            NodeAsset->Animation = AllocateMemory<animation_state_asset>();

            const char *Clip = Node["clip"].GetString();
            b32 IsLooping = Node["looping"].GetBool();

            CopyString(Clip, NodeAsset->Animation->AnimationClipName);
            NodeAsset->Animation->IsLooping = IsLooping;
        }
        else
        {
            Assert(!"Unknown node type");
        }
    }
}

internal void
LoadAnimationGrah(const char *FilePath, model_asset *Asset)
{
    char *Contents = ReadTextFile(FilePath);

    if (Contents)
    {
        Document Document;
        ParseResult ok = Document.Parse(Contents);

        if (ok)
        {
            animation_graph_asset *GraphAsset = &Asset->AnimationGraph;

            const char *Version = Document["version"].GetString();
            Value &Graph = Document["graph"];

            const char *Name = Graph["name"].GetString();
            const char *Entry = Graph["entry"].GetString();
            Value &Nodes = Graph["nodes"].GetArray();

            CopyString(Name, GraphAsset->Name);
            CopyString(Entry, GraphAsset->Entry);

            ProcessGraphNodes(GraphAsset, Nodes);
        }
        else
        {
            Assert(!"Failed to parse animation graph");
        }
    }
}

// todo: https://github.com/f3d-app/f3d/issues/414 !!!
internal void
LoadAnimationClips(const char *DirectoryPath, u32 Flags, model_asset *Asset)
{
    if (fs::exists(DirectoryPath))
    {
        u32 AnimationCount = 0;

        for (const fs::directory_entry &Entry : fs::directory_iterator(DirectoryPath))
        {
            if (Entry.is_regular_file())
            {
                ++AnimationCount;
            }
        }

        Asset->AnimationCount = AnimationCount;
        Asset->Animations = AllocateMemory<animation_clip>(Asset->AnimationCount);

        u32 AnimationIndex = 0;
        for (const fs::directory_entry &Entry : fs::directory_iterator(DirectoryPath))
        {
            if (Entry.is_regular_file())
            {
                fs::path FileName = Entry.path().filename();
                fs::path AnimationClipName = Entry.path().stem();

                char FilePath[64];
                FormatString(FilePath, "%s/%s", DirectoryPath, FileName.generic_string().c_str());

                LoadAnimationClipAsset(FilePath, Flags, Asset, AnimationClipName.generic_string().c_str(), AnimationIndex++);
            }
        }
    }
}

internal void
ProcessAsset(const char *FilePath, const char *AnimationConfigPath, const char *AnimationClipsPath, const char *OutputPath)
{
    u32 Flags =
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_GenNormals |
        aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices |
        aiProcess_ValidateDataStructure |
        aiProcess_LimitBoneWeights |
        aiProcess_RemoveRedundantMaterials |
        aiProcess_FixInfacingNormals |
        aiProcess_GlobalScale |
        aiProcess_OptimizeGraph |
        aiProcess_OptimizeMeshes;

    model_asset Asset;

    LoadModelAsset(FilePath, &Asset, Flags);
    LoadAnimationGrah(AnimationConfigPath, &Asset);
    LoadAnimationClips(AnimationClipsPath, Flags, &Asset);

    WriteModelAsset(OutputPath, &Asset);

#if 1
    model_asset TestAsset = {};
    ReadModelAsset(OutputPath, &TestAsset, &Asset);
#endif
}
