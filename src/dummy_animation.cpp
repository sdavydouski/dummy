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

    // todo: ?
    if (!Result.Current || !Result.Next)
    {
        Result.Current = PoseSample->KeyFrames + (PoseSample->KeyFrameCount - 1);
        Result.Next = PoseSample->KeyFrames + 0;
    }

    return Result;
}

// todo: looping animations has discontinuity between last and first key frames (assimp issue?)
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
            //*LocalJointPose = NextKeyFrame->Pose;
        }
    }

    if (Animation->InPlace)
    {
        joint_pose *RootTranslationLocalJointPose = GetRootTranslationLocalJointPose(SkeletonPose);
        RootTranslationLocalJointPose->Translation = vec3(0.f, RootTranslationLocalJointPose->Translation.y, 0.f);
        //RootTranslationLocalJointPose->Translation = vec3(0.f, 0.f, 0.f);
    }
}

inline animation_node *
GetActiveNode(animation_graph *Graph)
{
    animation_node *Result = Graph->Nodes + Graph->ActiveNodeIndex;
    return Result;
}

internal void
CrossFade(animation_mixer *Mixer, animation_node *From, animation_node *To, f32 Duration)
{
    Mixer->From = From;
    Mixer->To = To;
    Mixer->Duration = Duration;
    Mixer->Time = 0.f;
    Mixer->StartWeight = To->Weight;

    // todo: ?
    //FadeIn(Mixer, To, Duration);
    //FadeOut(Mixer, From, Duration);
}

inline void
AdjustAnimationNodeWeight(animation_node *Node, f32 Weight)
{
    Assert(Weight >= 0.f && Weight <= 1.f);

    Node->Weight = Weight;

    switch (Node->Type)
    {
        case AnimationNodeType_SingleMotion:
        {
            Node->Animation.Weight = Node->Weight;
            Node->Animation.WeightCoefficient = 1.f;
            Node->Animation.PlaybackRate = Node->Weight;

            break;
        }
        case AnimationNodeType_BlendSpace:
        {
            for (u32 ValueIndex = 0; ValueIndex < Node->BlendSpace->AnimationBlendSpaceValueCount; ++ValueIndex)
            {
                animation_blend_space_1d_value *Value = Node->BlendSpace->AnimationBlendSpaceValues + ValueIndex;

                // todo: weights are overriden inside LinearLerpBlend!
                //Value->AnimationState.Weight *= Node->Weight;
                //Value->AnimationState.PlaybackRate *= Node->Weight;

                Value->AnimationState.WeightCoefficient = Node->Weight;
            }

            break;
        }
        case AnimationNodeType_Graph:
        {
            animation_node *SubNode = GetActiveNode(Node->Graph);
            AdjustAnimationNodeWeight(SubNode, Weight);

            break;
        }
        default:
        {
            Assert(!"Invalid animation node type");
        }
    }
}

internal void
AnimationMixerPerFrameUpdate(animation_mixer *Mixer, f32 Delta)
{
    if (Mixer->From && Mixer->To)
    {
        Mixer->Time += Delta;

        if (Mixer->Time > Mixer->Duration)
        {
            Mixer->Time = Mixer->Duration;
        }

        // todo: ease-in/ease-out
        f32 Value = Mixer->Time / Mixer->Duration;

        f32 FromWeight = (1.f - Mixer->StartWeight) - Value;
        AdjustAnimationNodeWeight(Mixer->From, FromWeight);

        f32 ToWeight = Mixer->StartWeight + Value;
        AdjustAnimationNodeWeight(Mixer->To, ToWeight);

        if (Mixer->Time == Mixer->Duration)
        {
#if 0
            Mixer->From->Weight = 0.f;
            Mixer->From->Animation.Weight = 0.f;
            Mixer->From->Animation.PlaybackRate = 1.f;
            Mixer->From->Animation.Time = 0.f;

            Mixer->To->Weight = 1.f;
            Mixer->To->Animation.Weight = 1.f;
            Mixer->To->Animation.PlaybackRate = 1.f;
#endif
            // ?
            Mixer->From->Weight = 0.f;
            Mixer->To->Weight = 1.f;

            Mixer->From = 0;
            Mixer->To = 0;
            Mixer->Duration = 0.f;
            Mixer->Time = 0.f;
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

    animation_blend_space_2d_value *A = &AnimationBlendSpaceTriangle->Points[0];
    animation_blend_space_2d_value *B = &AnimationBlendSpaceTriangle->Points[1];
    animation_blend_space_2d_value *C = &AnimationBlendSpaceTriangle->Points[2];

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
    animation_blend_space_1d_value *AnimationBlendValueFrom = 0;
    animation_blend_space_1d_value *AnimationBlendValueTo = 0;

    for (u32 AnimationBlendValueIndex = 0; AnimationBlendValueIndex < AnimationBlendSpace->AnimationBlendSpaceValueCount - 1; ++AnimationBlendValueIndex)
    {
        animation_blend_space_1d_value *CurrentAnimationBlendValue = AnimationBlendSpace->AnimationBlendSpaceValues + AnimationBlendValueIndex;
        animation_blend_space_1d_value *NextAnimationBlendValue = AnimationBlendSpace->AnimationBlendSpaceValues + AnimationBlendValueIndex + 1;

        if (InRange(BlendParameter, CurrentAnimationBlendValue->Value, NextAnimationBlendValue->Value))
        {
            AnimationBlendValueFrom = CurrentAnimationBlendValue;
            AnimationBlendValueTo = NextAnimationBlendValue;
            break;
        }
    }

    // todo: ?
#if 1
    for (u32 AnimationBlendValueIndex = 0; AnimationBlendValueIndex < AnimationBlendSpace->AnimationBlendSpaceValueCount; ++AnimationBlendValueIndex)
    {
        animation_blend_space_1d_value *AnimationBlendValue = AnimationBlendSpace->AnimationBlendSpaceValues + AnimationBlendValueIndex;
        AnimationBlendValue->AnimationState.Weight = 0.f;
    }
#endif

    Assert(AnimationBlendValueFrom && AnimationBlendValueTo);

    f32 t = (BlendParameter - AnimationBlendValueFrom->Value) / (AnimationBlendValueTo->Value - AnimationBlendValueFrom->Value);

    Assert(t >= 0.f && t <= 1.f);

    AnimationBlendValueFrom->AnimationState.Weight = 1.f - t;
    //AnimationBlendValueFrom->AnimationState.PlaybackRate = 1.f - t;
    AnimationBlendValueTo->AnimationState.Weight = t;
    //AnimationBlendValueTo->AnimationState.PlaybackRate = t;
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

inline void
ResetAnimationState(animation_state *Animation)
{
    Animation->Time = 0.f;
    Animation->Weight = 0.f;
    Animation->WeightCoefficient = 1.f;
    Animation->PlaybackRate = 1.f;
}

// todo: symmetric?
inline void
EnableAnimationNode(animation_node *Node)
{
    Node->Weight = 1.f;
    Node->Time = 0.f;

    switch (Node->Type)
    {
        case AnimationNodeType_SingleMotion:
        {
            Node->Animation.Time = 0.f;
            Node->Animation.Weight = 1.f;
            Node->Animation.WeightCoefficient = 1.f;
            Node->Animation.PlaybackRate = 1.f;

            break;
        }
        case AnimationNodeType_BlendSpace:
        {
            for (u32 ValueIndex = 0; ValueIndex < Node->BlendSpace->AnimationBlendSpaceValueCount; ++ValueIndex)
            {
                animation_blend_space_1d_value *Value = Node->BlendSpace->AnimationBlendSpaceValues;

                // todo: ?
            }

            break;
        }
        case AnimationNodeType_Graph:
        {
            for (u32 SubNodeIndex = 0; SubNodeIndex < Node->Graph->NodeCount; ++SubNodeIndex)
            {
                animation_node *SubNode = Node->Graph->Nodes + SubNodeIndex;

                // enable first subnode
                EnableAnimationNode(SubNode);
                break;
            }

            break;
        }
    }
}

internal void
DisableAnimationNode(animation_node *Node)
{
    Node->Weight = 0.f;
    Node->Time = 0.f;

    switch (Node->Type)
    {
        case AnimationNodeType_SingleMotion:
        {
            ResetAnimationState(&Node->Animation);

            break;
        }
        case AnimationNodeType_BlendSpace:
        {
            for (u32 ValueIndex = 0; ValueIndex < Node->BlendSpace->AnimationBlendSpaceValueCount; ++ValueIndex)
            {
                animation_blend_space_1d_value *Value = Node->BlendSpace->AnimationBlendSpaceValues;
                ResetAnimationState(&Value->AnimationState);
            }

            break;
        }
        case AnimationNodeType_Graph:
        {
            for (u32 SubNodeIndex = 0; SubNodeIndex < Node->Graph->NodeCount; ++SubNodeIndex)
            {
                animation_node *SubNode = Node->Graph->Nodes + SubNodeIndex;
                DisableAnimationNode(SubNode);
            }

            break;
        }
    }
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

            DisableAnimationNode(FromNode);
            EnableAnimationNode(ToNode);

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
TransitionToNode(animation_graph *Graph, const char *NodeName)
{
    animation_node *FromNode = GetActiveNode(Graph);
    animation_transition *Transition = 0;

    for (u32 TransitionIndex = 0; TransitionIndex < FromNode->TransitionCount; ++TransitionIndex)
    {
        animation_transition *CurrentTransition = FromNode->Transitions + TransitionIndex;

        if (StringEquals(CurrentTransition->To->Name, NodeName))
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

            DisableAnimationNode(FromNode);
            EnableAnimationNode(ToNode);

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
AnimationBlendSpacePerFrameUpdate(animation_blend_space_1d *BlendSpace, f32 BlendParameter, f32 Time01)
{
    LinearLerpBlend(BlendSpace, BlendParameter);

    for (u32 AnimationIndex = 0; AnimationIndex < BlendSpace->AnimationBlendSpaceValueCount; ++AnimationIndex)
    {
        animation_blend_space_1d_value *Value = BlendSpace->AnimationBlendSpaceValues + AnimationIndex;

        if (Value->AnimationState.Weight > 0.f)
        {
            animation_state *AnimationState = &Value->AnimationState;

            AnimationState->Time = Time01 * AnimationState->Clip->Duration * AnimationState->PlaybackRate;

            // todo: do I need this?
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
    }
}

internal void
AnimationGraphPerFrameUpdate(animation_graph *Graph, f32 Delta, f32 BlendParameter);

internal void
AnimationNodePerFrameUpdate(animation_graph *Graph, animation_node *Node, f32 Delta, f32 BlendParameter)
{
    Assert(Node->Weight > 0.f);

    Node->Time += Delta;

    if (Node->Time > 1.f)
    {
        Node->Time = 0.f;
    }

    if (Node->Params)
    {
        if (StringEquals(Node->Name, "Idle_Node"))
        {
            Node->Params->Time += Delta;

            if (Node->Params->Time > Node->Params->MaxTime)
            {
                Node->Params->Time = 0.f;
                Node->Params->MaxTime = RandomBetween(Node->Params->RNG, 7.f, 15.f);

                if (Random01(Node->Params->RNG) > 0.5f)
                {
                    TransitionToNode(Graph, "Long_Idle_Node");
                }
                else
                {
                    TransitionToNode(Graph, "Long_Idle_2_Node");
                }
            }
        }
    }

    switch (Node->Type)
    {
        case AnimationNodeType_SingleMotion:
        {
            AnimationStatePerFrameUpdate(&Node->Animation, Delta);

            // todo: ?
#if 1
            if (StringEquals(Node->Name, "Long_Idle_Node") || StringEquals(Node->Name, "Long_Idle_2_Node"))
            {
                if (Node->Animation.Weight == 0.f)
                {
                    Node->Weight = 0.f;
                    TransitionToNode(Graph, "Idle_Node");
                }
            }
#endif
            

            break;
        }
        case AnimationNodeType_BlendSpace:
        {
            AnimationBlendSpacePerFrameUpdate(Node->BlendSpace, BlendParameter, Node->Time);
            break;
        }
        case AnimationNodeType_Graph:
        {
            AnimationGraphPerFrameUpdate(Node->Graph, Delta, 0.f);
            break;
        }
        default:
        {
            Assert(!"Invalid animation node type");
        }
    }
}

internal void
AnimationGraphPerFrameUpdate(animation_graph *Graph, f32 Delta, f32 BlendParameter)
{
    AnimationMixerPerFrameUpdate(&Graph->Mixer, Delta);
    //AnimationMixerPerFrameUpdatePre(&Graph->Mixer, Delta);

    for (u32 NodeIndex = 0; NodeIndex < Graph->NodeCount; ++NodeIndex)
    {
        animation_node *Node = Graph->Nodes + NodeIndex;

        if (Node->Weight > 0.f)
        {
            AnimationNodePerFrameUpdate(Graph, Node, Delta, BlendParameter);
        }
    }

    //AnimationMixerPerFrameUpdatePost(&Graph->Mixer, Delta);
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

inline animation_state
CreateAnimationState(animation_clip *Clip)
{
    animation_state Result = {};

    Result.Clip = Clip;
    Result.Time = 0.f;
    Result.Weight = 0.f;
    Result.WeightCoefficient = 1.f;
    Result.PlaybackRate = 1.f;

    return Result;
}

inline void
BuildAnimationNode(animation_node *Node, entity_state State, const char *Name, u32 Index, animation_clip *Clip)
{
    Node->Type = AnimationNodeType_SingleMotion;
    Node->State = State;
    CopyString(Name, Node->Name, ArrayCount(Node->Name));
    Node->Index = Index;
    Node->Weight = 0.f;
    Node->Animation = CreateAnimationState(Clip);
    Node->Params = 0;
}

inline void
BuildAnimationNode(animation_node *Node, entity_state State, const char *Name, u32 Index, animation_blend_space_1d *BlendSpace)
{
    Node->Type = AnimationNodeType_BlendSpace;
    Node->State = State;
    CopyString(Name, Node->Name, ArrayCount(Node->Name));
    Node->Index = Index;
    Node->Weight = 0.f;
    Node->BlendSpace = BlendSpace;
    Node->Params = 0;
}

inline void
BuildAnimationNode(animation_node *Node, entity_state State, const char *Name, u32 Index, animation_graph *Graph)
{
    Node->Type = AnimationNodeType_Graph;
    Node->State = State;
    CopyString(Name, Node->Name, ArrayCount(Node->Name));
    Node->Index = Index;
    Node->Weight = 0.f;
    Node->Graph = Graph;
    Node->Params = 0;
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
}

internal void
BuildAnimationGraph(animation_graph *Graph, model *Model, memory_arena *Arena, random_sequence *RNG)
{
    *Graph = {};

    Graph->Mixer = {};
    Graph->NodeCount = 2;
    Graph->Nodes = PushArray(Arena, Graph->NodeCount, animation_node);

    u32 NodeIndex = 0;

    // Create Nodes

    // Idle
    animation_node *NodeIdle = Graph->Nodes + NodeIndex;

    animation_graph *IdleGraph = PushType(Arena, animation_graph);
    IdleGraph->Mixer = {};
    IdleGraph->NodeCount = 3;
    IdleGraph->Nodes = PushArray(Arena, IdleGraph->NodeCount, animation_node);
    BuildAnimationNode(NodeIdle, EntityState_Idle, "Idle_Graph", NodeIndex++, IdleGraph);

    {
        animation_node *IdleEntry = IdleGraph->Nodes + 0;
        BuildAnimationNode(IdleEntry, EntityState_None, "Idle_Node", 0, GetAnimationClip(Model, "Idle_2"));
        // 
        IdleEntry->Params = PushType(Arena, animation_node_params);
        IdleEntry->Params->Time = 0.f;
        IdleEntry->Params->MaxTime = RandomBetween(RNG, 7.f, 15.f);
        IdleEntry->Params->RNG = RNG;

        animation_node *LongIdleEntry = IdleGraph->Nodes + 1;
        BuildAnimationNode(LongIdleEntry, EntityState_None, "Long_Idle_Node", 1, GetAnimationClip(Model, "Idle"));

        animation_node *LongIdle2Entry = IdleGraph->Nodes + 2;
        BuildAnimationNode(LongIdle2Entry, EntityState_None, "Long_Idle_2_Node", 1, GetAnimationClip(Model, "Idle_3"));

        // Transitions

        // --
        IdleEntry->TransitionCount = 2;
        IdleEntry->Transitions = PushArray(Arena, IdleEntry->TransitionCount, animation_transition);

        animation_transition *IdleToLongIdleTransition = IdleEntry->Transitions + 0;
        BuildAnimationTransition(IdleToLongIdleTransition, IdleEntry, LongIdleEntry, AnimationTransitionType_Crossfade, 0.2f);

        animation_transition *IdleToLongIdle2Transition = IdleEntry->Transitions + 1;
        BuildAnimationTransition(IdleToLongIdle2Transition, IdleEntry, LongIdle2Entry, AnimationTransitionType_Crossfade, 0.2f);

        // --
        LongIdleEntry->TransitionCount = 1;
        LongIdleEntry->Transitions = PushArray(Arena, LongIdleEntry->TransitionCount, animation_transition);

        animation_transition *LongIdleToIdleTransition = LongIdleEntry->Transitions + 0;
        BuildAnimationTransition(LongIdleToIdleTransition, LongIdleEntry, IdleEntry, AnimationTransitionType_Crossfade, 0.2f);

        // --
        LongIdle2Entry->TransitionCount = 1;
        LongIdle2Entry->Transitions = PushArray(Arena, LongIdle2Entry->TransitionCount, animation_transition);

        animation_transition *LongIdle2ToIdleTransition = LongIdle2Entry->Transitions + 0;
        BuildAnimationTransition(LongIdle2ToIdleTransition, LongIdle2Entry, IdleEntry, AnimationTransitionType_Crossfade, 0.2f);

        //
        IdleGraph->ActiveNodeIndex = 0;
        IdleEntry->Weight = 1.f;
        IdleEntry->Animation.Weight = 1.f;
        IdleEntry->Animation.WeightCoefficient = 1.f;
    }

    // Moving
    animation_node *NodeWalking = Graph->Nodes + NodeIndex;

    animation_blend_space_1d *BlendSpace = PushType(Arena, animation_blend_space_1d);
    BlendSpace->AnimationBlendSpaceValueCount = 3;
    BlendSpace->AnimationBlendSpaceValues = PushArray(Arena, BlendSpace->AnimationBlendSpaceValueCount, animation_blend_space_1d_value);

    {
        animation_blend_space_1d_value *Value = BlendSpace->AnimationBlendSpaceValues + 0;
        Value->Value = 0.f;
        Value->AnimationState = CreateAnimationState(GetAnimationClip(Model, "Idle_2"));
    }

    {
        animation_blend_space_1d_value *Value = BlendSpace->AnimationBlendSpaceValues + 1;
        Value->Value = 0.75f;
        Value->AnimationState = CreateAnimationState(GetAnimationClip(Model, "Walking"));
    }

    {
        animation_blend_space_1d_value *Value = BlendSpace->AnimationBlendSpaceValues + 2;
        Value->Value = 1.f;
        Value->AnimationState = CreateAnimationState(GetAnimationClip(Model, "Running"));
    }

    BuildAnimationNode(NodeWalking, EntityState_Moving, "Moving_Node", NodeIndex++, BlendSpace);

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

    int t = 0;
}

internal void
CalculateFinalSkeletonPose(animation_graph *AnimationGraph, skeleton_pose *Pose, memory_arena *Arena)
{
    // todo: wrong?
    //u32 AnimationCount = AnimationGraph->NodeCount;
    u32 AnimationCount = 4;

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
            switch (Node->Type)
            {
                case AnimationNodeType_SingleMotion:
                {
                    animation_state **ActiveAnimationState = ActiveAnimations + ActiveAnimationIndex++;
                    *ActiveAnimationState = &Node->Animation;

                    ++ActiveAnimationCount;

                    break;
                }
                case AnimationNodeType_BlendSpace:
                {
                    for (u32 Index = 0; Index < Node->BlendSpace->AnimationBlendSpaceValueCount; ++Index)
                    {
                        animation_blend_space_1d_value *Value = Node->BlendSpace->AnimationBlendSpaceValues + Index;

                        if (Value->AnimationState.Weight > 0.f)
                        {
                            animation_state **ActiveAnimationState = ActiveAnimations + ActiveAnimationIndex++;
                            *ActiveAnimationState = &Value->AnimationState;

                            ++ActiveAnimationCount;
                        }
                    }

                    break;
                }
                case AnimationNodeType_Graph:
                {
                    for (u32 NodeIndex = 0; NodeIndex < Node->Graph->NodeCount; ++NodeIndex)
                    {
                        animation_node *SubNode = Node->Graph->Nodes + NodeIndex;

                        // todo: recursive?
                        if (SubNode->Animation.Weight > 0.f)
                        {
                            animation_state **ActiveAnimationState = ActiveAnimations + ActiveAnimationIndex++;
                            *ActiveAnimationState = &SubNode->Animation;

                            ++ActiveAnimationCount;
                        }
                    }

                    break;
                }
            }

            
        }
    }

    if (ActiveAnimationCount > 0)
    {
        f32 TotalWeight = 0.f;
        for (u32 ActiveAnimationIndex = 0; ActiveAnimationIndex < ActiveAnimationCount; ++ActiveAnimationIndex)
        {
            animation_state *Animation = ActiveAnimations[ActiveAnimationIndex];

            // 
            Animation->Weight *= Animation->WeightCoefficient;
            //Animation->WeightCoefficient = 1.f;

            TotalWeight += Animation->Weight;
        }
        Assert(Abs(1.f - TotalWeight) < 0.001f);

        // Creating skeleton pose for each input clip
        for (u32 SkeletonPoseIndex = 0; SkeletonPoseIndex < ActiveAnimationCount; ++SkeletonPoseIndex)
        {
            skeleton_pose *SkeletonPose = SkeletonPoses + SkeletonPoseIndex;
            SkeletonPose->Skeleton = Pose->Skeleton;
            SkeletonPose->LocalJointPoses = PushArray(ScopedMemory.Arena, Pose->Skeleton->JointCount, joint_pose);

            // todo: ?
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

