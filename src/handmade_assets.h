#pragma once

#define MAX_JOINT_NAME_LENGTH 256
#define MAX_ANIMATION_NAME_LENGTH 256

enum material_property_type
{
    MATERIAL_PROPERTY_COLOR_AMBIENT,
    MATERIAL_PROPERTY_COLOR_DIFFUSE,
    MATERIAL_PROPERTY_COLOR_SPECULAR,

    MATERIAL_PROPERTY_FLOAT_SPECULAR_SHININESS,

    MATERIAL_PROPERTY_TEXTURE_DIFFUSE,
    MATERIAL_PROPERTY_TEXTURE_SPECULAR,
    MATERIAL_PROPERTY_TEXTURE_NORMALS

    // ...
};

struct bitmap
{
    i32 Width;
    i32 Height;
    i32 Channels;
    void *Pixels;
};

// todo: import materials separatly from assimp (https://blender.stackexchange.com/questions/57531/fbx-export-why-there-are-no-materials-or-textures)?
struct material_property
{
    material_property_type Type;
    union
    {
        f32 Value;
        vec4 Color;
        bitmap Bitmap;
    };
};

struct material_asset
{
    u32 PropertyCount;
    material_property *Properties;
};

struct joint
{
    char Name[MAX_JOINT_NAME_LENGTH];
    mat4 InvBindTranform;
    i32 ParentIndex;
};

struct joint_pose
{
    quat Rotation;
    vec3 Translation;
    vec3 Scale;
};

struct skeleton
{
    u32 JointCount;
    joint *Joints;
    joint_pose *LocalJointPoses;
    mat4 *GlobalJointPoses;
};

struct key_frame
{
    f32 Time;
    joint_pose *Pose;
};

struct animation_sample
{
    u32 KeyFrameCount;
    key_frame *KeyFrames;
};

struct animation
{
    //skeleton *Skeleton;

    char Name[MAX_ANIMATION_NAME_LENGTH];
    f32 Duration;

    u32 PoseSampleCount;
    animation_sample *PoseSamples;
};

struct joint_weight
{
    u32 JointIndex;
    f32 Weight;
};

//struct vertex
//{
//    vec3 Position;
//    vec3 Normal;
//    vec2 TextureCoords;
//    i32 JointIndices[4];
//    vec4 Weights;
//};

struct mesh
{
    //skeleton *Skeleton;

    u32 VertexCount;
    vertex *Vertices;

    u32 IndexCount;
    u32 *Indices;
};

struct model_asset
{
    skeleton Skeleton;

    u32 MeshCount;
    mesh *Meshes;

    u32 MaterialCount;
    material_asset *Materials;

    u32 AnimationCount;
    animation *Animations;
};

struct model_asset_header
{
    i32 MagicValue;
    i32 Version;
    u32 SkeletonHeaderOffset;
    u32 MeshesHeaderOffset;
    //u32 MaterialsHeaderOffset;
    //u32 AnimationsHeaderOffset;
};

struct model_asset_skeleton_header
{
    u32 JointCount;
    u32 JointsOffset;
    u32 LocalJointPosesOffset;
    u32 GlobalJointPosesOffset;
};

struct model_asset_meshes_header
{
    u32 MeshCount;
    u32 MeshesOffset;
};

struct model_asset_mesh_header
{
    u32 VertexCount;
    u32 IndexCount;
    u32 VerticesOffset;
    u32 IndicesOffset;
};
