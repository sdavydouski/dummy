#pragma once

#define MAX_JOINT_NAME_LENGTH 256
#define MAX_ANIMATION_NAME_LENGTH 256

#define joint_pose transform

enum material_property_type
{
    MaterialProperty_Float_Shininess,
    
    MaterialProperty_Color_Ambient,
    MaterialProperty_Color_Diffuse,
    MaterialProperty_Color_Specular,
    
    MaterialProperty_Texture_Diffuse,
    MaterialProperty_Texture_Specular,
    MaterialProperty_Texture_Shininess,
    MaterialProperty_Texture_Normal
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
    u32 Id;
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
    // todo: ?
    joint_pose *LocalJointPoses;
    mat4 *GlobalJointPoses;
};

struct skeleton_pose
{
    skeleton *Skeleton;
    joint_pose *LocalJointPoses;
    //mat4 *GlobalJointPoses;
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
    char Name[MAX_ANIMATION_NAME_LENGTH];
    f32 Duration;
    b32 IsLooping;
    b32 InPlace;

    u32 PoseSampleCount;
    animation_sample *PoseSamples;
};

struct animation_clip_state
{
    animation_clip *Animation;
    f32 Time;
    f32 Weight;
    f32 PlaybackRate;
    b32 IsEnabled;
};

struct animation_blend_space_value_1d
{
    animation_clip_state *AnimationState;
    f32 Value;
};

struct animation_blend_space_1d
{
    u32 AnimationBlendSpaceValueCount;
    animation_blend_space_value_1d *AnimationBlendSpaceValues;
};

struct animation_blend_space_value_2d
{
    animation_clip_state *AnimationState;
    vec2 Value;
};

struct animation_blend_space_2d_triangle
{
    animation_blend_space_value_2d Points[3];
};

struct animation_blend_space_2d
{
    u32 AnimationBlendSpaceTriangleCount;
    animation_blend_space_2d_triangle *AnimationBlendSpaceTriangles;
};

struct animation_state_set
{
    u32 AnimationStateCount;
    animation_clip_state *AnimationStates;
};

struct joint_weight
{
    u32 JointIndex;
    f32 Weight;
};

enum primitive_type
{
    PrimitiveType_Line,
    PrimitiveType_Triangle
};

struct skinned_vertex
{
    vec3 Position;
    vec3 Normal;
    vec3 Tangent;
    // todo: calculate bitangent instead of storing it? (fourth weight too)
    vec3 Bitangent;
    vec2 TextureCoords;
    i32 JointIndices[4];
    vec4 Weights;
};

struct mesh
{
    u32 Id;
    u32 MaterialIndex;
    primitive_type PrimitiveType;

    u32 VertexCount;
    skinned_vertex *Vertices;

    u32 IndexCount;
    u32 *Indices;
};

struct model
{
    skeleton *Skeleton;

    u32 MeshCount;
    mesh *Meshes;

    u32 MaterialCount;
    mesh_material *Materials;

    u32 AnimationCount;
    animation_clip *Animations;

    u32 SkinningMatrixCount;
    mat4 *SkinningMatrices;
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

#pragma pack(push, 1)

struct model_asset_header
{
    i32 MagicValue;
    i32 Version;
    u64 SkeletonHeaderOffset;
    u64 MeshesHeaderOffset;
    u64 MaterialsHeaderOffset;
    u64 AnimationsHeaderOffset;
};

struct model_asset_skeleton_header
{
    u32 JointCount;
    u64 JointsOffset;
    u64 LocalJointPosesOffset;
    u64 GlobalJointPosesOffset;
};

struct model_asset_meshes_header
{
    u32 MeshCount;
    u64 MeshesOffset;
};

struct model_asset_mesh_header
{
    primitive_type PrimitiveType;
    u32 MaterialIndex;
    u32 VertexCount;
    u32 IndexCount;
    u64 VerticesOffset;
    u64 IndicesOffset;
};

struct model_asset_materials_header
{
    u32 MaterialCount;
    u64 MaterialsOffset;
};

struct model_asset_material_header
{
    u32 PropertyCount;
    u64 PropertiesOffset;
};

struct model_asset_material_property_header
{
    material_property_type Type;
    union
    {
        f32 Value;
        vec4 Color;
        bitmap Bitmap;
    };
    u64 BitmapOffset;
};

struct model_asset_animations_header
{
    u32 AnimationCount;
    u64 AnimationsOffset;
};

struct model_asset_animation_header
{
    char Name[MAX_ANIMATION_NAME_LENGTH];
    f32 Duration;
    b32 IsLooping;
    b32 InPlace;
    u32 PoseSampleCount;
    u64 PoseSamplesOffset;
};

struct model_asset_animation_sample_header
{
    u32 JointIndex;
    u32 KeyFrameCount;
    u64 KeyFramesOffset;
};

#pragma pack(pop)
