#define _CRT_SECURE_NO_WARNINGS

#include "handmade_defs.h"
#include "handmade_math.h"
#include "handmade_renderer.h"
#include "handmade_assets.h"

#undef PI

#include <assimp/cimport.h>        // Plain-C interface
#include <assimp/scene.h>          // Output data structure
#include <assimp/postprocess.h>    // Post processing flags

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <string>
#include <vector>
#include <unordered_map>

using std::string;

template <typename TValue>
using dynamic_array = std::vector<TValue>;

template <typename TKey, typename TValue>
using hashtable = std::unordered_map<TKey, TValue>;

struct assimp_node
{
    aiNode *Node;
    aiBone *Bone;
};

#define INVALID_FLOAT -1.f
#define INVALID_COLOR vec4(-1.f)

#define MAX_MATERIAL_PROPERTY_COUNT 16u
#define TEXTURE_COORDINATES_SET_INDEX 0

#define MAX_WEIGHT_COUNT 4

inline b32
StringEquals(const char *Str1, const char *Str2) {
    b32 Result = strcmp(Str1, Str2) == 0;
    return Result;
}

inline void
CopyString(const char *Source, char *Dest, u32 Length)
{
    for (u32 Index = 0; Index < Length; ++Index)
    {
        Dest[Index] = Source[Index];
    }

    Dest[Length] = 0;
}

inline void
CopyString(string Source, char *Dest)
{
    CopyString(Source.c_str(), Dest, (u32)Source.length());
}

inline void
GetDirectoryPath(char *FilePath, char *DirectoryPath)
{
    char *LastDelimiter = FilePath;

    for (char *Scan = FilePath; *Scan; ++Scan)
    {
        if (*Scan == '\\')
        {
            LastDelimiter = Scan + 1;
        }
    }

    CopyString(FilePath, DirectoryPath, (u32)(LastDelimiter - FilePath));
}

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

inline void
LoadImage(const char *FilePath, bitmap *Bitmap)
{
    Bitmap->Pixels = stbi_load(FilePath, &Bitmap->Width, &Bitmap->Height, &Bitmap->Channels, 0);
}

void ProcessAssimpTextures(
    aiMaterial *AssimpMaterial,
    aiTextureType AssimpTextureType,
    const char *DirectoryPath,
    u32 *TextureCount,
    bitmap **Bitmaps
    )
{
    *TextureCount = aiGetMaterialTextureCount(AssimpMaterial, AssimpTextureType);
    *Bitmaps = (bitmap *)malloc(*TextureCount * sizeof(bitmap));

    for (u32 TextureIndex = 0; TextureIndex < *TextureCount; ++TextureIndex)
    {
        aiString TexturePath;
        if (aiGetMaterialTexture(AssimpMaterial, AssimpTextureType, TextureIndex, &TexturePath) == AI_SUCCESS)
        {
            bitmap *Bitmap = *Bitmaps + TextureIndex;

            string FullPath = string(DirectoryPath) + AssimpString2StdString(TexturePath);
            LoadImage(FullPath.c_str(), Bitmap);
        }
    }
}

void ProcessAssimpMaterial(aiMaterial *AssimpMaterial, const char *DirectoryPath, material_asset *Material)
{
    Material->Properties = (material_property *)malloc(MAX_MATERIAL_PROPERTY_COUNT * sizeof(material_property));
    u32 MaterialPropertyIndex = 0;

    f32 SpecularShininess;
    if (aiGetMaterialFloat(AssimpMaterial, AI_MATKEY_SHININESS, &SpecularShininess) == AI_SUCCESS)
    {
        material_property *MaterialProperty = Material->Properties + MaterialPropertyIndex;
        MaterialProperty->Type = MATERIAL_PROPERTY_FLOAT_SPECULAR_SHININESS;
        MaterialProperty->Value = SpecularShininess;

        ++MaterialPropertyIndex;
    }

    aiColor4D AmbientColor;
    if (aiGetMaterialColor(AssimpMaterial, AI_MATKEY_COLOR_AMBIENT, &AmbientColor) == AI_SUCCESS)
    {
        material_property *MaterialProperty = Material->Properties + MaterialPropertyIndex;
        MaterialProperty->Type = MATERIAL_PROPERTY_COLOR_AMBIENT;
        MaterialProperty->Color = AssimpColor2Vector(AmbientColor);

        ++MaterialPropertyIndex;
    }

    aiColor4D DiffuseColor;
    if (aiGetMaterialColor(AssimpMaterial, AI_MATKEY_COLOR_DIFFUSE, &DiffuseColor) == AI_SUCCESS)
    {
        material_property *MaterialProperty = Material->Properties + MaterialPropertyIndex;
        MaterialProperty->Type = MATERIAL_PROPERTY_COLOR_DIFFUSE;
        MaterialProperty->Color = AssimpColor2Vector(DiffuseColor);

        ++MaterialPropertyIndex;
    }

    aiColor4D SpecularColor;
    if (aiGetMaterialColor(AssimpMaterial, AI_MATKEY_COLOR_SPECULAR, &SpecularColor) == AI_SUCCESS)
    {
        material_property *MaterialProperty = Material->Properties + MaterialPropertyIndex;
        MaterialProperty->Type = MATERIAL_PROPERTY_COLOR_SPECULAR;
        MaterialProperty->Color = AssimpColor2Vector(SpecularColor);

        ++MaterialPropertyIndex;
    }

    u32 DiffuseMapCount = 0;
    bitmap *DiffuseMaps = 0;
    ProcessAssimpTextures(AssimpMaterial, aiTextureType_DIFFUSE, DirectoryPath, &DiffuseMapCount, &DiffuseMaps);
    for (u32 DiffuseMapIndex = 0; DiffuseMapIndex < DiffuseMapCount; ++DiffuseMapIndex)
    {
        bitmap *DiffuseMap = DiffuseMaps + DiffuseMapIndex;

        material_property *MaterialProperty = Material->Properties + MaterialPropertyIndex;
        MaterialProperty->Type = MATERIAL_PROPERTY_TEXTURE_DIFFUSE;
        MaterialProperty->Bitmap = *DiffuseMap;

        ++MaterialPropertyIndex;
    }

    u32 SpecularMapCount = 0;
    bitmap *SpecularMaps = 0;
    ProcessAssimpTextures(AssimpMaterial, aiTextureType_SPECULAR, DirectoryPath, &SpecularMapCount, &SpecularMaps);
    for (u32 SpecularMapIndex = 0; SpecularMapIndex < SpecularMapCount; ++SpecularMapIndex)
    {
        bitmap *SpecularMap = SpecularMaps + SpecularMapIndex;

        material_property *MaterialProperty = Material->Properties + MaterialPropertyIndex;
        MaterialProperty->Type = MATERIAL_PROPERTY_TEXTURE_SPECULAR;
        MaterialProperty->Bitmap = *SpecularMap;

        ++MaterialPropertyIndex;
    }

    u32 NormalsMapCount = 0;
    bitmap *NormalsMaps = 0;
    ProcessAssimpTextures(AssimpMaterial, aiTextureType_NORMALS, DirectoryPath, &NormalsMapCount, &NormalsMaps);
    for (u32 NormalsMapIndex = 0; NormalsMapIndex < NormalsMapCount; ++NormalsMapIndex)
    {
        bitmap *NormalsMap = NormalsMaps + NormalsMapIndex;

        material_property *MaterialProperty = Material->Properties + MaterialPropertyIndex;
        MaterialProperty->Type = MATERIAL_PROPERTY_TEXTURE_NORMALS;
        MaterialProperty->Bitmap = *NormalsMap;

        ++MaterialPropertyIndex;
    }

    Material->PropertyCount = MaterialPropertyIndex;

    Assert(MaterialPropertyIndex < MAX_MATERIAL_PROPERTY_COUNT);
}

void ProcessAssimpAnimation(aiAnimation *AssimpAnimation, animation *Animation)
{
    Assert(AssimpAnimation->mName.length < MAX_ANIMATION_NAME_LENGTH);
    CopyString(AssimpAnimation->mName.C_Str(), Animation->Name, (u32)(AssimpAnimation->mName.length + 1));
    Animation->Duration = (f32)(AssimpAnimation->mDuration / AssimpAnimation->mTicksPerSecond);
    Animation->PoseSampleCount = AssimpAnimation->mNumChannels;
    Animation->PoseSamples = (animation_sample *)malloc(Animation->PoseSampleCount * sizeof(animation_sample));

    for (u32 ChannelIndex = 0; ChannelIndex < AssimpAnimation->mNumChannels; ++ChannelIndex)
    {
        // Channel desribes the movement of a single joint over time
        aiNodeAnim *Channel = AssimpAnimation->mChannels[ChannelIndex];

        Assert((Channel->mNumPositionKeys == Channel->mNumRotationKeys) && (Channel->mNumPositionKeys == Channel->mNumScalingKeys));

        animation_sample *AnimationSample = Animation->PoseSamples + ChannelIndex;
        AnimationSample->KeyFrameCount = Channel->mNumPositionKeys;
        AnimationSample->KeyFrames = (key_frame *)malloc(AnimationSample->KeyFrameCount * sizeof(key_frame));

        for (u32 KeyIndex = 0; KeyIndex < Channel->mNumPositionKeys; ++KeyIndex)
        {
            aiVectorKey *PositionKey = Channel->mPositionKeys + KeyIndex;
            aiQuatKey *RotationKey = Channel->mRotationKeys + KeyIndex;
            aiVectorKey *ScalingKey = Channel->mScalingKeys + KeyIndex;

            Assert((PositionKey->mTime == RotationKey->mTime) && (PositionKey->mTime == ScalingKey->mTime));

            key_frame *KeyFrame = AnimationSample->KeyFrames + KeyIndex;
            KeyFrame->Time = (f32)PositionKey->mTime;
            KeyFrame->Pose = {};
            KeyFrame->Pose.Rotation = AssimpQuaternion2Quaternion(RotationKey->mValue);
            KeyFrame->Pose.Translation = AssimpVector2Vector(PositionKey->mValue);
            KeyFrame->Pose.Scale = AssimpVector2Vector(ScalingKey->mValue);
        }
    }
}

u32 FindJointIndexByName(const char *JointName, skeleton *Skeleton)
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

void ProcessAssimpMesh(aiMesh *AssimpMesh, u32 AssimpMeshIndex, aiNode *AssimpRootNode, mesh *Mesh, skeleton *Skeleton)
{
    switch (AssimpMesh->mPrimitiveTypes)
    {
        case aiPrimitiveType_LINE:
        {
            Mesh->PrimitiveType = PrimitiveType_Line;
            break;
        }
        case aiPrimitiveType_TRIANGLE:
        {
            Mesh->PrimitiveType = PrimitiveType_Triangle;
            break;
        }
        default: {
            Assert(!"Invalid primitive type");
        }
    }

    Mesh->VertexCount = AssimpMesh->mNumVertices;
    Mesh->Vertices = (vertex *)malloc(Mesh->VertexCount * sizeof(vertex));

    if (AssimpMesh->HasPositions())
    {
        for (u32 VertexIndex = 0; VertexIndex < AssimpMesh->mNumVertices; ++VertexIndex)
        {
            vertex *Vertex = Mesh->Vertices + VertexIndex;
            *Vertex = {};

            aiVector3D AssimpPosition = AssimpMesh->mVertices[VertexIndex];
            Vertex->Position = AssimpVector2Vector(AssimpPosition);

            if (AssimpMesh->HasNormals())
            {
                aiVector3D AssimpNormal = AssimpMesh->mNormals[VertexIndex];
                Vertex->Normal = AssimpVector2Vector(AssimpNormal);
            }

            if (AssimpMesh->HasTextureCoords(TEXTURE_COORDINATES_SET_INDEX))
            {
                aiVector3D AssimpTextureCoords = AssimpMesh->mTextureCoords[TEXTURE_COORDINATES_SET_INDEX][VertexIndex];
                Vertex->TextureCoords = vec2(AssimpVector2Vector(AssimpTextureCoords).xy);
            }
        }
    }

    if (AssimpMesh->HasBones())
    {
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
                [](const joint_weight &A, const joint_weight &B) -> b32
            {
                return A.Weight > B.Weight;
            });

            JointWeights.resize(MAX_WEIGHT_COUNT);

        }

        for (u32 VertexIndex = 0; VertexIndex < Mesh->VertexCount; ++VertexIndex)
        {
            vertex *Vertex = Mesh->Vertices + VertexIndex;

            dynamic_array<joint_weight> &JointWeights = JointWeightsTable[VertexIndex];

            for (u32 JointWeightIndex = 0; JointWeightIndex < MAX_WEIGHT_COUNT; ++JointWeightIndex)
            {
                joint_weight JointWeight = JointWeights[JointWeightIndex];

                Vertex->JointIndices[JointWeightIndex] = JointWeight.JointIndex;
                Vertex->Weights[JointWeightIndex] = JointWeight.Weight;
            }
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
        Mesh->Indices = (u32 *)malloc(Mesh->IndexCount * sizeof(u32));

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

void ProcessAssimpNodeHierarchy(aiNode *AssimpNode, hashtable<string, assimp_node *> &SceneNodes)
{
    assimp_node *Node = (assimp_node *)malloc(sizeof(assimp_node) * 1);
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

aiNode *GetBoneRootNode(aiNode *AssimpNode, hashtable<string, assimp_node *> &SceneNodes)
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

void ProcessAssimpBoneHierarchy(aiNode *AssimpNode, hashtable<string, assimp_node *> &SceneNodes, skeleton *Skeleton, i32 ParentIndex)
{
    static u32 CurrentIndex = 0;

    string Name = AssimpString2StdString(AssimpNode->mName);
    assimp_node *SceneNode = SceneNodes[Name];

    //Assert(SceneNode->Bone);

    joint *Joint = Skeleton->Joints + CurrentIndex;
    Assert(Name.length() < MAX_JOINT_NAME_LENGTH);
    CopyString(Name, Joint->Name);
    //Joint->InvBindTranform = AssimpMatrix2Matrix(SceneNode->Bone->mOffsetMatrix);
    Joint->ParentIndex = ParentIndex;

    aiVector3D Scale;
    aiVector3D Translation;
    aiQuaternion Rotation;
    SceneNode->Node->mTransformation.Decompose(Scale, Rotation, Translation);

    joint_pose *LocalJointPose = Skeleton->LocalJointPoses + CurrentIndex;
    LocalJointPose->Scale = AssimpVector2Vector(Scale);
    if (Scale.x > 65.f)
    {
        //LocalJointPose->Scale *= 0.1f;
    }
    LocalJointPose->Translation = AssimpVector2Vector(Translation);
    LocalJointPose->Rotation = AssimpQuaternion2Quaternion(Rotation);

    ++CurrentIndex;

    for (u32 ChildIndex = 0; ChildIndex < AssimpNode->mNumChildren; ++ChildIndex)
    {
        aiNode *ChildNode = AssimpNode->mChildren[ChildIndex];
        ProcessAssimpBoneHierarchy(ChildNode, SceneNodes, Skeleton, ParentIndex + 1);
    }
}

inline mat4
CalculateJointPose(joint_pose *JointPose)
{
    mat4 Result = mat4(1.f);
    
    mat4 S = Scale(JointPose->Scale);
    mat4 T = Translate(JointPose->Translation);
    mat4 R = GetRotationMatrix(JointPose->Rotation);

    Result = T * R * S;

    return Result;
}

// todo: check ParentIndex!
mat4 CalculateGlobalJointPose(joint *CurrentJoint, joint_pose *CurrentJointPose, skeleton *Skeleton)
{
    mat4 Result = mat4(1.f);

    dynamic_array<mat4> JointPoses;

    while (true)
    {
        mat4 Pose = CalculateJointPose(CurrentJointPose);
        //Result = Result * Pose;

        JointPoses.push_back(Pose);

        if (CurrentJoint->ParentIndex == -1)
        {
            break;
        }

        CurrentJointPose = Skeleton->LocalJointPoses + CurrentJoint->ParentIndex;
        CurrentJoint = Skeleton->Joints + CurrentJoint->ParentIndex;
    }

    for (i32 Index = (i32)JointPoses.size() - 1; Index >= 0; --Index)
    {
        Result = Result * JointPoses[Index];
    }

    return Result;
}

// todo: forget about the bones right now and just draw joints of the skeleton
void ProcessAssimpSkeleton(const aiScene *AssimpScene, skeleton *Skeleton)
{
    hashtable<string, assimp_node *> SceneNodes;
    ProcessAssimpNodeHierarchy(AssimpScene->mRootNode, SceneNodes);

    u32 JointCount = SceneNodes.size();
#if 0
    for (u32 MeshIndex = 0; MeshIndex < AssimpScene->mNumMeshes; ++MeshIndex)
    {
        aiMesh *AssimpMesh = AssimpScene->mMeshes[MeshIndex];

        JointCount += AssimpMesh->mNumBones;

        for (u32 BoneIndex = 0; BoneIndex < AssimpMesh->mNumBones; ++BoneIndex)
        {
            aiBone *AssimpBone = AssimpMesh->mBones[BoneIndex];
            aiNode *AssimpNode = AssimpScene->mRootNode->FindNode(AssimpBone->mName);

            string Name = AssimpString2StdString(AssimpNode->mName);
            SceneNodes[Name]->Bone = AssimpBone;
        }
    }
#endif

    Skeleton->JointCount = JointCount;
    Skeleton->Joints = (joint *)malloc(sizeof(joint) * JointCount);
    Skeleton->LocalJointPoses = (joint_pose *)malloc(sizeof(joint_pose) * JointCount);
    Skeleton->GlobalJointPoses = (mat4 *)malloc(sizeof(mat4) * JointCount);

    //aiNode *BoneRoot = GetBoneRootNode(AssimpScene->mRootNode, SceneNodes);
    //ProcessAssimpBoneHierarchy(BoneRoot, SceneNodes, Skeleton, -1);
    ProcessAssimpBoneHierarchy(AssimpScene->mRootNode, SceneNodes, Skeleton, -1);

    for (u32 JointIndex = 0; JointIndex < JointCount; ++JointIndex)
    {
        joint *Joint = Skeleton->Joints + JointIndex;
        joint_pose *LocalJointPose = Skeleton->LocalJointPoses + JointIndex;
        mat4 *GlobalJointPose = Skeleton->GlobalJointPoses + JointIndex;

        *GlobalJointPose = CalculateGlobalJointPose(Joint, LocalJointPose, Skeleton);
    }
}

void ProcessAssimpScene(const aiScene *AssimpScene, const char *DirectoryPath, model_asset *Asset)
{
    // todo: check if model has skeleton information
    ProcessAssimpSkeleton(AssimpScene, &Asset->Skeleton);

    if (AssimpScene->HasMeshes())
    {
        Asset->MeshCount = AssimpScene->mNumMeshes;
        Asset->Meshes = (mesh *)malloc(Asset->MeshCount * sizeof(mesh));

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
        Asset->Materials = (material_asset *)malloc(Asset->MaterialCount * sizeof(material_asset));

        for (u32 MaterialIndex = 0; MaterialIndex < Asset->MaterialCount; ++MaterialIndex)
        {
            aiMaterial *AssimpMaterial = AssimpScene->mMaterials[MaterialIndex];
            material_asset *Material = Asset->Materials + MaterialIndex;

            ProcessAssimpMaterial(AssimpMaterial, DirectoryPath, Material);
        }
    }

    if (AssimpScene->HasAnimations())
    {
        Asset->AnimationCount = AssimpScene->mNumAnimations;
        Asset->Animations = (animation *)malloc(Asset->AnimationCount * sizeof(animation));

        for (u32 AnimationIndex = 0; AnimationIndex < Asset->AnimationCount; ++AnimationIndex)
        {
            aiAnimation *AssimpAnimation = AssimpScene->mAnimations[AnimationIndex];
            animation *Animation = Asset->Animations + AnimationIndex;

            ProcessAssimpAnimation(AssimpAnimation, Animation);
        }
    }
}

void LoadModelAsset(char *FilePath, u32 Flags, model_asset *Asset)
{
    const aiScene *AssimpScene = aiImportFile(FilePath, Flags);

    if (AssimpScene)
    {
        *Asset = {};

        char DirectoryPath[256];
        GetDirectoryPath(FilePath, DirectoryPath);

        ProcessAssimpScene(AssimpScene, DirectoryPath, Asset);

        aiReleaseImport(AssimpScene);
    }
}

internal void
ReadAssetFile(const char *FilePath, model_asset *Asset, model_asset *OriginalAsset)
{
    FILE *AssetFile = fopen(FilePath, "rb");
    
    fseek(AssetFile, 0, SEEK_END);
    u32 FileSize = ftell(AssetFile);
    fseek(AssetFile, 0, SEEK_SET);

    void *Buffer = malloc(FileSize);

    fread(Buffer, FileSize, 1, AssetFile);

    model_asset_header *Header = (model_asset_header *)Buffer;

    model_asset_skeleton_header *SkeletonHeader = (model_asset_skeleton_header *)((u8 *)Buffer + Header->SkeletonHeaderOffset);
    skeleton Skeleton = {};
    Skeleton.JointCount = SkeletonHeader->JointCount;
    Skeleton.Joints = (joint *)((u8*)Buffer + SkeletonHeader->JointsOffset);
    Skeleton.LocalJointPoses = (joint_pose *)((u8*)Buffer + SkeletonHeader->LocalJointPosesOffset);
    Skeleton.GlobalJointPoses = (mat4 *)((u8*)Buffer + SkeletonHeader->GlobalJointPosesOffset);

    model_asset_meshes_header *MeshesHeader = (model_asset_meshes_header *)((u8 *)Buffer + Header->MeshesHeaderOffset);

    u32 NextMeshHeaderOffset = 0;

    for (u32 MeshIndex = 0; MeshIndex < MeshesHeader->MeshCount; ++MeshIndex)
    {
        model_asset_mesh_header *MeshHeader = (model_asset_mesh_header *)((u8 *)Buffer + 
            MeshesHeader->MeshesOffset + NextMeshHeaderOffset);
        mesh Mesh = {};
        Mesh.PrimitiveType = MeshHeader->PrimitiveType;
        Mesh.VertexCount = MeshHeader->VertexCount;
        Mesh.IndexCount = MeshHeader->IndexCount;
        Mesh.Vertices = (vertex *)((u8 *)Buffer + MeshHeader->VerticesOffset);
        Mesh.Indices = (u32 *)((u8 *)Buffer + MeshHeader->IndicesOffset);

        NextMeshHeaderOffset += sizeof(model_asset_mesh_header) + 
            MeshHeader->VertexCount * sizeof(vertex) + MeshHeader->IndexCount * sizeof(u32);
    }

    fclose(AssetFile);
}

i32 main(i32 ArgCount, char **Args)
{
    char *FilePath = (char *)"models\\Animated Human.fbx";
    // todo: http://assimp.sourceforge.net/lib_html/postprocess_8h.html
    u32 Flags = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_ValidateDataStructure;

    model_asset Asset;
    LoadModelAsset(FilePath, Flags, &Asset);

    FILE *AssetFile = fopen("assets\\dummy.asset", "wb");

    model_asset_header Header = {};
    Header.MagicValue = 0x451;
    Header.Version = 1;
    Header.SkeletonHeaderOffset = sizeof(model_asset_header);
    Header.MeshesHeaderOffset = Header.SkeletonHeaderOffset + sizeof(model_asset_skeleton_header) + 
        Asset.Skeleton.JointCount * (sizeof(joint) + sizeof(joint_pose) + sizeof(mat4));

    fwrite(&Header, sizeof(model_asset_header), 1, AssetFile);

    // Writing skeleton
    model_asset_skeleton_header SkeletonHeader = {};
    SkeletonHeader.JointCount = Asset.Skeleton.JointCount;
    SkeletonHeader.JointsOffset = Header.SkeletonHeaderOffset + sizeof(SkeletonHeader);
    SkeletonHeader.LocalJointPosesOffset = SkeletonHeader.JointsOffset + SkeletonHeader.JointCount * sizeof(joint);
    SkeletonHeader.GlobalJointPosesOffset = SkeletonHeader.LocalJointPosesOffset + SkeletonHeader.JointCount * sizeof(joint_pose);

    fwrite(&SkeletonHeader, sizeof(model_asset_skeleton_header), 1, AssetFile);
    fwrite(Asset.Skeleton.Joints, sizeof(joint), Asset.Skeleton.JointCount, AssetFile);
    fwrite(Asset.Skeleton.LocalJointPoses, sizeof(joint_pose), Asset.Skeleton.JointCount, AssetFile);
    fwrite(Asset.Skeleton.GlobalJointPoses, sizeof(mat4), Asset.Skeleton.JointCount, AssetFile);

    // Writing meshes
    model_asset_meshes_header MeshesHeader = {};
    MeshesHeader.MeshCount = Asset.MeshCount;
    MeshesHeader.MeshesOffset = Header.MeshesHeaderOffset + sizeof(model_asset_meshes_header);

    fwrite(&MeshesHeader, sizeof(model_asset_meshes_header), 1, AssetFile);

    for (u32 MeshIndex = 0; MeshIndex < Asset.MeshCount; ++MeshIndex)
    {
        mesh *Mesh = Asset.Meshes + MeshIndex;

        model_asset_mesh_header MeshHeader = {};
        MeshHeader.PrimitiveType = Mesh->PrimitiveType;
        MeshHeader.VertexCount = Mesh->VertexCount;
        MeshHeader.IndexCount = Mesh->IndexCount;
        MeshHeader.VerticesOffset = MeshesHeader.MeshesOffset + MeshIndex * (sizeof(model_asset_mesh_header) + 
            Mesh->VertexCount * sizeof(vertex) + Mesh->IndexCount * sizeof(u32)) + sizeof(model_asset_mesh_header);
        MeshHeader.IndicesOffset = MeshHeader.VerticesOffset + Mesh->VertexCount * sizeof(vertex);

        fwrite(&MeshHeader, sizeof(model_asset_mesh_header), 1, AssetFile);
        fwrite(Mesh->Vertices, sizeof(vertex), Mesh->VertexCount, AssetFile);
        fwrite(Mesh->Indices, sizeof(u32), Mesh->IndexCount, AssetFile);
    }

    // todo: add materials and animations

    fclose(AssetFile);

#if 1
    model_asset TestAsset = {};
    ReadAssetFile("assets\\dummy.asset", &TestAsset, &Asset);
#endif
}
