inline joint_pose
Interpolate(joint_pose *A, f32 t, joint_pose *B)
{
    joint_pose Result;

    Result.Translation = Lerp(A->Translation, t, B->Translation);
    Result.Rotation = Slerp(A->Rotation, t, B->Rotation);
    // todo: logarithmic interpolation for scale? (https://www.cmu.edu/biolphys/deserno/pdf/log_interpol.pdf)
    Result.Scale = Lerp(A->Scale, t, B->Scale);

    return Result;
}

inline joint_pose
Interpolate(joint_pose *A, joint_pose *B, joint_pose *C, f32 Alpha, f32 Beta, f32 Gamma)
{
    joint_pose Result;

    // todo: recheck!
    Result.Translation = A->Translation * Alpha + B->Translation * Beta + C->Translation * Gamma;
    Result.Rotation = A->Rotation * Alpha + B->Rotation * Beta + C->Rotation * Gamma;
    Result.Scale = A->Scale * Alpha + B->Scale * Beta + C->Scale * Gamma;

    return Result;
}

internal mat4
CalculateGlobalJointPose(joint *CurrentJoint, joint_pose *CurrentJointPose, skeleton_pose *Pose)
{
    mat4 Result = mat4(1.f);

    while (true)
    {
        mat4 Global = Transform(*CurrentJointPose);
        Result = Global * Result;

        if (CurrentJoint->ParentIndex == -1)
        {
            break;
        }

        CurrentJointPose = Pose->LocalJointPoses + CurrentJoint->ParentIndex;
        CurrentJoint = Pose->Skeleton->Joints + CurrentJoint->ParentIndex;
    }

    return Result;
}

// todo: optimize (caching?)
inline animation_sample *
GetAnimationSampleByJointIndex(animation_clip *Animation, u32 JointIndex)
{
    animation_sample *Result = 0;

    for (u32 AnimationSampleIndex = 0; AnimationSampleIndex < Animation->PoseSampleCount; ++AnimationSampleIndex)
    {
        animation_sample *AnimationSample = Animation->PoseSamples + AnimationSampleIndex;

        if (AnimationSample->JointIndex == JointIndex)
        {
            Result = AnimationSample;
            break;
        }
    }

    return Result;
}

struct closest_key_frames_result
{
    key_frame *Current;
    key_frame *Next;
};

internal closest_key_frames_result
FindClosestKeyFrames(animation_sample *PoseSample, f32 CurrentTime)
{
    Assert(PoseSample->KeyFrameCount > 1);

    closest_key_frames_result Result = {};

    for (u32 KeyFrameIndex = 0; KeyFrameIndex < PoseSample->KeyFrameCount - 1; ++KeyFrameIndex)
    {
        key_frame *CurrentKeyFrame = PoseSample->KeyFrames + KeyFrameIndex;
        key_frame *NextKeyFrame = PoseSample->KeyFrames + KeyFrameIndex + 1;

        if (CurrentKeyFrame->Time <= CurrentTime && CurrentTime < NextKeyFrame->Time)
        {
            Result.Current = CurrentKeyFrame;
            Result.Next = NextKeyFrame;
            break;
        }
    }

    if (!Result.Current || !Result.Next)
    {
        Result.Current = PoseSample->KeyFrames + (PoseSample->KeyFrameCount - 1);
        Result.Next = PoseSample->KeyFrames + 0;
    }

    return Result;
}

// todo: looping animations has discontinuity between last and first key frames (assimp issue?)
// todo!!!: something is wrong with animation for some models
internal void
AnimateSkeletonPose(skeleton_pose *SkeletonPose, animation_clip *Animation, f32 CurrentTime)
{
    for (u32 JointIndex = 0; JointIndex < SkeletonPose->Skeleton->JointCount; ++JointIndex)
    {
        joint_pose *LocalJointPose = SkeletonPose->LocalJointPoses + JointIndex;

        animation_sample *PoseSample = GetAnimationSampleByJointIndex(Animation, JointIndex);

        if (PoseSample && PoseSample->KeyFrameCount > 1)
        {
            closest_key_frames_result ClosestKeyFrames = FindClosestKeyFrames(PoseSample, CurrentTime);
            key_frame *CurrentKeyFrame = ClosestKeyFrames.Current;
            key_frame *NextKeyFrame = ClosestKeyFrames.Next;

            f32 t = (CurrentTime - CurrentKeyFrame->Time) / (Abs(NextKeyFrame->Time - CurrentKeyFrame->Time));

            Assert(t >= 0.f && t <= 1.f);

            *LocalJointPose = Interpolate(&CurrentKeyFrame->Pose, t, &NextKeyFrame->Pose);
        }
    }

    if (Animation->InPlace)
    {
        joint_pose *RootTranslationLocalJointPose = GetRootTranslationLocalJointPose(SkeletonPose);
        RootTranslationLocalJointPose->Translation = vec3(0.f, RootTranslationLocalJointPose->Translation.y, 0.f);
    }
}

internal void
CrossFade(animation_mixer *Mixer, animation_node *From, animation_node *To, f32 Duration)
{
    Mixer->IsEnabled = true;
    Mixer->From = From;
    Mixer->To = To;
    Mixer->Duration = Duration;
    Mixer->CurrentTime = 0.f;
    Mixer->StartWeight = To->Weight;

    // todo: ?
    //FadeIn(Mixer, To, Duration);
    //FadeOut(Mixer, From, Duration);
}

internal void
AnimationMixerPerFrameUpdate(animation_mixer *Mixer, f32 Delta)
{
    if (Mixer->IsEnabled)
    {
        Mixer->CurrentTime += Delta;

        if (Mixer->CurrentTime < Mixer->Duration && !(Mixer->To->Weight == 1.f && Mixer->From->Weight == 0.f))
        {
            // todo: ease-in/ease-out
            f32 Value = (Mixer->CurrentTime / Mixer->Duration);

            f32 FromWeight = (1.f - Mixer->StartWeight) - Value;
            
            Mixer->From->Weight = FromWeight;
            Mixer->From->Animation.Weight = FromWeight;
            Mixer->From->Animation.PlaybackRate = FromWeight;

            f32 ToWeight = Mixer->StartWeight + Value;

            Mixer->To->Weight = ToWeight;
            Mixer->To->Animation.Weight = ToWeight;
            Mixer->To->Animation.PlaybackRate = ToWeight;

            // todo: ?
            Clamp(&Mixer->From->Weight, 0.f, 1.f);
            Clamp(&Mixer->From->Animation.Weight, 0.f, 1.f);
            Clamp(&Mixer->From->Animation.PlaybackRate, 0.f, 1.f);

            Clamp(&Mixer->To->Weight, 0.f, 1.f);
            Clamp(&Mixer->To->Animation.Weight, 0.f, 1.f);
            Clamp(&Mixer->To->Animation.PlaybackRate, 0.f, 1.f);
        }
        else
        {
            Mixer->IsEnabled = false;

            Mixer->From->Weight = 0.f;
            Mixer->From->Animation.Weight = 0.f;
            Mixer->From->Animation.PlaybackRate = 1.f;
            Mixer->From->Animation.Time = 0.f;

            Mixer->To->Weight = 1.f;
            Mixer->To->Animation.Weight = 1.f;
            Mixer->To->Animation.PlaybackRate = 1.f;

            Mixer->From = 0;
            Mixer->To = 0;
            Mixer->Duration = 0.f;
            Mixer->CurrentTime = 0.f;
        }
    }
}

internal void
UpdateGlobalJointPoses(skeleton_pose *Pose, transform Transform)
{
    joint_pose *RootLocalJointPose = GetRootLocalJointPose(Pose);
    RootLocalJointPose->Translation = Transform.Translation;
    RootLocalJointPose->Rotation = Transform.Rotation;
    RootLocalJointPose->Scale = Transform.Scale;

    for (u32 JointIndex = 0; JointIndex < Pose->Skeleton->JointCount; ++JointIndex)
    {
        joint *Joint = Pose->Skeleton->Joints + JointIndex;
        joint_pose *LocalJointPose = Pose->LocalJointPoses + JointIndex;
        mat4 *GlobalJointPose = Pose->GlobalJointPoses + JointIndex;

        *GlobalJointPose = CalculateGlobalJointPose(Joint, LocalJointPose, Pose);
    }
}


internal void
TriangularLerpBlend(
    animation_blend_space_2d_triangle *AnimationBlendSpaceTriangle,
    vec2 Blend,
    skeleton_pose *Pose,
    memory_arena *Arena
)
{
    scoped_memory ScopedMemory(Arena);
    skeleton_pose *SkeletonPoses = PushArray(ScopedMemory.Arena, 3, skeleton_pose);

    for (u32 SkeletonPoseIndex = 0; SkeletonPoseIndex < 3; ++SkeletonPoseIndex)
    {
        skeleton_pose *SkeletonPose = SkeletonPoses + SkeletonPoseIndex;
        SkeletonPose->Skeleton = Pose->Skeleton;
        SkeletonPose->LocalJointPoses = PushArray(ScopedMemory.Arena, Pose->Skeleton->JointCount, joint_pose);

        for (u32 JointIndex = 0; JointIndex < Pose->Skeleton->JointCount; ++JointIndex)
        {
            joint_pose *LocalJointPose = SkeletonPose->LocalJointPoses + JointIndex;
            joint_pose *SkeletonLocalJointPose = Pose->LocalJointPoses + JointIndex;

            *LocalJointPose = *SkeletonLocalJointPose;
        }
    }

    skeleton_pose *SkeletonPoseA = SkeletonPoses + 0;
    skeleton_pose *SkeletonPoseB = SkeletonPoses + 1;
    skeleton_pose *SkeletonPoseC = SkeletonPoses + 2;

    animation_blend_space_value_2d *A = &AnimationBlendSpaceTriangle->Points[0];
    animation_blend_space_value_2d *B = &AnimationBlendSpaceTriangle->Points[1];
    animation_blend_space_value_2d *C = &AnimationBlendSpaceTriangle->Points[2];

    AnimateSkeletonPose(SkeletonPoseA, A->AnimationState->Clip, A->AnimationState->Time);
    AnimateSkeletonPose(SkeletonPoseB, B->AnimationState->Clip, B->AnimationState->Time);
    AnimateSkeletonPose(SkeletonPoseC, C->AnimationState->Clip, C->AnimationState->Time);

    f32 Alpha, Beta, Gamma;
    Barycentric(Blend, A->Value, B->Value, C->Value, Alpha, Beta, Gamma);

    Assert(Abs(1.f - (Alpha + Beta + Gamma)) < 0.0001f);

    for (u32 JointIndex = 0; JointIndex < Pose->Skeleton->JointCount; ++JointIndex)
    {
        joint_pose *LocalJointPose = Pose->LocalJointPoses + JointIndex;

        joint_pose *LocalJointPoseA = SkeletonPoseA->LocalJointPoses + JointIndex;
        joint_pose *LocalJointPoseB = SkeletonPoseB->LocalJointPoses + JointIndex;
        joint_pose *LocalJointPoseC = SkeletonPoseC->LocalJointPoses + JointIndex;

        *LocalJointPose = Interpolate(LocalJointPoseA, LocalJointPoseB, LocalJointPoseC, Alpha, Beta, Gamma);
    }
}

internal void
LinearLerpBlend(animation_blend_space_1d *AnimationBlendSpace, f32 BlendParameter)
{
    animation_blend_space_value_1d *AnimationBlendValueFrom = 0;
    animation_blend_space_value_1d *AnimationBlendValueTo = 0;

    for (u32 AnimationBlendValueIndex = 0; AnimationBlendValueIndex < AnimationBlendSpace->AnimationBlendSpaceValueCount - 1; ++AnimationBlendValueIndex)
    {
        animation_blend_space_value_1d *CurrentAnimationBlendValue = AnimationBlendSpace->AnimationBlendSpaceValues + AnimationBlendValueIndex;
        animation_blend_space_value_1d *NextAnimationBlendValue = AnimationBlendSpace->AnimationBlendSpaceValues + AnimationBlendValueIndex + 1;

        if (InRange(BlendParameter, CurrentAnimationBlendValue->Value, NextAnimationBlendValue->Value))
        {
            AnimationBlendValueFrom = CurrentAnimationBlendValue;
            AnimationBlendValueTo = NextAnimationBlendValue;
            break;
        }
    }

    // todo: ?
    for (u32 AnimationBlendValueIndex = 0; AnimationBlendValueIndex < AnimationBlendSpace->AnimationBlendSpaceValueCount; ++AnimationBlendValueIndex)
    {
        animation_blend_space_value_1d *AnimationBlendValue = AnimationBlendSpace->AnimationBlendSpaceValues + AnimationBlendValueIndex;
        AnimationBlendValue->AnimationState->Weight = 0.f;
    }

    f32 t = (BlendParameter - AnimationBlendValueFrom->Value) / (AnimationBlendValueTo->Value - AnimationBlendValueFrom->Value);

    Assert(t >= 0.f && t <= 1.f);

    AnimationBlendValueFrom->AnimationState->Weight = 1.f - t;
    //AnimationBlendValueFrom->AnimationState->PlaybackRate = 1.f - t;
    AnimationBlendValueTo->AnimationState->Weight = t;
    //AnimationBlendValueTo->AnimationState->PlaybackRate = t;
}

inline joint_pose
WeightedJointPose(joint_pose Pose, f32 Weight)
{
    joint_pose Result = {};

    Result.Scale = Pose.Scale * Weight;
    Result.Rotation = Pose.Rotation * Weight;
    Result.Translation = Pose.Translation * Weight;

    return Result;
}

inline animation_node *
GetActiveNode(animation_graph *Graph)
{
    animation_node *Result = Graph->Nodes + Graph->ActiveNodeIndex;
    return Result;
}

internal void
TransitionToState(animation_graph *Graph, entity_state State)
{
    animation_node *FromNode = GetActiveNode(Graph);
    animation_transition *Transition = 0;

    for (u32 TransitionIndex = 0; TransitionIndex < FromNode->TransitionCount; ++TransitionIndex)
    {
        animation_transition *CurrentTransition = FromNode->Transitions + TransitionIndex;

        if (CurrentTransition->To->State == State)
        {
            Transition = CurrentTransition;
            break;
        }
    }

    Assert(Transition);

    animation_node *ToNode = Transition->To;

    switch (Transition->Type)
     {
        case AnimationTransitionType_Immediate:
        {
            Graph->ActiveNodeIndex = ToNode->Index;

            // Disable Node
            FromNode->Weight = 0.f;
            FromNode->Animation.Time = 0.f;
            FromNode->Animation.Weight = 0.f;
            FromNode->Animation.PlaybackRate = 1.f;

            // Enable Node
            ToNode->Weight = 1.f;
            ToNode->Animation.Time = 0.f;
            ToNode->Animation.Weight = 1.f;
            ToNode->Animation.PlaybackRate = 1.f;

            break;
        }
        case AnimationTransitionType_Crossfade:
        {
            Graph->ActiveNodeIndex = ToNode->Index;

            CrossFade(&Graph->Mixer, FromNode, ToNode, Transition->Duration);

            break;
        }
        case AnimationTransitionType_Transitional:
        {
            Graph->ActiveNodeIndex = Transition->TransitionNode->Index;

            CrossFade(&Graph->Mixer, FromNode, Transition->TransitionNode, Transition->Duration);

            break;
        }
        default:
        {
            Assert(!"Invalid Transition Type");
        }
    }
}

internal void
AnimationStatePerFrameUpdate(animation_state *AnimationState, f32 Delta)
{
    Assert(AnimationState->Weight > 0.f);

    AnimationState->Time += Delta * AnimationState->PlaybackRate;

    if (AnimationState->Time > AnimationState->Clip->Duration)
    {
        if (AnimationState->Clip->IsLooping)
        {
            AnimationState->Time = 0.f;
        }
        else
        {
            AnimationState->Time = 0.f;
            AnimationState->Weight = 0.f;
        }
    }
}

internal void
AnimationNodePerFrameUpdate(animation_graph *Graph, animation_node *Node, f32 Delta)
{
    Assert(Node->Weight > 0.f);

    Node->Animation.Time += Delta * Node->Animation.PlaybackRate;

    if (Node->Animation.Time > Node->Animation.Clip->Duration)
    {
        if (Node->Animation.Clip->IsLooping)
        {
            Node->Animation.Time = 0.f;
        }
        else
        {
            Node->Animation.Time = 0.f;
            Node->Animation.Weight = 0.f;

            // todo:
            animation_transition *Transition = Node->Transitions + 0;

            TransitionToState(Graph, Transition->To->State);
        }
    }
}

internal void
AnimationGraphPerFrameUpdate(animation_graph *Graph, f32 Delta)
{
    AnimationMixerPerFrameUpdate(&Graph->Mixer, Delta);

    for (u32 NodeIndex = 0; NodeIndex < Graph->NodeCount; ++NodeIndex)
    {
        animation_node *Node = Graph->Nodes + NodeIndex;

        if (Node->Weight > 0.f)
        {
            AnimationNodePerFrameUpdate(Graph, Node, Delta);
        }
    }
}

// todo: hashtable
internal animation_state *
GetAnimation(animation_state_set *AnimationStateSet, const char *AnimationClipName)
{
    animation_state *Result = 0;

    for (u32 AnimationStateIndex = 0; AnimationStateIndex < AnimationStateSet->AnimationStateCount; ++AnimationStateIndex)
    {
        animation_state *AnimationState = AnimationStateSet->AnimationStates + AnimationStateIndex;

        if (StringEquals(AnimationState->Clip->Name, AnimationClipName))
        {
            Result = AnimationState;
            break;
        }
    }

    Assert(Result);

    return Result;
}

internal animation_clip *
GetAnimationClip(model *Model, const char *ClipName)
{
    animation_clip *Result = 0;

    for (u32 AnimationClipIndex = 0; AnimationClipIndex < Model->AnimationCount; ++AnimationClipIndex)
    {
        animation_clip *AnimationClip = Model->Animations + AnimationClipIndex;

        if (StringEquals(AnimationClip->Name, ClipName))
        {
            Result = AnimationClip;
            break;
        }
    }

    Assert(Result);

    return Result;
}

inline void
BuildAnimationNode(animation_node *Node, entity_state State, u32 Index, animation_clip *Clip)
{
    Node->State = State;
    Node->Index = Index;
    Node->Weight = 0.f;
    Node->Animation = {};
    Node->Animation.Clip = Clip;
    Node->Animation.Time = 0.f;
    Node->Animation.Weight = 0.f;
    Node->Animation.PlaybackRate = 1.f;
}

inline void
BuildAnimationTransition(
    animation_transition *Transition, 
    animation_node *From, 
    animation_node *To, 
    animation_transition_type Type, 
    f32 Duration = 0.f,
    animation_node *TransitionNode = 0
)
{
    Transition->From = From;
    Transition->To = To;
    Transition->Type = Type;
    Transition->Duration = Duration;
    Transition->TransitionNode = TransitionNode;
}

inline animation_node *
GetAnimationNode(animation_graph *Graph, entity_state State)
{
    animation_node *Result = 0;

    for (u32 NodeIndex = 0; NodeIndex < Graph->NodeCount; ++NodeIndex)
    {
        animation_node *Node = Graph->Nodes + NodeIndex;

        if (Node->State == State)
        {
            Result = Node;
        }
    }

    Assert(Result);

    return Result;
}

inline void
ActivateAnimationNode(animation_graph *Graph, entity_state State)
{
    animation_node *Node= GetAnimationNode(Graph, State);

    Graph->ActiveNodeIndex = Node->Index;
    Node->Weight = 1.f;
    Node->Animation.Weight = 1.f;
}

internal void
BuildAnimationGraph(animation_graph *Graph, model *Model, memory_arena *Arena)
{
    *Graph = {};

    Graph->Mixer = {};
    Graph->NodeCount = 3;
    Graph->Nodes = PushArray(Arena, Graph->NodeCount, animation_node);

    u32 NodeIndex = 0;

    // Create Nodes
    animation_node *NodeIdle = Graph->Nodes + NodeIndex;
    BuildAnimationNode(NodeIdle, EntityState_Idle, NodeIndex++, GetAnimationClip(Model, "Idle_2"));

    animation_node *NodeWalking = Graph->Nodes + NodeIndex;
    BuildAnimationNode(NodeWalking, EntityState_Walking, NodeIndex++, GetAnimationClip(Model, "Walking"));

    // Create Transitions

    // Idle Node
    NodeIdle->TransitionCount = 1;
    NodeIdle->Transitions = PushArray(Arena, NodeIdle->TransitionCount, animation_transition);

    animation_transition *IdleToWalkingTransition = NodeIdle->Transitions + 0;
    BuildAnimationTransition(IdleToWalkingTransition, NodeIdle, NodeWalking, AnimationTransitionType_Crossfade, 0.2f);

    // Walking Node
    NodeWalking->TransitionCount = 1;
    NodeWalking->Transitions = PushArray(Arena, NodeWalking->TransitionCount, animation_transition);

    animation_transition *WalkingToIdleTransition = NodeWalking->Transitions + 0;
    BuildAnimationTransition(WalkingToIdleTransition, NodeWalking, NodeIdle, AnimationTransitionType_Crossfade, 0.2f);
}

internal void
CalculateFinalSkeletonPose(animation_graph *AnimationGraph, skeleton_pose *Pose, memory_arena *Arena)
{
    // todo: ?
    u32 AnimationCount = AnimationGraph->NodeCount;

    scoped_memory ScopedMemory(Arena);
    skeleton_pose *SkeletonPoses = PushArray(ScopedMemory.Arena, AnimationCount, skeleton_pose);

    animation_state **ActiveAnimations = PushArray(ScopedMemory.Arena, AnimationCount, animation_state *);

    u32 ActiveAnimationIndex = 0;
    u32 ActiveAnimationCount = 0;
    for (u32 NodeIndex = 0; NodeIndex < AnimationGraph->NodeCount; ++NodeIndex)
    {
        animation_node *Node = AnimationGraph->Nodes + NodeIndex;

        if (Node->Weight > 0.f)
        {
            animation_state **ActiveAnimationState = ActiveAnimations + ActiveAnimationIndex++;
            *ActiveAnimationState = &Node->Animation;

            ++ActiveAnimationCount;
        }
    }

    if (ActiveAnimationCount > 0)
    {
        f32 TotalWeight = 0.f;
        for (u32 ActiveAnimationIndex = 0; ActiveAnimationIndex < ActiveAnimationCount; ++ActiveAnimationIndex)
        {
            animation_state *Animation = ActiveAnimations[ActiveAnimationIndex];

            TotalWeight += Animation->Weight;
        }
        Assert((1.f - TotalWeight) < 0.001f);

        // Creating skeleton pose for each input clip
        for (u32 SkeletonPoseIndex = 0; SkeletonPoseIndex < ActiveAnimationCount; ++SkeletonPoseIndex)
        {
            skeleton_pose *SkeletonPose = SkeletonPoses + SkeletonPoseIndex;
            SkeletonPose->Skeleton = Pose->Skeleton;
            SkeletonPose->LocalJointPoses = PushArray(ScopedMemory.Arena, Pose->Skeleton->JointCount, joint_pose);

            for (u32 JointIndex = 0; JointIndex < Pose->Skeleton->JointCount; ++JointIndex)
            {
                joint_pose *LocalJointPose = SkeletonPose->LocalJointPoses + JointIndex;
                joint_pose *SkeletonLocalJointPose = Pose->LocalJointPoses + JointIndex;

                *LocalJointPose = *SkeletonLocalJointPose;
            }
        }

        // Extracting skeleton pose from each input clip
        for (u32 AnimationIndex = 0; AnimationIndex < ActiveAnimationCount; ++AnimationIndex)
        {
            animation_state *AnimationState = ActiveAnimations[AnimationIndex];
            skeleton_pose *SkeletonPose = SkeletonPoses + AnimationIndex;

            AnimateSkeletonPose(SkeletonPose, AnimationState->Clip, AnimationState->Time);
        }

        for (u32 JointIndex = 0; JointIndex < Pose->Skeleton->JointCount; ++JointIndex)
        {
            joint_pose *LocalJointPose = Pose->LocalJointPoses + JointIndex;
            joint_pose OutputLocalJointPose = {};

            for (u32 AnimationIndex = 0; AnimationIndex < ActiveAnimationCount; ++AnimationIndex)
            {
                animation_state *AnimationState = ActiveAnimations[AnimationIndex];
                skeleton_pose *SkeletonPose = SkeletonPoses + AnimationIndex;

                joint_pose *AnimatedLocalJoint = SkeletonPose->LocalJointPoses + JointIndex;
                joint_pose WeightedLocalJoint = WeightedJointPose(*AnimatedLocalJoint, AnimationState->Weight);

                OutputLocalJointPose.Scale += WeightedLocalJoint.Scale;
                OutputLocalJointPose.Rotation += WeightedLocalJoint.Rotation;
                OutputLocalJointPose.Translation += WeightedLocalJoint.Translation;
            }

            *LocalJointPose = OutputLocalJointPose;
        }
    }
}

