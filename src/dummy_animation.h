#pragma once

#define MAX_JOINT_NAME_LENGTH 256
#define MAX_ANIMATION_NAME_LENGTH 256

#define joint_pose transform

struct random_sequence;
struct animation_node;
struct animation_graph;

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

    u32 PoseSampleCount;
    animation_sample *PoseSamples;
};

struct animation_state
{
    f32 Time;
    // todo: This weight is used to store computed values during final pose computing. 
    // This of a better name or place it somewhere else?
    f32 Weight;

    b32 IsLooping;

    animation_clip *Clip;
};

struct blend_space_1d_value
{
    animation_state AnimationState;
    f32 Value;
    f32 Weight;
};

struct blend_space_1d
{
    f32 NormalizedTime;
    f32 Parameter;

    u32 ValueCount;
    blend_space_1d_value *Values;
};

enum animation_transition_type
{
    AnimationTransitionType_Immediate,
    AnimationTransitionType_Crossfade,
    AnimationTransitionType_Transitional
};

struct animation_transition
{
    animation_node *From;
    animation_node *To;

    animation_transition_type Type;
    union
    {
        f32 Duration;
        animation_node *TransitionNode;
    };
};

enum animation_node_type
{
    AnimationNodeType_Clip,
    AnimationNodeType_BlendSpace,
    AnimationNodeType_Graph
};

struct animation_node
{
    char Name[64];
    f32 Weight;

    animation_node_type Type;
    union
    {
        animation_state Animation;
        blend_space_1d *BlendSpace;
        animation_graph *Graph;
    };

    u32 TransitionCount;
    animation_transition *Transitions;
};

struct animation_mixer
{
    f32 Time;
    f32 Duration;
    f32 FadeInWeight;
    f32 FadeOutWeight;

    animation_node *FadeIn;
    animation_node *FadeOut;
};

struct animation_graph_params
{
    f32 Move;
};

struct animation_graph_state
{
    f32 Time;
};

struct game_input;

#define ANIMATION_GRAPH_SET_PARAMETERS(name) void name(animation_graph *Graph, f32 Move)
typedef ANIMATION_GRAPH_SET_PARAMETERS(animation_graph_set_parameters);

#define ANIMATION_GRAPH_UPDATE(name) void name(animation_graph *Graph, game_input *Input, random_sequence *Entropy, f32 MaxTime, f32 Delta)
typedef ANIMATION_GRAPH_UPDATE(animation_graph_update);

struct animation_graph
{
    u32 NodeCount;
    animation_node *Nodes;

    animation_node *Entry;
    animation_node *Active;

    animation_mixer Mixer;

    animation_graph_state State;

    animation_graph_set_parameters *SetParameters;
    animation_graph_update *Update;
};
