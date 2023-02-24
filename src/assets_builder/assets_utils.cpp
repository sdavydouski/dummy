#define INVALID_FLOAT -1.f
#define INVALID_COLOR vec4(-1.f)

#define MAX_MATERIAL_PROPERTY_COUNT 16u
#define TEXTURE_COORDINATES_SET_INDEX 0

#define MAX_WEIGHT_COUNT 4
#undef AI_CONFIG_PP_LBW_MAX_WEIGHTS
#define AI_CONFIG_PP_LBW_MAX_WEIGHTS MAX_WEIGHT_COUNT

struct assimp_node
{
    aiNode *Node;
    aiBone *Bone;
};

template <typename T>
inline T *
AllocateMemory(umm Count = 1)
{
    T *Result = (T *) calloc(Count, sizeof(T));

    Assert(Result);

    return Result;
}

inline u32
GetFileSize(FILE *File)
{
    u32 FileSize = 0;

    fseek(File, 0, SEEK_END);
    FileSize = ftell(File);
    fseek(File, 0, SEEK_SET);

    return FileSize;
}

internal char *
ReadTextFile(const char *FileName)
{
    char *Result = 0;
    FILE *File = fopen(FileName, "rb");

    if (File)
    {
        u32 Length = GetFileSize(File);

        Result = AllocateMemory<char>(Length + 1);

        fread(Result, 1, Length, File);
        Result[Length] = 0;

        fclose(File);
    }

    return Result;
}

// Assimp utils
inline vec4
AssimpColor2Vector(aiColor4D AssimpColor)
{
    vec4 Result;

    Result.r = AssimpColor.r;
    Result.g = AssimpColor.g;
    Result.b = AssimpColor.b;
    Result.a = AssimpColor.a;

    return Result;
}

inline vec3
AssimpVector2Vector(aiVector3D AssimpVector)
{
    vec3 Result;

    Result.x = AssimpVector.x;
    Result.y = AssimpVector.y;
    Result.z = AssimpVector.z;

    return Result;
}

inline quat
AssimpQuaternion2Quaternion(aiQuaternion AssimpQuaternion)
{
    quat Result;

    Result.x = AssimpQuaternion.x;
    Result.y = AssimpQuaternion.y;
    Result.z = AssimpQuaternion.z;
    Result.w = AssimpQuaternion.w;

    return Result;
}

inline mat4
AssimpMatrix2Matrix(aiMatrix4x4 aiMatrix)
{
    mat4 Result = mat4(
        vec4(aiMatrix.a1, aiMatrix.a2, aiMatrix.a3, aiMatrix.a4),
        vec4(aiMatrix.b1, aiMatrix.b2, aiMatrix.b3, aiMatrix.b4),
        vec4(aiMatrix.c1, aiMatrix.c2, aiMatrix.c3, aiMatrix.c4),
        vec4(aiMatrix.d1, aiMatrix.d2, aiMatrix.d3, aiMatrix.d4)
    );

    return Result;
}

inline string
AssimpString2StdString(aiString AssimpString)
{
    string Result = string(AssimpString.C_Str());

    return Result;
}

inline aiTexture *
FindAssimpTextureByFileName(const aiScene *AssimpScene, aiString TexturePath)
{
    aiTexture *Result = 0;

    for (u32 TextureIndex = 0; TextureIndex < AssimpScene->mNumTextures; ++TextureIndex)
    {
        aiTexture *AssimpTexture = AssimpScene->mTextures[TextureIndex];

        if (AssimpTexture->mFilename == TexturePath)
        {
            Result = AssimpTexture;
            break;
        }
    }

    Assert(Result);

    return Result;
}

internal void
ProcessAssimpTextures(
    const aiScene *AssimpScene,
    aiMaterial *AssimpMaterial,
    aiTextureType AssimpTextureType,
    u32 *TextureCount,
    bitmap **Bitmaps
)
{
    *TextureCount = aiGetMaterialTextureCount(AssimpMaterial, AssimpTextureType);
    *Bitmaps = AllocateMemory<bitmap>(*TextureCount);

    for (u32 TextureIndex = 0; TextureIndex < *TextureCount; ++TextureIndex)
    {
        aiString TexturePath;
        if (aiGetMaterialTexture(AssimpMaterial, AssimpTextureType, TextureIndex, &TexturePath) == AI_SUCCESS)
        {
            bitmap *Bitmap = *Bitmaps + TextureIndex;

            aiTexture *AssimpTexture = FindAssimpTextureByFileName(AssimpScene, TexturePath);

            if (AssimpTexture->mHeight == 0)
            {
                // Compressed texture data
                i32 TextureWidth;
                i32 TextureHeight;
                i32 TextureChannels;
                void *Pixels = stbi_load_from_memory((stbi_uc *)AssimpTexture->pcData, AssimpTexture->mWidth, &TextureWidth, &TextureHeight, &TextureChannels, 0);

                Bitmap->Width = TextureWidth;
                Bitmap->Height = TextureHeight;
                Bitmap->Channels = TextureChannels;
                Bitmap->IsHDR = false;
                Bitmap->Pixels = Pixels;
            }
            else
            {
                Assert(!"Not implemented");
            }
        }
    }
}

internal void
ProcessAssimpMaterial(const aiScene *AssimpScene, aiMaterial *AssimpMaterial, mesh_material *Material)
{
    Material->Properties = AllocateMemory<material_property>(MAX_MATERIAL_PROPERTY_COUNT);
    u32 MaterialPropertyIndex = 0;

    f32 SpecularShininess;
    if (aiGetMaterialFloat(AssimpMaterial, AI_MATKEY_SHININESS, &SpecularShininess) == AI_SUCCESS)
    {
        material_property *MaterialProperty = Material->Properties + MaterialPropertyIndex;
        MaterialProperty->Type = MaterialProperty_Float_Shininess;
        MaterialProperty->Value = SpecularShininess;

        ++MaterialPropertyIndex;
    }

    aiColor4D AmbientColor;
    if (aiGetMaterialColor(AssimpMaterial, AI_MATKEY_COLOR_AMBIENT, &AmbientColor) == AI_SUCCESS)
    {
        material_property *MaterialProperty = Material->Properties + MaterialPropertyIndex;
        MaterialProperty->Type = MaterialProperty_Color_Ambient;
        MaterialProperty->Color = AssimpColor2Vector(AmbientColor);

        ++MaterialPropertyIndex;
    }

    aiColor4D DiffuseColor;
    if (aiGetMaterialColor(AssimpMaterial, AI_MATKEY_COLOR_DIFFUSE, &DiffuseColor) == AI_SUCCESS)
    {
        material_property *MaterialProperty = Material->Properties + MaterialPropertyIndex;
        MaterialProperty->Type = MaterialProperty_Color_Diffuse;
        MaterialProperty->Color = AssimpColor2Vector(DiffuseColor);

        ++MaterialPropertyIndex;
    }

    aiColor4D SpecularColor;
    if (aiGetMaterialColor(AssimpMaterial, AI_MATKEY_COLOR_SPECULAR, &SpecularColor) == AI_SUCCESS)
    {
        material_property *MaterialProperty = Material->Properties + MaterialPropertyIndex;
        MaterialProperty->Type = MaterialProperty_Color_Specular;
        MaterialProperty->Color = AssimpColor2Vector(SpecularColor);

        ++MaterialPropertyIndex;
    }

    u32 DiffuseMapCount = 0;
    bitmap *DiffuseMaps = 0;
    ProcessAssimpTextures(AssimpScene, AssimpMaterial, aiTextureType_DIFFUSE, &DiffuseMapCount, &DiffuseMaps);
    for (u32 DiffuseMapIndex = 0; DiffuseMapIndex < DiffuseMapCount; ++DiffuseMapIndex)
    {
        bitmap *DiffuseMap = DiffuseMaps + DiffuseMapIndex;

        material_property *MaterialProperty = Material->Properties + MaterialPropertyIndex;
        MaterialProperty->Type = MaterialProperty_Texture_Diffuse;
        MaterialProperty->Bitmap = *DiffuseMap;

        ++MaterialPropertyIndex;
    }

    u32 SpecularMapCount = 0;
    bitmap *SpecularMaps = 0;
    ProcessAssimpTextures(AssimpScene, AssimpMaterial, aiTextureType_SPECULAR, &SpecularMapCount, &SpecularMaps);
    for (u32 SpecularMapIndex = 0; SpecularMapIndex < SpecularMapCount; ++SpecularMapIndex)
    {
        bitmap *SpecularMap = SpecularMaps + SpecularMapIndex;

        material_property *MaterialProperty = Material->Properties + MaterialPropertyIndex;
        MaterialProperty->Type = MaterialProperty_Texture_Specular;
        MaterialProperty->Bitmap = *SpecularMap;

        ++MaterialPropertyIndex;
    }

    u32 ShininessMapCount = 0;
    bitmap *ShininessMaps = 0;
    ProcessAssimpTextures(AssimpScene, AssimpMaterial, aiTextureType_SHININESS, &ShininessMapCount, &ShininessMaps);
    for (u32 ShininessMapIndex = 0; ShininessMapIndex < ShininessMapCount; ++ShininessMapIndex)
    {
        bitmap *ShininessMap = ShininessMaps + ShininessMapIndex;

        material_property *MaterialProperty = Material->Properties + MaterialPropertyIndex;
        MaterialProperty->Type = MaterialProperty_Texture_Shininess;
        MaterialProperty->Bitmap = *ShininessMap;

        ++MaterialPropertyIndex;
    }

    u32 MetalnessMapCount = 0;
    bitmap *MetalnessMaps = 0;
    ProcessAssimpTextures(AssimpScene, AssimpMaterial, aiTextureType_METALNESS, &MetalnessMapCount, &MetalnessMaps);
    for (u32 MetalnessMapIndex = 0; MetalnessMapIndex < MetalnessMapCount; ++MetalnessMapIndex)
    {
        bitmap *MetalnessMap = MetalnessMaps + MetalnessMapIndex;

        material_property *MaterialProperty = Material->Properties + MaterialPropertyIndex;
        MaterialProperty->Type = MaterialProperty_Texture_Metalness;
        MaterialProperty->Bitmap = *MetalnessMap;

        ++MaterialPropertyIndex;
    }

    u32 NormalsMapCount = 0;
    bitmap *NormalsMaps = 0;
    ProcessAssimpTextures(AssimpScene, AssimpMaterial, aiTextureType_NORMALS, &NormalsMapCount, &NormalsMaps);
    for (u32 NormalsMapIndex = 0; NormalsMapIndex < NormalsMapCount; ++NormalsMapIndex)
    {
        bitmap *NormalsMap = NormalsMaps + NormalsMapIndex;

        material_property *MaterialProperty = Material->Properties + MaterialPropertyIndex;
        MaterialProperty->Type = MaterialProperty_Texture_Normal;
        MaterialProperty->Bitmap = *NormalsMap;

        ++MaterialPropertyIndex;
    }

#if 0
    {
        u32 MapCount = 0;
        bitmap *Maps = 0;
        ProcessAssimpTextures(AssimpScene, AssimpMaterial, aiTextureType_BASE_COLOR, &MetalnessMapCount, &MetalnessMaps);

        if (MapCount > 0)
        {
            Assert(!"Rumble");
        }
    }

    {
        u32 MapCount = 0;
        bitmap *Maps = 0;
        ProcessAssimpTextures(AssimpScene, AssimpMaterial, aiTextureType_NORMAL_CAMERA, &MetalnessMapCount, &MetalnessMaps);

        if (MapCount > 0)
        {
            Assert(!"Rumble");
        }
    }

    {
        u32 MapCount = 0;
        bitmap *Maps = 0;
        ProcessAssimpTextures(AssimpScene, AssimpMaterial, aiTextureType_EMISSION_COLOR, &MetalnessMapCount, &MetalnessMaps);

        if (MapCount > 0)
        {
            Assert(!"Rumble");
        }
    }

    {
        u32 MapCount = 0;
        bitmap *Maps = 0;
        ProcessAssimpTextures(AssimpScene, AssimpMaterial, aiTextureType_DIFFUSE_ROUGHNESS, &MetalnessMapCount, &MetalnessMaps);

        if (MapCount > 0)
        {
            Assert(!"Rumble");
        }
    }

    {
        u32 MapCount = 0;
        bitmap *Maps = 0;
        ProcessAssimpTextures(AssimpScene, AssimpMaterial, aiTextureType_AMBIENT_OCCLUSION, &MetalnessMapCount, &MetalnessMaps);

        if (MapCount > 0)
        {
            Assert(!"Rumble");
        }
    }

    {
        u32 MapCount = 0;
        bitmap *Maps = 0;
        ProcessAssimpTextures(AssimpScene, AssimpMaterial, aiTextureType_DISPLACEMENT, &MetalnessMapCount, &MetalnessMaps);

        if (MapCount > 0)
        {
            Assert(!"Rumble");
        }
    }
    {
        u32 MapCount = 0;
        bitmap *Maps = 0;
        ProcessAssimpTextures(AssimpScene, AssimpMaterial, aiTextureType_EMISSIVE, &MetalnessMapCount, &MetalnessMaps);

        if (MapCount > 0)
        {
            Assert(!"Rumble");
        }
    }

    {
        u32 MapCount = 0;
        bitmap *Maps = 0;
        ProcessAssimpTextures(AssimpScene, AssimpMaterial, aiTextureType_HEIGHT, &MetalnessMapCount, &MetalnessMaps);

        if (MapCount > 0)
        {
            Assert(!"Rumble");
        }
    }

    {
        u32 MapCount = 0;
        bitmap *Maps = 0;
        ProcessAssimpTextures(AssimpScene, AssimpMaterial, aiTextureType_UNKNOWN, &MetalnessMapCount, &MetalnessMaps);

        if (MapCount > 0)
        {
            Assert(!"Rumble");
        }
    }
#endif

    Material->PropertyCount = MaterialPropertyIndex;

    Assert(MaterialPropertyIndex < MAX_MATERIAL_PROPERTY_COUNT);
}

inline u32
FindJointIndexByName(const char *JointName, skeleton *Skeleton)
{
    u32 Result = -1;

    for (u32 JointIndex = 0; JointIndex < Skeleton->JointCount; ++JointIndex)
    {
        joint *Joint = Skeleton->Joints + JointIndex;

        if (StringEquals(JointName, Joint->Name))
        {
            Result = JointIndex;
            break;
        }
    }

    Assert(Result != -1);

    return Result;
}

internal void
ProcessAssimpAnimation(aiAnimation *AssimpAnimation, animation_clip *Animation, skeleton *Skeleton)
{
    Assert(AssimpAnimation->mName.length < MAX_ANIMATION_NAME_LENGTH);

    //CopyString(AssimpAnimation->mName.C_Str(), Animation->Name);
    Animation->Duration = (f32)AssimpAnimation->mDuration / (f32)AssimpAnimation->mTicksPerSecond;
    Animation->PoseSampleCount = AssimpAnimation->mNumChannels;
    Animation->PoseSamples = AllocateMemory<animation_sample>(Animation->PoseSampleCount);

    for (u32 ChannelIndex = 0; ChannelIndex < AssimpAnimation->mNumChannels; ++ChannelIndex)
    {
        // Channel desribes the movement of a single joint over time
        aiNodeAnim *Channel = AssimpAnimation->mChannels[ChannelIndex];

        Assert((Channel->mNumPositionKeys == Channel->mNumRotationKeys) && (Channel->mNumPositionKeys == Channel->mNumScalingKeys));

        animation_sample *AnimationSample = Animation->PoseSamples + ChannelIndex;
        AnimationSample->JointIndex = FindJointIndexByName(Channel->mNodeName.C_Str(), Skeleton);
        AnimationSample->KeyFrameCount = Channel->mNumPositionKeys;
        AnimationSample->KeyFrames = AllocateMemory<key_frame>(AnimationSample->KeyFrameCount);

        for (u32 KeyIndex = 0; KeyIndex < Channel->mNumPositionKeys; ++KeyIndex)
        {
            aiVectorKey *PositionKey = Channel->mPositionKeys + KeyIndex;
            aiQuatKey *RotationKey = Channel->mRotationKeys + KeyIndex;
            aiVectorKey *ScalingKey = Channel->mScalingKeys + KeyIndex;

            Assert((PositionKey->mTime == RotationKey->mTime) && (PositionKey->mTime == ScalingKey->mTime));

            key_frame *KeyFrame = AnimationSample->KeyFrames + KeyIndex;
            KeyFrame->Time = (f32)PositionKey->mTime / (f32)AssimpAnimation->mTicksPerSecond;
            KeyFrame->Pose.Rotation = AssimpQuaternion2Quaternion(RotationKey->mValue);
            KeyFrame->Pose.Translation = AssimpVector2Vector(PositionKey->mValue);
            KeyFrame->Pose.Scale = AssimpVector2Vector(ScalingKey->mValue);
        }
    }
}

internal void
ProcessAssimpMesh(aiMesh *AssimpMesh, u32 AssimpMeshIndex, aiNode *AssimpRootNode, mesh *Mesh, skeleton *Skeleton)
{
    Mesh->MaterialIndex = AssimpMesh->mMaterialIndex;
    Mesh->VertexCount = AssimpMesh->mNumVertices;

    Mesh->Positions = 0;
    Mesh->Normals = 0;
    Mesh->Tangents = 0;
    Mesh->Bitangents = 0;
    Mesh->TextureCoords = 0;
    Mesh->Weights = 0;
    Mesh->JointIndices = 0;

    if (AssimpMesh->HasPositions())
    {
        Mesh->Positions = AllocateMemory<vec3>(Mesh->VertexCount);
    }

    if (AssimpMesh->HasNormals())
    {
        Mesh->Normals = AllocateMemory<vec3>(Mesh->VertexCount);
    }

    if (AssimpMesh->HasTangentsAndBitangents())
    {
        Mesh->Tangents = AllocateMemory<vec3>(Mesh->VertexCount);
        Mesh->Bitangents = AllocateMemory<vec3>(Mesh->VertexCount);
    }

    if (AssimpMesh->HasTextureCoords(TEXTURE_COORDINATES_SET_INDEX))
    {
        Mesh->TextureCoords = AllocateMemory<vec2>(Mesh->VertexCount);
    }

    for (u32 VertexIndex = 0; VertexIndex < AssimpMesh->mNumVertices; ++VertexIndex)
    {
        if (AssimpMesh->HasPositions())
        {
            vec3 *Position = Mesh->Positions + VertexIndex;

            aiVector3D AssimpPosition = AssimpMesh->mVertices[VertexIndex];
            *Position = AssimpVector2Vector(AssimpPosition);
        }

        if (AssimpMesh->HasNormals())
        {
            vec3 *Normal = Mesh->Normals + VertexIndex;

            aiVector3D AssimpNormal = AssimpMesh->mNormals[VertexIndex];
            *Normal = AssimpVector2Vector(AssimpNormal);
        }

        if (AssimpMesh->HasTangentsAndBitangents())
        {
            vec3 *Tangent = Mesh->Tangents + VertexIndex;
            vec3 *Bitangent = Mesh->Bitangents + VertexIndex;

            aiVector3D AssimpTangent = AssimpMesh->mTangents[VertexIndex];
            aiVector3D AssimpBiTangent = AssimpMesh->mBitangents[VertexIndex];

            *Tangent = AssimpVector2Vector(AssimpTangent);
            *Bitangent = AssimpVector2Vector(AssimpBiTangent);
        }

        if (AssimpMesh->HasTextureCoords(TEXTURE_COORDINATES_SET_INDEX))
        {
            vec2 *TextureCoords = Mesh->TextureCoords + VertexIndex;

            aiVector3D AssimpTextureCoords = AssimpMesh->mTextureCoords[TEXTURE_COORDINATES_SET_INDEX][VertexIndex];
            *TextureCoords = vec2(AssimpVector2Vector(AssimpTextureCoords).xy);
        }
    }

    if (AssimpMesh->HasBones())
    {
        Mesh->Weights = AllocateMemory<vec4>(Mesh->VertexCount);
        Mesh->JointIndices = AllocateMemory<ivec4>(Mesh->VertexCount);

        hashtable<u32, dynamic_array<joint_weight>> JointWeightsTable = {};

        for (u32 BoneIndex = 0; BoneIndex < AssimpMesh->mNumBones; ++BoneIndex)
        {
            aiBone *AssimpBone = AssimpMesh->mBones[BoneIndex];

            for (u32 VertexWeightIndex = 0; VertexWeightIndex < AssimpBone->mNumWeights; ++VertexWeightIndex)
            {
                aiVertexWeight *AssimpVertexWeight = AssimpBone->mWeights + VertexWeightIndex;

                u32 VertexIndex = AssimpVertexWeight->mVertexId;
                f32 JointWeight = AssimpVertexWeight->mWeight;
                u32 JointIndex = FindJointIndexByName(AssimpBone->mName.C_Str(), Skeleton);

                JointWeightsTable[VertexIndex].push_back({ JointIndex, JointWeight });
            }
        }

        hashtable<u32, dynamic_array<joint_weight>>::iterator it;
        for (it = JointWeightsTable.begin(); it != JointWeightsTable.end(); ++it)
        {
            dynamic_array<joint_weight> &JointWeights = it->second;

            std::sort(
                JointWeights.begin(),
                JointWeights.end(),
                [](const joint_weight &A, const joint_weight &B) -> bool32
                {
                    return A.Weight > B.Weight;
                }
            );

            JointWeights.resize(MAX_WEIGHT_COUNT);

        }

        for (u32 VertexIndex = 0; VertexIndex < Mesh->VertexCount; ++VertexIndex)
        {
            vec4 *Weight = Mesh->Weights + VertexIndex;
            ivec4 *JointIndices = Mesh->JointIndices + VertexIndex;

            dynamic_array<joint_weight> &JointWeights = JointWeightsTable[VertexIndex];

            for (u32 JointWeightIndex = 0; JointWeightIndex < MAX_WEIGHT_COUNT; ++JointWeightIndex)
            {
                joint_weight JointWeight = JointWeights[JointWeightIndex];

                JointIndices->Elements[JointWeightIndex] = JointWeight.JointIndex;
                Weight->Elements[JointWeightIndex] = JointWeight.Weight;
            }

            Weight->Elements[3] = 1.f - (Weight->Elements[0] + Weight->Elements[1] + Weight->Elements[2]);

            Assert(Weight->Elements[0] + Weight->Elements[1] + Weight->Elements[2] + Weight->Elements[3] == 1.f);
        }
    }

    if (AssimpMesh->HasFaces())
    {
        u32 IndexCount = 0;

        for (u32 FaceIndex = 0; FaceIndex < AssimpMesh->mNumFaces; ++FaceIndex)
        {
            aiFace *Face = AssimpMesh->mFaces + FaceIndex;
            IndexCount += Face->mNumIndices;
        }

        Mesh->IndexCount = IndexCount;
        Mesh->Indices = AllocateMemory<u32>(Mesh->IndexCount);

        for (u32 FaceIndex = 0; FaceIndex < AssimpMesh->mNumFaces; ++FaceIndex)
        {
            aiFace *Face = AssimpMesh->mFaces + FaceIndex;

            for (u32 FaceIndexIndex = 0; FaceIndexIndex < Face->mNumIndices; ++FaceIndexIndex)
            {
                u32 *Index = Mesh->Indices + (FaceIndex * Face->mNumIndices + FaceIndexIndex);
                *Index = Face->mIndices[FaceIndexIndex];
            }
        }
    }
}

internal void
ProcessAssimpNodeHierarchy(aiNode *AssimpNode, hashtable<string, assimp_node *> &SceneNodes)
{
    assimp_node *Node = AllocateMemory<assimp_node>();
    Node->Node = AssimpNode;
    Node->Bone = 0;             // will be filled later

    string Name = AssimpString2StdString(AssimpNode->mName);
    SceneNodes[Name] = Node;

    for (u32 ChildIndex = 0; ChildIndex < AssimpNode->mNumChildren; ++ChildIndex)
    {
        aiNode *ChildNode = AssimpNode->mChildren[ChildIndex];
        ProcessAssimpNodeHierarchy(ChildNode, SceneNodes);
    }
}

internal aiNode *
GetBoneRootNode(aiNode *AssimpNode, hashtable<string, assimp_node *> &SceneNodes)
{
    string Name = AssimpString2StdString(AssimpNode->mName);
    assimp_node *SceneNode = SceneNodes[Name];
    if (SceneNode->Bone)
    {
        return SceneNode->Node;
    }

    for (u32 ChildIndex = 0; ChildIndex < AssimpNode->mNumChildren; ++ChildIndex)
    {
        aiNode *ChildNode = AssimpNode->mChildren[ChildIndex];
        return GetBoneRootNode(ChildNode, SceneNodes);
    }

    return 0;
}

inline aiNode *
GetAssimpRootNode(const aiScene *AssimpScene)
{
    aiNode *RootNode = AssimpScene->mRootNode;

    for (u32 ChildIndex = 0; ChildIndex < RootNode->mNumChildren; ++ChildIndex)
    {
        aiNode *ChildNode = RootNode->mChildren[ChildIndex];

        if (ChildNode->mName == aiString("RootNode"))
        {
            RootNode = ChildNode;
            break;
        }
    }

    return RootNode;
}

internal void
ProcessAssimpBoneHierarchy(
    aiNode *AssimpNode,
    hashtable<string, assimp_node *> &SceneNodes,
    skeleton_pose *Pose,
    i32 &CurrentIndexForward,
    i32 &CurrentIndexParent
)
{
    string Name = AssimpString2StdString(AssimpNode->mName);
    assimp_node *SceneNode = SceneNodes[Name];

    joint *Joint = Pose->Skeleton->Joints + CurrentIndexForward;
    Assert(Name.length() < MAX_JOINT_NAME_LENGTH);
    CopyString(Name.c_str(), Joint->Name);
    Joint->InvBindTranform = SceneNode->Bone
        ? AssimpMatrix2Matrix(SceneNode->Bone->mOffsetMatrix)
        : mat4(1.f);

    Joint->ParentIndex = CurrentIndexParent;

    aiVector3D Scale;
    aiVector3D Translation;
    aiQuaternion Rotation;
    SceneNode->Node->mTransformation.Decompose(Scale, Rotation, Translation);

    transform *LocalJointPose = Pose->LocalJointPoses + CurrentIndexForward;
    LocalJointPose->Scale = AssimpVector2Vector(Scale);
    LocalJointPose->Translation = AssimpVector2Vector(Translation);
    LocalJointPose->Rotation = AssimpQuaternion2Quaternion(Rotation);

    CurrentIndexParent = CurrentIndexForward;
    ++CurrentIndexForward;

    for (u32 ChildIndex = 0; ChildIndex < AssimpNode->mNumChildren; ++ChildIndex)
    {
        aiNode *ChildNode = AssimpNode->mChildren[ChildIndex];

        ProcessAssimpBoneHierarchy(ChildNode, SceneNodes, Pose, CurrentIndexForward, CurrentIndexParent);
    }

    CurrentIndexParent = Joint->ParentIndex;
}

// todo: duplicate
internal mat4
CalculateGlobalJointPose(joint *CurrentJoint, transform *CurrentJointPose, skeleton_pose *Pose)
{
    mat4 Result = mat4(1.f);

    while (true)
    {
        mat4 GlobalPose = Transform(*CurrentJointPose);
        Result = GlobalPose * Result;

        if (CurrentJoint->ParentIndex == -1)
        {
            break;
        }

        CurrentJointPose = Pose->LocalJointPoses + CurrentJoint->ParentIndex;
        CurrentJoint = Pose->Skeleton->Joints + CurrentJoint->ParentIndex;
    }

    return Result;
}

// todo: duplicate
inline u32
GetMeshVerticesSize(mesh *Mesh)
{
    u32 Size = 0;

    if (Mesh->Positions)
    {
        Size += Mesh->VertexCount * sizeof(vec3);
    }

    if (Mesh->Normals)
    {
        Size += Mesh->VertexCount * sizeof(vec3);
    }

    if (Mesh->Tangents)
    {
        Size += Mesh->VertexCount * sizeof(vec3);
    }

    if (Mesh->Bitangents)
    {
        Size += Mesh->VertexCount * sizeof(vec3);
    }

    if (Mesh->TextureCoords)
    {
        Size += Mesh->VertexCount * sizeof(vec2);
    }

    if (Mesh->Weights)
    {
        Size += Mesh->VertexCount * sizeof(vec4);
    }

    if (Mesh->JointIndices)
    {
        Size += Mesh->VertexCount * sizeof(ivec4);
    }

    return Size;
}

// todo: duplicate
internal bounds
CalculateAxisAlignedBoundingBox(u32 VertexCount, vec3 *Vertices)
{
    vec3 vMin = Vertices[0];
    vec3 vMax = Vertices[0];

    for (u32 VertexIndex = 1; VertexIndex < VertexCount; ++VertexIndex)
    {
        vec3 *Vertex = Vertices + VertexIndex;

        vMin = Min(vMin, *Vertex);
        vMax = Max(vMax, *Vertex);
    }

    bounds Result = {};

    Result.Min = vMin;
    Result.Max = vMax;

    return Result;
}

internal bounds
CalculateAxisAlignedBoundingBox(u32 MeshCount, mesh *Meshes)
{
    bounds Result = {};

    if (MeshCount > 0)
    {
        mesh *FirstMesh = First(Meshes);
        bounds Box = CalculateAxisAlignedBoundingBox(FirstMesh->VertexCount, FirstMesh->Positions);

        vec3 vMin = Box.Min;
        vec3 vMax = Box.Max;

        for (u32 MeshIndex = 1; MeshIndex < MeshCount; ++MeshIndex)
        {
            mesh *Mesh = Meshes + MeshIndex;
            bounds Box = CalculateAxisAlignedBoundingBox(Mesh->VertexCount, Mesh->Positions);

            vMin = Min(vMin, Box.Min);
            vMax = Max(vMax, Box.Max);
        }

        Result.Min = vMin;
        Result.Max = vMax;
    }

    return Result;
}

internal void
ProcessAssimpSkeleton(const aiScene *AssimpScene, skeleton *Skeleton, skeleton_pose *Pose)
{
    hashtable<string, assimp_node *> SceneNodes;

    aiNode *RootNode = GetAssimpRootNode(AssimpScene);
    ProcessAssimpNodeHierarchy(RootNode, SceneNodes);

    u32 JointCount = (u32)SceneNodes.size();

    //if (JointCount > 1)
    {
        for (u32 MeshIndex = 0; MeshIndex < AssimpScene->mNumMeshes; ++MeshIndex)
        {
            aiMesh *AssimpMesh = AssimpScene->mMeshes[MeshIndex];

            for (u32 BoneIndex = 0; BoneIndex < AssimpMesh->mNumBones; ++BoneIndex)
            {
                aiBone *AssimpBone = AssimpMesh->mBones[BoneIndex];
                aiNode *AssimpNode = AssimpScene->mRootNode->FindNode(AssimpBone->mName);

                string Name = AssimpString2StdString(AssimpNode->mName);
                SceneNodes[Name]->Bone = AssimpBone;
            }
        }

        Skeleton->JointCount = JointCount;
        Skeleton->Joints = AllocateMemory<joint>(JointCount);

        Pose->Skeleton = Skeleton;
        Pose->LocalJointPoses = AllocateMemory<transform>(JointCount);
        Pose->GlobalJointPoses = AllocateMemory<mat4>(JointCount);

        i32 CurrentIndexForward = 0;
        i32 CurrentIndexParent = -1;
        ProcessAssimpBoneHierarchy(RootNode, SceneNodes, Pose, CurrentIndexForward, CurrentIndexParent);

        for (u32 JointIndex = 0; JointIndex < JointCount; ++JointIndex)
        {
            joint *Joint = Skeleton->Joints + JointIndex;
            transform *LocalJointPose = Pose->LocalJointPoses + JointIndex;
            mat4 *GlobalJointPose = Pose->GlobalJointPoses + JointIndex;

            *GlobalJointPose = CalculateGlobalJointPose(Joint, LocalJointPose, Pose);
        }
    }
}

internal void
ProcessAssimpScene(const aiScene *AssimpScene, model_asset *Asset)
{
    ProcessAssimpSkeleton(AssimpScene, &Asset->Skeleton, &Asset->BindPose);

    if (AssimpScene->HasMeshes())
    {
        Asset->MeshCount = AssimpScene->mNumMeshes;
        Asset->Meshes = AllocateMemory<mesh>(Asset->MeshCount);

        for (u32 MeshIndex = 0; MeshIndex < Asset->MeshCount; ++MeshIndex)
        {
            aiMesh *AssimpMesh = AssimpScene->mMeshes[MeshIndex];
            mesh *Mesh = Asset->Meshes + MeshIndex;

            ProcessAssimpMesh(AssimpMesh, MeshIndex, AssimpScene->mRootNode, Mesh, &Asset->Skeleton);
        }
    }

    if (AssimpScene->HasMaterials())
    {
        Asset->MaterialCount = AssimpScene->mNumMaterials;
        Asset->Materials = AllocateMemory<mesh_material>(Asset->MaterialCount);

        for (u32 MaterialIndex = 0; MaterialIndex < Asset->MaterialCount; ++MaterialIndex)
        {
            aiMaterial *AssimpMaterial = AssimpScene->mMaterials[MaterialIndex];
            mesh_material *Material = Asset->Materials + MaterialIndex;

            ProcessAssimpMaterial(AssimpScene, AssimpMaterial, Material);
        }
    }

#if 0
    if (AssimpScene->HasAnimations())
    {
        Asset->AnimationCount = AssimpScene->mNumAnimations;
        Asset->Animations = AllocateMemory<animation_clip>(Asset->AnimationCount);

        for (u32 AnimationIndex = 0; AnimationIndex < Asset->AnimationCount; ++AnimationIndex)
        {
            aiAnimation *AssimpAnimation = AssimpScene->mAnimations[AnimationIndex];
            animation_clip *Animation = Asset->Animations + AnimationIndex;

            ProcessAssimpAnimation(AssimpAnimation, Animation, &Asset->Skeleton);
        }
    }
#endif
}
