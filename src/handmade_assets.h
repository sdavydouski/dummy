#pragma once

#define MAX_JOINT_NAME_LENGTH 256
#define MAX_ANIMATION_NAME_LENGTH 256

#define joint_pose transform

enum material_property_type
{
    MATERIAL_PROPERTY_FLOAT_SHININESS,

    MATERIAL_PROPERTY_COLOR_AMBIENT,
    MATERIAL_PROPERTY_COLOR_DIFFUSE,
    MATERIAL_PROPERTY_COLOR_SPECULAR,

    MATERIAL_PROPERTY_TEXTURE_DIFFUSE,
    MATERIAL_PROPERTY_TEXTURE_SPECULAR,
    MATERIAL_PROPERTY_TEXTURE_SHININESS,
    MATERIAL_PROPERTY_TEXTURE_NORMALS
};

struct bitmap
{
    i32 Width;
    i32 Height;
    i32 Channels;
    void *Pixels;
};

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

struct mesh_material
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

struct skeleton
{
    u32 JointCount;
    joint *Joints;
    joint_pose *LocalJointPoses;
    mat4 *GlobalJointPoses;
};

struct key_frame
{
    joint_pose Pose;
    f32 Time;
};

struct animation_sample
{
    u32 JointIndex;
    u32 KeyFrameCount;
    key_frame *KeyFrames;
};

struct animation_clip
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

struct mesh
{
    //skeleton *Skeleton;

    primitive_type PrimitiveType;

    u32 VertexCount;
    skinned_vertex *Vertices;

    u32 IndexCount;
    u32 *Indices;
};

struct model_asset
{
    skeleton Skeleton;

    u32 MeshCount;
    mesh *Meshes;

    u32 MaterialCount;
    mesh_material *Materials;

    u32 AnimationCount;
    animation_clip *Animations;
};

struct model_asset_header
{
    i32 MagicValue;
    i32 Version;
    u32 SkeletonHeaderOffset;
    u32 MeshesHeaderOffset;
    u32 AnimationsHeaderOffset;
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
    primitive_type PrimitiveType;
    u32 VertexCount;
    u32 IndexCount;
    u32 VerticesOffset;
    u32 IndicesOffset;
};

struct model_asset_animations_header
{
    u32 AnimationCount;
    u32 AnimationsOffset;
};

struct model_asset_animation_header
{
    char Name[MAX_ANIMATION_NAME_LENGTH];
    f32 Duration;
    u32 PoseSampleCount;
    u64 PoseSamplesOffset;
};

struct model_asset_animation_sample_header
{
    u32 JointIndex;
    u32 KeyFrameCount;
    u64 KeyFramesOffset;
};
