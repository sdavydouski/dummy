#pragma once

#define MAX_JOINT_NAME_LENGTH 256
#define MAX_ANIMATION_NAME_LENGTH 256

#define joint_pose transform

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
};

struct skeleton_pose
{
    skeleton *Skeleton;
    joint_pose *LocalJointPoses;
    mat4 *GlobalJointPoses;
};

struct joint_weight
{
    u32 JointIndex;
    f32 Weight;
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

// todo: play in reverse
struct animation_state
{
    f32 Time;
    f32 Weight;
    f32 WeightCoefficient;
    f32 PlaybackRate;

    animation_clip *Clip;
};

struct animation_blend_space_1d_value
{
    animation_state AnimationState;
    f32 Value;
};

struct animation_blend_space_1d
{
    u32 AnimationBlendSpaceValueCount;
    animation_blend_space_1d_value *AnimationBlendSpaceValues;
};

struct animation_blend_space_2d_value
{
    animation_state *AnimationState;
    vec2 Value;
};

struct animation_blend_space_2d_triangle
{
    animation_blend_space_2d_value Points[3];
};

struct animation_blend_space_2d
{
    u32 AnimationBlendSpaceTriangleCount;
    animation_blend_space_2d_triangle *AnimationBlendSpaceTriangles;
};

enum entity_state
{
    EntityState_None,
    EntityState_Idle,
    EntityState_Moving
};

enum animation_transition_type
{
    AnimationTransitionType_Immediate,
    AnimationTransitionType_Crossfade,
    AnimationTransitionType_Transitional
};

struct animation_transition
{
    animation_transition_type Type;

    f32 Duration;
    struct animation_node *TransitionNode;

    struct animation_node *From;
    struct animation_node *To;

};

enum animation_node_type
{
    AnimationNodeType_SingleMotion,
    AnimationNodeType_BlendSpace,
    AnimationNodeType_Graph
};

struct animation_node_params
{
    f32 Time;
    f32 MaxTime;
    struct random_sequence *RNG;
};

struct animation_node
{
    entity_state State;

    animation_node_type Type;
    union
    {
        animation_state Animation;
        animation_blend_space_1d *BlendSpace;
        struct animation_graph *Graph;
    };

    u32 TransitionCount;
    animation_transition *Transitions;

    u32 Index;
    f32 Weight;
    f32 Time;

    animation_node_params *Params;
    char Name[64];
};

struct animation_mixer
{
    f32 Time;
    f32 Duration;
    f32 StartWeight;

    animation_node *From;
    animation_node *To;
};

struct animation_graph
{
    u32 NodeCount;
    animation_node *Nodes;

    u32 ActiveNodeIndex;

    animation_mixer Mixer;
};

inline joint_pose *
GetRootLocalJointPose(skeleton_pose *Pose)
{
    joint_pose *Result = Pose->LocalJointPoses + 0;
    return Result;
}

inline joint_pose *
GetRootTranslationLocalJointPose(skeleton_pose *SkeletonPose)
{
    joint_pose *Result = SkeletonPose->LocalJointPoses + 1;
    return Result;
}