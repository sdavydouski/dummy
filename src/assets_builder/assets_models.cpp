#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "meshoptimizer.h"
#include "indexgenerator.cpp"
#include "vcacheoptimizer.cpp"
#include "overdrawoptimizer.cpp"
#include "vfetchoptimizer.cpp"
#include "simplifier.cpp"

#include "rapidjson/document.h"

#include "dummy.h"
#include "assets.h"

#include "dummy_bounds.cpp"

namespace json = rapidjson;

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

    // If the texture is embedded, TexturePath receives a '*' followed by the id of
    // the texture (for the textures stored in the corresponding scene)
    if (TexturePath.data[0] == '*')
    {
        u32 TextureIndex = std::atoi(TexturePath.C_Str() + 1);

        Assert(TextureIndex < AssimpScene->mNumTextures);

        Result = AssimpScene->mTextures[TextureIndex];
    }
    else
    {
        for (u32 TextureIndex = 0; TextureIndex < AssimpScene->mNumTextures; ++TextureIndex)
        {
            aiTexture *AssimpTexture = AssimpScene->mTextures[TextureIndex];

            if (AssimpTexture->mFilename == TexturePath)
            {
                Result = AssimpTexture;
                break;
            }
        }
    }

    Assert(Result);

    return Result;
}

dummy_internal void
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

dummy_internal void
ProcessAssimpMaterial(const aiScene *AssimpScene, aiMaterial *AssimpMaterial, mesh_material *Material)
{
    Material->Properties = AllocateMemory<material_property>(MAX_MATERIAL_PROPERTY_COUNT);
    u32 MaterialPropertyIndex = 0;

    i32 ShadingModel;
    aiGetMaterialInteger(AssimpMaterial, AI_MATKEY_SHADING_MODEL, &ShadingModel);

    switch (ShadingModel)
    {
        case aiShadingMode_Phong:
        case aiShadingMode_Blinn:
        {
            Material->ShadingMode = ShadingMode_Phong;

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

            break;
        }
        case aiShadingMode_PBR_BRDF:
        {
            Material->ShadingMode = ShadingMode_PBR;

            f32 Metalness;
            if (aiGetMaterialFloat(AssimpMaterial, AI_MATKEY_METALLIC_FACTOR, &Metalness) == AI_SUCCESS)
            {
                material_property *MaterialProperty = Material->Properties + MaterialPropertyIndex;
                MaterialProperty->Type = MaterialProperty_Float_Metalness;
                MaterialProperty->Value = Metalness;

                ++MaterialPropertyIndex;
            }

            f32 Roughness;
            if (aiGetMaterialFloat(AssimpMaterial, AI_MATKEY_ROUGHNESS_FACTOR, &Roughness) == AI_SUCCESS)
            {
                material_property *MaterialProperty = Material->Properties + MaterialPropertyIndex;
                MaterialProperty->Type = MaterialProperty_Float_Roughness;
                MaterialProperty->Value = Roughness;

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

            u32 AlbedoMapCount = 0;
            bitmap *AlbedoMaps = 0;
            ProcessAssimpTextures(AssimpScene, AssimpMaterial, aiTextureType_BASE_COLOR, &AlbedoMapCount, &AlbedoMaps);
            for (u32 AlbedoMapIndex = 0; AlbedoMapIndex < AlbedoMapCount; ++AlbedoMapIndex)
            {
                bitmap *AlbedoMap = AlbedoMaps + AlbedoMapIndex;

                material_property *MaterialProperty = Material->Properties + MaterialPropertyIndex;
                MaterialProperty->Type = MaterialProperty_Texture_Albedo;
                MaterialProperty->Bitmap = *AlbedoMap;

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

            u32 RoughnessMapCount = 0;
            bitmap *RoughnessMaps = 0;
            ProcessAssimpTextures(AssimpScene, AssimpMaterial, aiTextureType_DIFFUSE_ROUGHNESS, &RoughnessMapCount, &RoughnessMaps);
            for (u32 RoughnessMapIndex = 0; RoughnessMapIndex < RoughnessMapCount; ++RoughnessMapIndex)
            {
                bitmap *RoughnessMap = RoughnessMaps + RoughnessMapIndex;

                material_property *MaterialProperty = Material->Properties + MaterialPropertyIndex;
                MaterialProperty->Type = MaterialProperty_Texture_Roughness;
                MaterialProperty->Bitmap = *RoughnessMap;

                ++MaterialPropertyIndex;
            }

            u32 AmbientOcclusionMapCount = 0;
            bitmap *AmbientOcclusionMaps = 0;
            ProcessAssimpTextures(AssimpScene, AssimpMaterial, aiTextureType_AMBIENT_OCCLUSION, &AmbientOcclusionMapCount, &AmbientOcclusionMaps);
            for (u32 AmbientOcclusionMapIndex = 0; AmbientOcclusionMapIndex < AmbientOcclusionMapCount; ++AmbientOcclusionMapIndex)
            {
                //bitmap *AmbientOcclusionMap = AmbientOcclusionMaps + AmbientOcclusionMapIndex;

                //material_property *MaterialProperty = Material->Properties + MaterialPropertyIndex;
                //MaterialProperty->Type = MaterialProperty_Texture_AmbientOcclusion;
                //MaterialProperty->Bitmap = *AmbientOcclusionMap;

                //++MaterialPropertyIndex;
            }

            break;
        }
        default:
        {
            Material->ShadingMode = ShadingMode_Flat;

            break;
        }
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

dummy_internal void
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

dummy_internal void
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

dummy_internal void
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

dummy_internal aiNode *
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

dummy_internal void
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
dummy_internal mat4
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

dummy_internal obb
CalculateOrientedBoundingBox(u32 MeshCount, mesh *Meshes)
{
    // 1. Calculate obb for each mesh
    // 2. Extract vertices from each obb
    // 3. Calculate final obb from these vertices

    obb *Boxes = AllocateMemory<obb>(MeshCount);

    u32 VertexCount = MeshCount * 8;
    vec3 *Vertices = AllocateMemory<vec3>(VertexCount);

    for (u32 MeshIndex = 0; MeshIndex < MeshCount; ++MeshIndex)
    {
        mesh *Mesh = Meshes + MeshIndex;
        obb *Box = Boxes + MeshIndex;

        *Box = CalculateOrientedBoundingBox(Mesh->VertexCount, Mesh->Positions);
        CalculateVertices(*Box, Vertices + MeshIndex * 8);
    }
    
    obb Result = CalculateOrientedBoundingBox(VertexCount, Vertices);

    return Result;
}

dummy_internal void
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

dummy_internal void
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


// For testing
dummy_internal u32
ReadAnimationGraph(animation_graph_asset *GraphAsset, u64 Offset, u8 *Buffer)
{
    model_asset_animation_graph_header *AnimationGraphHeader = (model_asset_animation_graph_header *) (Buffer + Offset);
    CopyString(AnimationGraphHeader->Name, GraphAsset->Name);
    CopyString(AnimationGraphHeader->Entry, GraphAsset->Entry);
    CopyString(AnimationGraphHeader->Animator, GraphAsset->Animator);
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

        NodeAsset->AdditiveAnimationCount = NodeHeader->AdditiveAnimationCount;
        NodeAsset->AdditiveAnimations = (additive_animation_asset *)(Buffer + NodeHeader->AdditiveAnimationsOffset);

        switch (NodeAsset->Type)
        {
            case AnimationNodeType_Clip:
            {
                model_asset_animation_state_header *AnimationStateHeader = (model_asset_animation_state_header *) (Buffer + NodeHeader->Offset);

                NodeAsset->Animation = AllocateMemory<animation_state_asset>();
                CopyString(AnimationStateHeader->AnimationClipName, NodeAsset->Animation->AnimationClipName);
                NodeAsset->Animation->IsLooping = AnimationStateHeader->IsLooping;
                NodeAsset->Animation->EnableRootMotion = AnimationStateHeader->EnableRootMotion;

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
            case AnimationNodeType_Reference:
            {
                model_asset_animation_reference_header *AnimationReferenceHeader = (model_asset_animation_reference_header *)(Buffer + NodeHeader->Offset);

                NodeAsset->Reference = AllocateMemory<animation_reference_asset>();
                CopyString(AnimationReferenceHeader->NodeName, NodeAsset->Reference->NodeName);

                TotalPrevNodeSize += sizeof(model_asset_animation_reference_header);
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

        TotalPrevNodeSize += sizeof(model_asset_animation_node_header) + NodeHeader->TransitionCount * sizeof(animation_transition_asset) + NodeHeader->AdditiveAnimationCount * sizeof(additive_animation_asset);
    }

    return TotalPrevNodeSize;
}

// For testing
dummy_internal void
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
    BindPose.LocalJointPoses = (transform *) ((u8 *) Buffer + SkeletonPoseHeader->LocalJointPosesOffset);
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
            Mesh.JointIndices = (ivec4 *) (Buffer + MeshHeader->VerticesOffset + VerticesOffset);
            VerticesOffset += sizeof(ivec4) * MeshHeader->VertexCount;
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
        Material.ShadingMode = MaterialHeader->ShadingMode;
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
                case MaterialProperty_Texture_Albedo:
                {
                    MaterialProperty->Bitmap = MaterialPropertyHeader->Bitmap;
                    MaterialProperty->Bitmap.Pixels = (void *)(Buffer + MaterialPropertyHeader->BitmapOffset);

#if 0
                    char FileName[64];
                    FormatString(FileName, "Albedo - %d - %d.bmp", MaterialIndex, MaterialPropertyIndex);

                    stbi_write_bmp(FileName, MaterialProperty->Bitmap.Width, MaterialProperty->Bitmap.Height, MaterialProperty->Bitmap.Channels, MaterialProperty->Bitmap.Pixels);
#endif

                    u32 BitmapSize = MaterialProperty->Bitmap.Width * MaterialProperty->Bitmap.Height * MaterialProperty->Bitmap.Channels;

                    NextMaterialPropertyHeaderOffset += sizeof(model_asset_material_property_header) + BitmapSize;

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
                case MaterialProperty_Texture_Metalness:
                {
                    MaterialProperty->Bitmap = MaterialPropertyHeader->Bitmap;
                    MaterialProperty->Bitmap.Pixels = (void *)(Buffer + MaterialPropertyHeader->BitmapOffset);

#if 0
                    char FileName[64];
                    FormatString(FileName, "Metalness - %d - %d.bmp", MaterialIndex, MaterialPropertyIndex);

                    stbi_write_bmp(FileName, MaterialProperty->Bitmap.Width, MaterialProperty->Bitmap.Height, MaterialProperty->Bitmap.Channels, MaterialProperty->Bitmap.Pixels);
#endif

                    u32 BitmapSize = MaterialProperty->Bitmap.Width * MaterialProperty->Bitmap.Height * MaterialProperty->Bitmap.Channels;

                    NextMaterialPropertyHeaderOffset += sizeof(model_asset_material_property_header) + BitmapSize;

                    break;
                }
                case MaterialProperty_Texture_Roughness:
                {
                    MaterialProperty->Bitmap = MaterialPropertyHeader->Bitmap;
                    MaterialProperty->Bitmap.Pixels = (void *)(Buffer + MaterialPropertyHeader->BitmapOffset);

#if 0
                    char FileName[64];
                    FormatString(FileName, "Metalness - %d - %d.bmp", MaterialIndex, MaterialPropertyIndex);

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

        Animation.EventCount = AnimationHeader->EventCount;
        Animation.Events = (animation_event *) (Buffer + AnimationHeader->EventsOffset);

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

        NextAnimationHeaderOffset += sizeof(model_asset_animation_header) + NextAnimationSampleHeaderOffset + AnimationHeader->EventCount * sizeof(animation_event);
    }

    fclose(AssetFile);
}

dummy_internal u32
WriteAnimationGraph(animation_graph_asset *AnimationGraph, u64 Offset, FILE *AssetFile)
{
    model_asset_animation_graph_header AnimationGraphHeader = {};
    CopyString(AnimationGraph->Name, AnimationGraphHeader.Name);
    CopyString(AnimationGraph->Entry, AnimationGraphHeader.Entry);
    CopyString(AnimationGraph->Animator, AnimationGraphHeader.Animator);
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

        NodeHeader.AdditiveAnimationCount = Node->AdditiveAnimationCount;
        NodeHeader.AdditiveAnimationsOffset = NodeHeader.TransitionsOffset + NodeHeader.TransitionCount * sizeof(animation_transition_asset);

        NodeHeader.Offset = NodeHeader.AdditiveAnimationsOffset + NodeHeader.AdditiveAnimationCount * sizeof(additive_animation_asset);

        fwrite(&NodeHeader, sizeof(model_asset_animation_node_header), 1, AssetFile);
        fwrite(Node->Transitions, sizeof(animation_transition_asset), Node->TransitionCount, AssetFile);
        fwrite(Node->AdditiveAnimations, sizeof(additive_animation_asset), Node->AdditiveAnimationCount, AssetFile);

        switch (NodeHeader.Type)
        {
            case AnimationNodeType_Clip:
            {
                model_asset_animation_state_header AnimationStateHeader = {};
                CopyString(Node->Animation->AnimationClipName, AnimationStateHeader.AnimationClipName);
                AnimationStateHeader.IsLooping = Node->Animation->IsLooping;
                AnimationStateHeader.EnableRootMotion = Node->Animation->EnableRootMotion;

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
            case AnimationNodeType_Reference:
            {
                model_asset_animation_reference_header AnimationReferenceHeader = {};

                CopyString(Node->Reference->NodeName, AnimationReferenceHeader.NodeName);

                fwrite(&AnimationReferenceHeader, sizeof(model_asset_animation_reference_header), 1, AssetFile);

                TotalPrevNodeSize += sizeof(model_asset_animation_reference_header);
                break;
            }
            case AnimationNodeType_Graph:
            {
                u32 NodeSize = WriteAnimationGraph(Node->Graph, NodeHeader.Offset, AssetFile);

                TotalPrevNodeSize += NodeSize + sizeof(model_asset_animation_graph_header);
                break;
            }
        }

        TotalPrevNodeSize += sizeof(model_asset_animation_node_header) + NodeHeader.TransitionCount * sizeof(animation_transition_asset) + NodeHeader.AdditiveAnimationCount * sizeof(additive_animation_asset);
    }

    return TotalPrevNodeSize;
}

dummy_internal void
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
            fwrite(Mesh->JointIndices, sizeof(ivec4), Mesh->VertexCount, AssetFile);
        }

        fwrite(Mesh->Indices, sizeof(u32), Mesh->IndexCount, AssetFile);
    }
}

dummy_internal void
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
        MaterialHeader.ShadingMode = Material->ShadingMode;
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
                case MaterialProperty_Float_Metalness:
                case MaterialProperty_Float_Roughness:
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
                case MaterialProperty_Texture_Albedo:
                case MaterialProperty_Texture_Metalness:
                case MaterialProperty_Texture_Roughness:
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

dummy_internal void
WriteAnimationClips(model_asset *Asset, u64 Offset, FILE *AssetFile)
{
    model_asset_animations_header AnimationsHeader = {};
    AnimationsHeader.AnimationCount = Asset->AnimationCount;
    AnimationsHeader.AnimationsOffset = Offset + sizeof(model_asset_animations_header);

    fwrite(&AnimationsHeader, sizeof(model_asset_animations_header), 1, AssetFile);

    for (u32 AnimationIndex = 0; AnimationIndex < Asset->AnimationCount; ++AnimationIndex)
    {
        animation_clip *Animation = Asset->Animations + AnimationIndex;

        u64 AnimationHeaderStreamPosition = ftell(AssetFile);
        u64 CurrentStreamPosition = ftell(AssetFile);

        model_asset_animation_header AnimationHeader = {};
        CopyString(Animation->Name, AnimationHeader.Name);
        AnimationHeader.Duration = Animation->Duration;

        fwrite(&AnimationHeader, sizeof(model_asset_animation_header), 1, AssetFile);

        CurrentStreamPosition = ftell(AssetFile);

        AnimationHeader.PoseSampleCount = Animation->PoseSampleCount;
        AnimationHeader.PoseSamplesOffset = CurrentStreamPosition;

        for (u32 AnimationPoseIndex = 0; AnimationPoseIndex < Animation->PoseSampleCount; ++AnimationPoseIndex)
        {
            animation_sample *AnimationPose = Animation->PoseSamples + AnimationPoseIndex;

            u64 PoseSampleHeaderStreamPosition = ftell(AssetFile);

            model_asset_animation_sample_header AnimationSampleHeader = {};
            AnimationSampleHeader.JointIndex = AnimationPose->JointIndex;
            
            fwrite(&AnimationSampleHeader, sizeof(model_asset_animation_sample_header), 1, AssetFile);

            CurrentStreamPosition = ftell(AssetFile);
            AnimationSampleHeader.KeyFrameCount = AnimationPose->KeyFrameCount;
            AnimationSampleHeader.KeyFramesOffset = CurrentStreamPosition;

            fwrite(AnimationPose->KeyFrames, sizeof(key_frame), AnimationPose->KeyFrameCount, AssetFile);

            u64 BeforeSeekStreamPosition = ftell(AssetFile);
            fseek(AssetFile, (long ) PoseSampleHeaderStreamPosition, SEEK_SET);
            fwrite(&AnimationSampleHeader, sizeof(model_asset_animation_sample_header), 1, AssetFile);
            fseek(AssetFile, (long) BeforeSeekStreamPosition, SEEK_SET);
        }

        CurrentStreamPosition = ftell(AssetFile);

        AnimationHeader.EventCount = Animation->EventCount;
        AnimationHeader.EventsOffset = CurrentStreamPosition;

        fwrite(Animation->Events, sizeof(animation_event), Animation->EventCount, AssetFile);
        
        u64 BeforeSeekStreamPosition = ftell(AssetFile);
        fseek(AssetFile, (long) AnimationHeaderStreamPosition, SEEK_SET);
        fwrite(&AnimationHeader, sizeof(model_asset_animation_header), 1, AssetFile);
        fseek(AssetFile, (long) BeforeSeekStreamPosition, SEEK_SET);
    }
}

dummy_internal void
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
    ModelAssetHeader.Bounds = Asset->Bounds;
    ModelAssetHeader.BoundsOBB = Asset->BoundsOBB;

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
    SkeletonPoseHeader.GlobalJointPosesOffset = SkeletonPoseHeader.LocalJointPosesOffset + SkeletonHeader.JointCount * sizeof(transform);

    fwrite(&SkeletonPoseHeader, sizeof(model_asset_skeleton_pose_header), 1, AssetFile);
    fwrite(Asset->BindPose.LocalJointPoses, sizeof(transform), Asset->Skeleton.JointCount, AssetFile);
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

dummy_internal void
LoadModelAsset(const char *FilePath, model_asset *Asset, u32 Flags)
{
    aiPropertyStore *AssimpPropertyStore = aiCreatePropertyStore();
    aiSetImportPropertyInteger(AssimpPropertyStore, AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, 0);
    aiSetImportPropertyFloat(AssimpPropertyStore, AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 1.f);

    const aiScene *AssimpScene = aiImportFileExWithProperties(FilePath, Flags, 0, AssimpPropertyStore);

    if (AssimpScene)
    {
        *Asset = {};

        ProcessAssimpScene(AssimpScene, Asset);

        Asset->Bounds = CalculateAxisAlignedBoundingBox(Asset->MeshCount, Asset->Meshes);
        Asset->BoundsOBB = CalculateOrientedBoundingBox(Asset->MeshCount, Asset->Meshes);

        aiReleaseImport(AssimpScene);
    }
    else
    {
        const char *ErrorMessage = aiGetErrorString();
        Assert(!ErrorMessage);
    }

    aiReleasePropertyStore(AssimpPropertyStore);
}

dummy_internal void
LoadAnimationClipAsset(const char *FilePath, u32 Flags, model_asset *Asset, animation_clip *Animation)
{
    aiPropertyStore *AssimpPropertyStore = aiCreatePropertyStore();

    const aiScene *AssimpScene = aiImportFileExWithProperties(FilePath, Flags, 0, AssimpPropertyStore);

    Assert(AssimpScene);
    Assert(AssimpScene->mNumAnimations == 1);

    aiAnimation *AssimpAnimation = AssimpScene->mAnimations[0];

    ProcessAssimpAnimation(AssimpAnimation, Animation, &Asset->Skeleton);

    aiReleaseImport(AssimpScene);
    aiReleasePropertyStore(AssimpPropertyStore);
}

dummy_internal void
LoadAnimationMetaAsset(const char *FilePath, animation_clip *Animation)
{
    char *Contents = ReadTextFile(FilePath);

    if (Contents)
    {
        json::Document Document;
        json::ParseResult ok = Document.Parse(Contents);

        if (ok)
        {
            json::Value &Events = Document["events"].GetArray();

            Animation->EventCount = Events.Size();
            Animation->Events = AllocateMemory<animation_event>(Animation->EventCount);

            for (u32 EventIndex = 0; EventIndex < Animation->EventCount; ++EventIndex)
            {
                animation_event *AnimationEvent = Animation->Events + EventIndex;

                json::Value &Event = Events[EventIndex];

                const char *Name = Event["name"].GetString();
                f32 Time = Event["time"].GetFloat();

                CopyString(Name, AnimationEvent->Name);
                AnimationEvent->Time = Time;
                AnimationEvent->IsFired = false;
            }
        }
        else
        {
            Assert(!"Failed to parse animation meta");
        }
    }
}

dummy_internal void
ProcessGraphNodes(animation_graph_asset *GraphAsset, json::Value &Nodes)
{
    GraphAsset->NodeCount = Nodes.Size();
    GraphAsset->Nodes = AllocateMemory<animation_node_asset>(GraphAsset->NodeCount);

    for (u32 NodeIndex = 0; NodeIndex < GraphAsset->NodeCount; ++NodeIndex)
    {
        animation_node_asset *NodeAsset = GraphAsset->Nodes + NodeIndex;

        json::Value &Node = Nodes[NodeIndex];

        const char *Name = Node["name"].GetString();
        const char *Type = Node["type"].GetString();

        CopyString(Name, NodeAsset->Name);

        // Transitions
        if (Node.HasMember("transitions"))
        {
            json::Value &Transitions = Node["transitions"].GetArray();

            NodeAsset->TransitionCount = Transitions.Size();
            NodeAsset->Transitions = AllocateMemory<animation_transition_asset>(NodeAsset->TransitionCount);

            for (u32 TransitionIndex = 0; TransitionIndex < NodeAsset->TransitionCount; ++TransitionIndex)
            {
                animation_transition_asset *TransitionAsset = NodeAsset->Transitions + TransitionIndex;

                json::Value &TransitionValue = Transitions[TransitionIndex];

                const char *From = Name;
                const char *To = TransitionValue["to"].GetString();

                CopyString(From, TransitionAsset->From);
                CopyString(To, TransitionAsset->To);

                const char *TransitionType = TransitionValue["type"].GetString();

                if (StringEquals(TransitionType, "transitional"))
                {
                    TransitionAsset->Type = AnimationTransitionType_Transitional;

                    const char *TransitionNode = TransitionValue["through"].GetString();
                    CopyString(TransitionNode, TransitionAsset->TransitionNode);

                    TransitionAsset->Duration = TransitionValue["blend"].GetFloat();
                }
                else if (StringEquals(TransitionType, "crossfade"))
                {
                    TransitionAsset->Type = AnimationTransitionType_Crossfade;
                    TransitionAsset->Duration = TransitionValue["blend"].GetFloat();
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
        }
        else
        {
            NodeAsset->TransitionCount = 0;
            NodeAsset->Transitions = 0;
        }

        // Additive animations
        if (Node.HasMember("additive"))
        {
            json::Value &AdditiveAnimations = Node["additive"].GetArray();

            NodeAsset->AdditiveAnimationCount = AdditiveAnimations.Size();
            NodeAsset->AdditiveAnimations = AllocateMemory<additive_animation_asset>(NodeAsset->AdditiveAnimationCount);

            for (u32 AdditiveAnimationIndex = 0; AdditiveAnimationIndex < NodeAsset->AdditiveAnimationCount; ++AdditiveAnimationIndex)
            {
                additive_animation_asset *AdditiveAsset = NodeAsset->AdditiveAnimations + AdditiveAnimationIndex;

                json::Value &AdditiveValue = AdditiveAnimations[AdditiveAnimationIndex];

                const char *Target = AdditiveValue["target"].GetString();
                const char *Base = AdditiveValue["base"].GetString();

                CopyString(Target, AdditiveAsset->TargetClipName);
                CopyString(Base, AdditiveAsset->BaseClipName);

                AdditiveAsset->BaseKeyFrameIndex = AdditiveValue["base_keyframe_index"].GetUint();
                AdditiveAsset->IsLooping = AdditiveValue["looping"].GetBool();
            }
        }
        else
        {
            NodeAsset->AdditiveAnimationCount = 0;
            NodeAsset->AdditiveAnimations = 0;
        }

        // Nodes
        if (StringEquals(Type, "Graph"))
        {
            NodeAsset->Type = AnimationNodeType_Graph;
            NodeAsset->Graph = AllocateMemory<animation_graph_asset>();

            const char *Name = Node["name"].GetString();
            const char *Entry = Node["entry"].GetString();
            json::Value &SubNodes = Node["nodes"].GetArray();

            CopyString(Name, NodeAsset->Graph->Name);
            CopyString(Entry, NodeAsset->Graph->Entry);
            CopyString("", NodeAsset->Graph->Animator);

            ProcessGraphNodes(NodeAsset->Graph, SubNodes);
        }
        else if (StringEquals(Type, "Blendspace"))
        {
            NodeAsset->Type = AnimationNodeType_BlendSpace;
            NodeAsset->Blendspace = AllocateMemory<blend_space_1d_asset>();

            json::Value &Values = Node["values"].GetArray();

            NodeAsset->Blendspace->ValueCount = Values.Size();
            NodeAsset->Blendspace->Values = AllocateMemory<blend_space_1d_value_asset>(NodeAsset->Blendspace->ValueCount);

            for (u32 ValueIndex = 0; ValueIndex < NodeAsset->Blendspace->ValueCount; ++ValueIndex)
            {
                blend_space_1d_value_asset *ValueAsset = NodeAsset->Blendspace->Values + ValueIndex;

                json::Value &BlendspaceValue = Values[ValueIndex];

                const char *Clip = BlendspaceValue["clip"].GetString();

                CopyString(Clip, ValueAsset->AnimationClipName);
                ValueAsset->Value = BlendspaceValue["value"].GetFloat();
                ValueAsset->EnableRootMotion = BlendspaceValue["root_motion"].GetBool();
            }
        }
        else if (StringEquals(Type, "Reference"))
        {
            NodeAsset->Type = AnimationNodeType_Reference;
            NodeAsset->Reference = AllocateMemory<animation_reference_asset>();

            const char *NodeName = Node["node"].GetString();
            CopyString(NodeName, NodeAsset->Reference->NodeName);
        }
        else if (StringEquals(Type, "Animation"))
        {
            NodeAsset->Type = AnimationNodeType_Clip;
            NodeAsset->Animation = AllocateMemory<animation_state_asset>();

            bool32 IsLooping = Node["looping"].GetBool();
            bool32 EnableRootMotion = Node["root_motion"].GetBool();
            
            NodeAsset->Animation->IsLooping = IsLooping;
            NodeAsset->Animation->EnableRootMotion = EnableRootMotion;
            
            const char *Clip = Node["clip"].GetString();
            CopyString(Clip, NodeAsset->Animation->AnimationClipName);
        }
        else
        {
            Assert(!"Unknown node type");
        }
    }
}

dummy_internal void
LoadAnimationGrah(const char *FilePath, model_asset *Asset)
{
    char *Contents = ReadTextFile(FilePath);

    if (Contents)
    {
        json::Document Document;
        json::ParseResult ok = Document.Parse(Contents);

        if (ok)
        {
            animation_graph_asset *GraphAsset = &Asset->AnimationGraph;

            const char *Version = Document["version"].GetString();
            const char *Animator = Document["animator"].GetString();
            json::Value &Graph = Document["graph"];

            const char *Name = Graph["name"].GetString();
            const char *Entry = Graph["entry"].GetString();
            json::Value &Nodes = Graph["nodes"].GetArray();

            CopyString(Name, GraphAsset->Name);
            CopyString(Entry, GraphAsset->Entry);
            CopyString(Animator, GraphAsset->Animator);

            ProcessGraphNodes(GraphAsset, Nodes);
        }
        else
        {
            Assert(!"Failed to parse animation graph");
        }
    }
}

dummy_internal void
LoadAnimationClips(const char *DirectoryPath, u32 Flags, model_asset *Asset)
{
    if (fs::exists(DirectoryPath))
    {
        u32 AnimationCount = 0;

        for (const fs::directory_entry &Entry : fs::directory_iterator(DirectoryPath))
        {
            if (Entry.is_regular_file() && Entry.path().extension() == ".fbx")
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
                fs::path FileExtension = Entry.path().extension();

                if (FileExtension == ".fbx")
                {
                    fs::path AnimationClipName = Entry.path().stem();

                    animation_clip *Animation = Asset->Animations + AnimationIndex++;
                    CopyString(AnimationClipName.generic_string().c_str(), Animation->Name);

                    char AnimationClipFilePath[256];
                    FormatString(AnimationClipFilePath, "%s/%s", DirectoryPath, FileName.generic_string().c_str());

                    LoadAnimationClipAsset(AnimationClipFilePath, Flags, Asset, Animation);

                    fs::path AnimationMetaFileName = FileName;
                    AnimationMetaFileName.replace_extension(".json");

                    char AnimationMetaFilePath[256];
                    FormatString(AnimationMetaFilePath, "%s/%s", DirectoryPath, AnimationMetaFileName.generic_string().c_str());

                    if (fs::exists(AnimationMetaFilePath))
                    {
                        LoadAnimationMetaAsset(AnimationMetaFilePath, Animation);
                    }
                    else
                    {
                        Animation->EventCount = 0;
                        Animation->Events = 0;
                    }
                }
                else if (FileExtension == ".json")
                {
                    // skip
                }
                else
                {
                    Assert(!"Invalid extension");
                }
            }
        }
    }
}

dummy_internal void
OptimizeModelAsset(model_asset *Asset)
{
    for (u32 MeshIndex = 0; MeshIndex < Asset->MeshCount; ++MeshIndex)
    {
        mesh *Mesh = Asset->Meshes + MeshIndex;

        f32 Threshold = 0.2f;
        f32 TargetError = 0.002f;
        u32 TargetIndexCount = (u32) (Mesh->IndexCount * Threshold);
        u32 *TargetIndices = AllocateMemory<u32>(Mesh->IndexCount);
        
        u32 NewIndexCount = (u32) meshopt_simplify(TargetIndices, Mesh->Indices, Mesh->IndexCount, (f32 *) Mesh->Positions, Mesh->VertexCount, sizeof(vec3), TargetIndexCount, TargetError);

        Mesh->IndexCount = NewIndexCount;
        Mesh->Indices = TargetIndices;

        meshopt_Stream Streams[7] = {};

        u32 StreamCount = 0;

        if (Mesh->Positions)
        {
            Streams[StreamCount++] = { Mesh->Positions, sizeof(vec3), sizeof(vec3) };
        }

        if (Mesh->Normals)
        {
            Streams[StreamCount++] = { Mesh->Normals, sizeof(vec3), sizeof(vec3) };
        }

        if (Mesh->Tangents)
        {
            Streams[StreamCount++] = { Mesh->Tangents, sizeof(vec3), sizeof(vec3) };
        }

        if (Mesh->Bitangents)
        {
            Streams[StreamCount++] = { Mesh->Bitangents, sizeof(vec3), sizeof(vec3) };
        }

        if (Mesh->TextureCoords)
        {
            Streams[StreamCount++] = { Mesh->TextureCoords, sizeof(vec2), sizeof(vec2) };
        }

        if (Mesh->Weights)
        {
            Streams[StreamCount++] = { Mesh->Weights, sizeof(vec4), sizeof(vec4) };
        }

        if (Mesh->JointIndices)
        {
            Streams[StreamCount++] = { Mesh->JointIndices, sizeof(ivec4), sizeof(ivec4) };
        }

        u32 *RemapTable = AllocateMemory<u32>(Mesh->VertexCount);
        meshopt_generateVertexRemapMulti(RemapTable, Mesh->Indices, Mesh->IndexCount, Mesh->VertexCount, Streams, StreamCount);

        meshopt_optimizeVertexCache(Mesh->Indices, Mesh->Indices, Mesh->IndexCount, Mesh->VertexCount);
        meshopt_optimizeOverdraw(Mesh->Indices, Mesh->Indices, Mesh->IndexCount, (f32 *) Mesh->Positions, Mesh->VertexCount, sizeof(vec3), 1.05f);

        u32 NewVertexCount = (u32) meshopt_optimizeVertexFetchRemap(RemapTable, Mesh->Indices, Mesh->IndexCount, Mesh->VertexCount);

        meshopt_remapIndexBuffer(Mesh->Indices, Mesh->Indices, Mesh->IndexCount, RemapTable);

        for (u32 StreamIndex = 0; StreamIndex < StreamCount; ++StreamIndex)
        {
            meshopt_Stream Stream = Streams[StreamIndex];

            void *Data = (void *) AllocateMemory<u8>(NewVertexCount * Stream.size);

            meshopt_remapVertexBuffer(Data, Stream.data, Mesh->VertexCount, Stream.size, RemapTable);

            switch (StreamIndex)
            {
                case 0:
                {
                    Mesh->Positions = (vec3 *) Data;
                    break;
                }
                case 1:
                {
                    Mesh->Normals = (vec3 *) Data;
                    break;
                }
                case 2:
                {
                    Mesh->Tangents = (vec3 *) Data;
                    break;
                }
                case 3:
                {
                    Mesh->Bitangents = (vec3 *) Data;
                    break;
                }
                case 4:
                {
                    Mesh->TextureCoords = (vec2 *) Data;
                    break;
                }
                case 5:
                {
                    Mesh->Weights = (vec4 *) Data;
                    break;
                }
                case 6:
                {
                    Mesh->JointIndices = (ivec4 *) Data;
                    break;
                }
                default:
                {
                    Assert(!"Invalid stream index");
                    break;
                }
            }
        }

        Mesh->VertexCount = NewVertexCount;
    }
}

dummy_internal void
ProcessModelAsset(const char *FilePath, const char *AnimationConfigPath, const char *AnimationClipsPath, const char *OutputPath)
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
        aiProcess_OptimizeGraph |
        aiProcess_OptimizeMeshes |
        aiProcess_GlobalScale;

    model_asset Asset;

    LoadModelAsset(FilePath, &Asset, Flags);
    LoadAnimationGrah(AnimationConfigPath, &Asset);
    LoadAnimationClips(AnimationClipsPath, Flags, &Asset);

    //OptimizeModelAsset(&Asset);

    WriteModelAsset(OutputPath, &Asset);

#if 1
    model_asset TestAsset = {};
    ReadModelAsset(OutputPath, &TestAsset, &Asset);
#endif
}

dummy_internal void
ProcessModelAsset(const char *FilePath, const char *OutputPath)
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
        aiProcess_OptimizeGraph |
        aiProcess_OptimizeMeshes;

    model_asset Asset;

    LoadModelAsset(FilePath, &Asset, Flags);

    OptimizeModelAsset(&Asset);

    WriteModelAsset(OutputPath, &Asset);

#if 1
    model_asset TestAsset = {};
    ReadModelAsset(OutputPath, &TestAsset, &Asset);
#endif
}
