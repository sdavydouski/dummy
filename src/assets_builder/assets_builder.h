#pragma once

#include "handmade_defs.h"
#include "handmade_math.h"

#undef PI

#include <assimp/cimport.h>        // Plain-C interface
#include <assimp/scene.h>          // Output data structure
#include <assimp/postprocess.h>    // Post processing flags

#include <string>
#include <vector>
#include <unordered_map>

using std::string;

template <typename TValue>
using dynamic_array = std::vector<TValue>;

template <typename TKey, typename TValue>
using hashtable = std::unordered_map<TKey, TValue>;

#define MAX_JOINT_NAME_LENGTH 256
#define MAX_ANIMATION_NAME_LENGTH 256

struct assimp_node
{
    aiNode *Node;
    aiBone *Bone;
};

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

struct material
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

struct vertex
{
    vec3 Position;
    vec3 Normal;
    vec2 TextureCoords;
    i32 JointIndices[4];
    vec4 Weights;
};

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
    material *Materials;

    u32 AnimationCount;
    animation *Animations;
};

// todo: offsets to skeleton, meshes, etc.
struct model_asset_header
{
    i32 MagicValue;
    i32 Version;
};

struct model_asset_skeleton_header
{
    u32 JointCount;
    u32 JointsOffset;
    u32 LocalJointPosesOffset;
    u32 GlobalJointPosesOffset;
};
