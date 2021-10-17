inline joint_pose *
GetRootLocalJointPose(skeleton_pose *Pose)
{
    joint_pose *Result = Pose->LocalJointPoses + 0;
    return Result;
}

inline joint_pose *
GetRootTranslationLocalJointPose(skeleton_pose *SkeletonPose)
{
    // todo:
    joint_pose *Result = SkeletonPose->LocalJointPoses + 1;
    return Result;
}

inline joint_pose
Lerp(joint_pose *From, f32 t, joint_pose *To)
{
    joint_pose Result;

    Result.Translation = Lerp(From->Translation, t, To->Translation);
    Result.Rotation = Slerp(From->Rotation, t, To->Rotation);
    Result.Scale = Lerp(From->Scale, t, To->Scale);

    return Result;
}

internal void
Lerp(skeleton_pose *From, f32 t, skeleton_pose *To, skeleton_pose *Dest)
{
    u32 JointCount = Dest->Skeleton->JointCount;

    Assert(From->Skeleton->JointCount == JointCount);
    Assert(To->Skeleton->JointCount == JointCount);

    for (u32 JointIndex = 0; JointIndex < JointCount; ++JointIndex)
    {
        joint_pose *FromPose = From->LocalJointPoses + JointIndex;
        joint_pose *ToPose = To->LocalJointPoses + JointIndex;
        joint_pose *DestPose = Dest->LocalJointPoses + JointIndex;

        *DestPose = Lerp(FromPose, t, ToPose);
    }
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

internal void
FindClosestKeyFrames(animation_sample *PoseSample, f32 CurrentTime, key_frame **Prev, key_frame **Next)
{
    if (PoseSample->KeyFrameCount > 1)
    {
        b32 Found = false;

        for (u32 KeyFrameIndex = 0; KeyFrameIndex < PoseSample->KeyFrameCount - 1; ++KeyFrameIndex)
        {
            key_frame *CurrentKeyFrame = PoseSample->KeyFrames + KeyFrameIndex;
            key_frame *NextKeyFrame = PoseSample->KeyFrames + KeyFrameIndex + 1;

            if (CurrentKeyFrame->Time <= CurrentTime && CurrentTime < NextKeyFrame->Time)
            {
                Found = true;

                *Prev = CurrentKeyFrame;
                *Next = NextKeyFrame;

                break;
            }
        }

        if (!Found)
        {
            *Prev = Last(PoseSample->KeyFrames, PoseSample->KeyFrameCount);
            *Next = First(PoseSample->KeyFrames);
        }
    }
    else
    {
        *Prev = First(PoseSample->KeyFrames);
        *Next = First(PoseSample->KeyFrames);
    }
}

internal void
AnimateSkeletonPose(skeleton_pose *SkeletonPose, animation_clip *Animation, b32 InPlace, f32 Time)
{
    for (u32 JointIndex = 0; JointIndex < SkeletonPose->Skeleton->JointCount; ++JointIndex)
    {
        joint_pose *LocalJointPose = SkeletonPose->LocalJointPoses + JointIndex;

        animation_sample *PoseSample = GetAnimationSampleByJointIndex(Animation, JointIndex);

        if (PoseSample)
        {
            key_frame *PrevKeyFrame = 0;
            key_frame *NextKeyFrame = 0;
            FindClosestKeyFrames(PoseSample, Time, &PrevKeyFrame, &NextKeyFrame);

            f32 t = 0.f;

            if (PoseSample->KeyFrameCount > 1)
            {
                t = (Time - PrevKeyFrame->Time) / (Abs(NextKeyFrame->Time - PrevKeyFrame->Time));
            }

            Assert(t >= 0.f && t <= 1.f);

            *LocalJointPose = Lerp(&PrevKeyFrame->Pose, t, &NextKeyFrame->Pose);
        }
    }

    if (InPlace)
    {
        joint_pose *RootTranslationLocalJointPose = GetRootTranslationLocalJointPose(SkeletonPose);
        RootTranslationLocalJointPose->Translation = vec3(0.f, RootTranslationLocalJointPose->Translation.y, 0.f);
    }
}

// todo: do I need these helper functions?
inline void
ResetAnimationState(animation_state *Animation)
{
    Animation->Time = 0.f;
}

// todo: similar functions (enable/disable?)
inline void
EnableAnimationNode(animation_node *Node)
{
    Node->Weight = 1.f;

    switch (Node->Type)
    {
        case AnimationNodeType_SingleMotion:
        {
            ResetAnimationState(&Node->Animation);

            break;
        }
        case AnimationNodeType_BlendSpace:
        {
            for (u32 ValueIndex = 0; ValueIndex < Node->BlendSpace->ValueCount; ++ValueIndex)
            {
                blend_space_1d_value *Value = Node->BlendSpace->Values;

                ResetAnimationState(&Value->AnimationState);
            }

            break;
        }
        case AnimationNodeType_Graph:
        {
            EnableAnimationNode(Node->Graph->Entry);
            // todo: use ActiveAnimationNode instead?
            Node->Graph->Active = Node->Graph->Entry;

            break;
        }
    }
}

internal void
DisableAnimationNode(animation_node *Node)
{
    Node->Weight = 0.f;

    switch (Node->Type)
    {
        case AnimationNodeType_SingleMotion:
        {
            ResetAnimationState(&Node->Animation);

            break;
        }
        case AnimationNodeType_BlendSpace:
        {
            for (u32 ValueIndex = 0; ValueIndex < Node->BlendSpace->ValueCount; ++ValueIndex)
            {
                blend_space_1d_value *Value = Node->BlendSpace->Values + ValueIndex;

                Value->Weight = 0.f;
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

            Node->Graph->Mixer = {};
            Node->Graph->Entry->Weight = 1.f;
            Node->Graph->Active = Node->Graph->Entry;

            break;
        }
    }
}

inline void
FadeIn(animation_mixer* Mixer, animation_node *Node, f32 Duration, f32 FadeInWeight)
{
    Mixer->Time = 0.f;
    Mixer->Duration = Duration;
    Mixer->FadeIn = Node;
    Mixer->FadeInWeight = FadeInWeight;
}

inline void
FadeOut(animation_mixer* Mixer, animation_node *Node, f32 Duration, f32 FadeOutWeight)
{
    Mixer->Time = 0.f;
    Mixer->Duration = Duration;
    Mixer->FadeOut = Node;
    Mixer->FadeOutWeight = FadeOutWeight;
}

inline void
CrossFade(animation_mixer *Mixer, animation_node *From, animation_node *To, f32 Duration)
{
    FadeIn(Mixer, To, Duration, To->Weight);
    FadeOut(Mixer, From, Duration, From->Weight);
}

internal void
AnimationMixerPerFrameUpdate(animation_mixer *Mixer, f32 Delta)
{
    Mixer->Time += Delta;

    if (Mixer->Time > Mixer->Duration)
    {
        Mixer->Time = Mixer->Duration;
    }

    f32 Value = Mixer->Time / Mixer->Duration;

    if (Mixer->FadeIn)
    {
        f32 FadeInWeight = Min(1.f, Mixer->FadeInWeight + Value);
        Assert(FadeInWeight >= 0.f && FadeInWeight <= 1.f);

        Mixer->FadeIn->Weight = FadeInWeight;

        if (Mixer->Time == Mixer->Duration)
        {
            Assert(Mixer->FadeIn->Weight == 1.f);

            // todo: don't need to do this? 
            //EnableAnimationNode(Mixer->FadeIn);
        }
    }

    if (Mixer->FadeOut)
    {
        f32 FadeOutWeight = Max(0.f, (Mixer->FadeOutWeight - Value));
        Assert(FadeOutWeight >= 0.f && FadeOutWeight <= 1.f);

        Mixer->FadeOut->Weight = FadeOutWeight;

        if (Mixer->Time == Mixer->Duration)
        {
            Assert(Mixer->FadeOut->Weight == 0.f);

            DisableAnimationNode(Mixer->FadeOut);
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
TransitionToNode(animation_graph *Graph, const char *NodeName)
{
    animation_node *FromNode = Graph->Active;
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
            Graph->Active = ToNode;

            EnableAnimationNode(ToNode);
            DisableAnimationNode(FromNode);

            break;
        }
        case AnimationTransitionType_Crossfade:
        {
            Graph->Active = ToNode;

            CrossFade(&Graph->Mixer, FromNode, ToNode, Transition->Duration);

            break;
        }
        default:
        {
            Assert(!"Invalid Transition Type");
        }
    }

    // todo: check if node is graph and reset it?
    if (ToNode->Type == AnimationNodeType_Graph)
    {
        DisableAnimationNode(ToNode);
    }
}

internal void
AnimationStatePerFrameUpdate(animation_state *AnimationState, f32 Delta)
{
    AnimationState->Time += Delta;

    if (AnimationState->Time > AnimationState->Clip->Duration)
    {
        if (AnimationState->IsLooping)
        {
            AnimationState->Time = 0.f;
        }
        else
        {
            AnimationState->Time = AnimationState->Clip->Duration;
        }
    }
}

internal void
LinearLerpBlend(blend_space_1d *BlendSpace, f32 BlendParameter)
{
    Assert(BlendSpace->ValueCount > 1);

    blend_space_1d_value *BlendValueFrom = 0;
    blend_space_1d_value *BlendValueTo = 0;

    for (u32 AnimationBlendValueIndex = 0; AnimationBlendValueIndex < BlendSpace->ValueCount - 1; ++AnimationBlendValueIndex)
    {
        blend_space_1d_value *CurrentAnimationBlendValue = BlendSpace->Values + AnimationBlendValueIndex;
        blend_space_1d_value *NextAnimationBlendValue = BlendSpace->Values + AnimationBlendValueIndex + 1;

        if (InRange(BlendParameter, CurrentAnimationBlendValue->Value, NextAnimationBlendValue->Value))
        {
            BlendValueFrom = CurrentAnimationBlendValue;
            BlendValueTo = NextAnimationBlendValue;
            break;
        }
    }

    // todo: perhaps, it's better to do a full weights reset after final skeleton pose calculation?
    for (u32 AnimationBlendValueIndex = 0; AnimationBlendValueIndex < BlendSpace->ValueCount; ++AnimationBlendValueIndex)
    {
        blend_space_1d_value *AnimationBlendValue = BlendSpace->Values + AnimationBlendValueIndex;
        AnimationBlendValue->Weight = 0.f;
    }

    Assert(BlendValueFrom && BlendValueTo);

    f32 t = (BlendParameter - BlendValueFrom->Value) / (BlendValueTo->Value - BlendValueFrom->Value);

    Assert(t >= 0.f && t <= 1.f);

    BlendValueFrom->Weight = 1.f - t;
    BlendValueTo->Weight = t;
}

internal void
AnimationBlendSpacePerFrameUpdate(blend_space_1d *BlendSpace, animation_graph_params Params, f32 Delta)
{
    LinearLerpBlend(BlendSpace, Params.Move);

    f32 Duration = 0.f;
    for (u32 AnimationIndex = 0; AnimationIndex < BlendSpace->ValueCount; ++AnimationIndex)
    {
        blend_space_1d_value *Value = BlendSpace->Values + AnimationIndex;
        Duration += Value->AnimationState.Clip->Duration * Value->Weight;
    }

    f32 NormalizedDelta = Delta / Duration;
    BlendSpace->NormalizedTime += NormalizedDelta;

    if (BlendSpace->NormalizedTime > 1.f)
    {
        BlendSpace->NormalizedTime = 0.f;
    }

    for (u32 AnimationIndex = 0; AnimationIndex < BlendSpace->ValueCount; ++AnimationIndex)
    {
        blend_space_1d_value *Value = BlendSpace->Values + AnimationIndex;

        if (Value->Weight > 0.f)
        {
            animation_state *AnimationState = &Value->AnimationState;
            AnimationState->Time = BlendSpace->NormalizedTime * AnimationState->Clip->Duration;
        }
    }
}

internal void AnimationGraphPerFrameUpdate(animation_graph *Graph, animation_graph_params Params, f32 Delta);

internal void
AnimationNodePerFrameUpdate(animation_node *Node, animation_graph_params Params, f32 Delta)
{
    Assert(Node->Weight > 0.f);

    switch (Node->Type)
    {
        case AnimationNodeType_SingleMotion:
        {
            AnimationStatePerFrameUpdate(&Node->Animation, Delta);
            break;
        }
        case AnimationNodeType_BlendSpace:
        {
            AnimationBlendSpacePerFrameUpdate(Node->BlendSpace, Params, Delta);
            break;
        }
        case AnimationNodeType_Graph:
        {
            AnimationGraphPerFrameUpdate(Node->Graph, Params, Delta);
            break;
        }
        default:
        {
            Assert(!"Invalid animation node type");
        }
    }
}

internal void
AnimationGraphPerFrameUpdate(animation_graph *Graph, animation_graph_params Params, f32 Delta)
{
    AnimationMixerPerFrameUpdate(&Graph->Mixer, Delta);

    for (u32 NodeIndex = 0; NodeIndex < Graph->NodeCount; ++NodeIndex)
    {
        animation_node *Node = Graph->Nodes + NodeIndex;

        if (Node->Weight > 0.f)
        {
            AnimationNodePerFrameUpdate(Node, Params, Delta);
        }
    }
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
CreateAnimationState(animation_clip *Clip, b32 IsLooping, b32 InPlace)
{
    animation_state Result = {};
    Result.Clip = Clip;
    Result.IsLooping = IsLooping;
    Result.InPlace = InPlace;

    return Result;
}

inline void
BuildAnimationNode(animation_node *Node, const char *Name, animation_clip *Clip, b32 IsLooping, b32 InPlace)
{
    *Node = {};
    Node->Type = AnimationNodeType_SingleMotion;
    CopyString(Name, Node->Name);
    Node->Animation = CreateAnimationState(Clip, IsLooping, InPlace);
}

inline void
BuildAnimationNode(animation_node *Node, const char *Name, blend_space_1d *BlendSpace)
{
    *Node = {};
    Node->Type = AnimationNodeType_BlendSpace;
    CopyString(Name, Node->Name);
    Node->BlendSpace = BlendSpace;
}

inline void
BuildAnimationNode(animation_node *Node, const char *Name, animation_graph *Graph)
{
    *Node = {};
    Node->Type = AnimationNodeType_Graph;
    CopyString(Name, Node->Name);
    Node->Graph = Graph;
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
    *Transition = {};
    Transition->From = From;
    Transition->To = To;
    Transition->Type = Type;
    Transition->Duration = Duration;
    Transition->TransitionNode = TransitionNode;
}

inline animation_node *
GetAnimationNode(animation_graph *Graph, char *NodeName)
{
    animation_node *Result = 0;

    for (u32 NodeIndex = 0; NodeIndex < Graph->NodeCount; ++NodeIndex)
    {
        animation_node *Node = Graph->Nodes + NodeIndex;

        if (StringEquals(Node->Name, NodeName))
        {
            Result = Node;
        }
    }

    Assert(Result);

    return Result;
}

inline void
ActivateAnimationNode(animation_graph *Graph, const char *NodeName)
{
    animation_node *Node= GetAnimationNode(Graph, (char *)NodeName);

    Graph->Active = Node;
    EnableAnimationNode(Node);
}

internal void
BuildAnimationGraph(animation_graph *Graph, animation_graph_asset *Asset, model *Model, memory_arena *Arena)
{
    *Graph = {};

    Graph->NodeCount = Asset->NodeCount;
    Graph->Nodes = PushArray(Arena, Graph->NodeCount, animation_node);

    for (u32 NodeIndex = 0; NodeIndex < Asset->NodeCount; ++NodeIndex)
    {
        animation_node *Node = Graph->Nodes + NodeIndex;
        animation_node_asset *NodeAsset = Asset->Nodes + NodeIndex;

        switch (NodeAsset->Type)
        {
            case AnimationNodeType_SingleMotion:
            {
                BuildAnimationNode(
                    Node, 
                    NodeAsset->Name, 
                    GetAnimationClip(Model, NodeAsset->Animation->AnimationClipName), 
                    NodeAsset->Animation->IsLooping, 
                    NodeAsset->Animation->InPlace
                );

                break;
            }
            case AnimationNodeType_BlendSpace:
            {
                blend_space_1d *BlendSpace = PushType(Arena, blend_space_1d);
                BlendSpace->ValueCount = NodeAsset->Blendspace->ValueCount;
                BlendSpace->Values = PushArray(Arena, BlendSpace->ValueCount, blend_space_1d_value);

                for (u32 ValueIndex = 0; ValueIndex < NodeAsset->Blendspace->ValueCount; ++ValueIndex)
                {
                    blend_space_1d_value *Value = BlendSpace->Values + ValueIndex;
                    blend_space_1d_value_asset *ValueAsset = NodeAsset->Blendspace->Values + ValueIndex;

                    Value->Value =ValueAsset->Value;
                    // todo: IsLooping/InPlace for blendspace?
                    Value->AnimationState = CreateAnimationState(GetAnimationClip(Model, ValueAsset->AnimationClipName), true, true);
                }

                BuildAnimationNode(Node, NodeAsset->Name, BlendSpace);

                break;
            }
            case AnimationNodeType_Graph:
            {
                animation_graph *NodeGraph = PushType(Arena, animation_graph);

                BuildAnimationGraph(NodeGraph, NodeAsset->Graph, Model, Arena);
                BuildAnimationNode(Node, NodeAsset->Name, NodeGraph);

                break;
            }
        }
    }

    // Transitions
    for (u32 NodeIndex = 0; NodeIndex < Graph->NodeCount; ++NodeIndex)
    {
        animation_node *Node = Graph->Nodes + NodeIndex;
        animation_node_asset *NodeAsset = Asset->Nodes + NodeIndex;

        Node->TransitionCount = NodeAsset->TransitionCount;
        Node->Transitions = PushArray(Arena, Node->TransitionCount, animation_transition);

        for (u32 TransitionIndex = 0; TransitionIndex < NodeAsset->TransitionCount; ++TransitionIndex)
        {
            animation_transition *Transition = Node->Transitions + TransitionIndex;
            animation_transition_asset *TransitionAsset = NodeAsset->Transitions + TransitionIndex;

            animation_node *From = GetAnimationNode(Graph, TransitionAsset->From);
            animation_node *To = GetAnimationNode(Graph, TransitionAsset->To);
            BuildAnimationTransition(Transition, From, To, TransitionAsset->Type, TransitionAsset->Duration);
        }
    }

    animation_node *Entry = GetAnimationNode(Graph, Asset->Entry);
    Graph->Entry = Entry;
    ActivateAnimationNode(Graph, Entry->Name);
}

internal u32
GetActiveAnimationCount(animation_graph *Graph)
{
    u32 Result = 0;

    for (u32 NodeIndex = 0; NodeIndex < Graph->NodeCount; ++NodeIndex)
    {
        animation_node *Node = Graph->Nodes + NodeIndex;

        if (Node->Weight > 0.f)
        {
            switch (Node->Type)
            {
                case AnimationNodeType_SingleMotion:
                {
                    ++Result;
                    break;
                }
                case AnimationNodeType_BlendSpace:
                {
                    for (u32 Index = 0; Index < Node->BlendSpace->ValueCount; ++Index)
                    {
                        blend_space_1d_value *Value = Node->BlendSpace->Values + Index;

                        if (Value->Weight > 0.f)
                        {
                            ++Result;
                        }
                    }

                    break;
                }
                case AnimationNodeType_Graph:
                {
                    Result += GetActiveAnimationCount(Node->Graph);
                    break;
                }
            }
        }
    }

    return Result;
}

internal void
GetActiveAnimations(animation_graph *Graph, animation_state **ActiveAnimations, u32 &ActiveAnimationIndex, f32 GraphWeight)
{
    for (u32 NodeIndex = 0; NodeIndex < Graph->NodeCount; ++NodeIndex)
    {
        animation_node *Node = Graph->Nodes + NodeIndex;

        if (Node->Weight > 0.f)
        {
            switch (Node->Type)
            {
                case AnimationNodeType_SingleMotion:
                {
                    animation_state **ActiveAnimationState = ActiveAnimations + ActiveAnimationIndex++;
                    
                    *ActiveAnimationState = &Node->Animation;
                    (*ActiveAnimationState)->Weight = GraphWeight * Node->Weight;

                    break;
                }
                case AnimationNodeType_BlendSpace:
                {
                    for (u32 Index = 0; Index < Node->BlendSpace->ValueCount; ++Index)
                    {
                        blend_space_1d_value *Value = Node->BlendSpace->Values + Index;

                        if (Value->Weight > 0.f)
                        {
                            animation_state **ActiveAnimationState = ActiveAnimations + ActiveAnimationIndex++;
                            *ActiveAnimationState = &Value->AnimationState;
                            (*ActiveAnimationState)->Weight = GraphWeight * Node->Weight * Value->Weight;
                        }
                    }

                    break;
                }
                case AnimationNodeType_Graph:
                {
                    GetActiveAnimations(Node->Graph, ActiveAnimations, ActiveAnimationIndex, Node->Weight);

                    break;
                }
            }
        }
    }
}

internal void
CalculateSkeletonPose(animation_graph *Graph, skeleton_pose *DestPose, memory_arena *Arena)
{
    u32 ActiveAnimationCount = GetActiveAnimationCount(Graph);

    if (ActiveAnimationCount > 0)
    {
        scoped_memory ScopedMemory(Arena);

        skeleton_pose *SkeletonPoses = PushArray(ScopedMemory.Arena, ActiveAnimationCount, skeleton_pose);
        animation_state **ActiveAnimations = PushArray(ScopedMemory.Arena, ActiveAnimationCount, animation_state *);
        
        u32 ActiveAnimationIndex = 0;
        GetActiveAnimations(Graph, ActiveAnimations, ActiveAnimationIndex, 1.f);
        
        // Checking weights (not necessary)
        f32 TotalWeight = 0.f;
        for (u32 ActiveAnimationIndex = 0; ActiveAnimationIndex < ActiveAnimationCount; ++ActiveAnimationIndex)
        {
            animation_state *Animation = ActiveAnimations[ActiveAnimationIndex];
            TotalWeight += Animation->Weight;
        }
        Assert(Abs(1.f - TotalWeight) < EPSILON);
        //
        
        // Creating skeleton pose for each input clip
        for (u32 SkeletonPoseIndex = 0; SkeletonPoseIndex < ActiveAnimationCount; ++SkeletonPoseIndex)
        {
            skeleton_pose *SkeletonPose = SkeletonPoses + SkeletonPoseIndex;
            SkeletonPose->Skeleton = DestPose->Skeleton;
            SkeletonPose->LocalJointPoses = PushArray(ScopedMemory.Arena, DestPose->Skeleton->JointCount, joint_pose);

            for (u32 JointIndex = 0; JointIndex < DestPose->Skeleton->JointCount; ++JointIndex)
            {
                joint_pose *LocalJointPose = SkeletonPose->LocalJointPoses + JointIndex;
                joint_pose *SkeletonLocalJointPose = DestPose->LocalJointPoses + JointIndex;

                *LocalJointPose = *SkeletonLocalJointPose;
            }
        }

        // todo: multithreading?
        // Extracting skeleton pose from each input clip
        for (u32 AnimationIndex = 0; AnimationIndex < ActiveAnimationCount; ++AnimationIndex)
        {
            animation_state *AnimationState = ActiveAnimations[AnimationIndex];
            skeleton_pose *SkeletonPose = SkeletonPoses + AnimationIndex;

            AnimateSkeletonPose(SkeletonPose, AnimationState->Clip, AnimationState->InPlace, AnimationState->Time);
        }

        // Lerping between all skeleton poses
        skeleton_pose *Pose = First(SkeletonPoses);
        animation_state *Animation = *First(ActiveAnimations);

        Lerp(Pose, 0.f, Pose, DestPose);

        f32 AccumulatedWeight = Animation->Weight;
        u32 CurrentPoseIndex = 1;

        while (CurrentPoseIndex < ActiveAnimationCount)
        {
            skeleton_pose *NextPose = SkeletonPoses + CurrentPoseIndex;
            animation_state *NextAnimation = ActiveAnimations[CurrentPoseIndex];
            ++CurrentPoseIndex;

            f32 NextWeight = NextAnimation->Weight;

            f32 t = NextWeight / (AccumulatedWeight + NextWeight);

            Assert(t >= 0.f && t <= 1.f);

            Lerp(Pose, t, NextPose, DestPose);

            Pose = DestPose;
            Animation = NextAnimation;
            AccumulatedWeight += NextWeight;
        }

        // Clearing weights (not necessary?)
        for (u32 ActiveAnimationIndex = 0; ActiveAnimationIndex < ActiveAnimationCount; ++ActiveAnimationIndex)
        {
            animation_state *Animation = ActiveAnimations[ActiveAnimationIndex];
            Animation->Weight = 0.f;
        }
    }
}

internal void
PlayerAnimationControllerUpdate(animation_graph *Graph, game_input *Input, random_sequence *Entropy, f32 MaxTime, f32 Delta)
{
    animation_node *Active = Graph->Active;

    f32 MoveMaginute = Clamp(Magnitude(Input->Move.Range), 0.f, 1.f);

    if (StringEquals(Active->Name, "StateIdle"))
    {
        // StateIdle sub-graph
        animation_graph *StateIdleGraph = Active->Graph;
        animation_node *Active = StateIdleGraph->Active;

        if (StringEquals(Active->Name, "StateIdle_0"))
        {
            StateIdleGraph->Time += Delta;

            if (StateIdleGraph->Time > MaxTime)
            {
                StateIdleGraph->Time = 0.f;

                if (Random01(Entropy) > 0.5f)
                {
                    TransitionToNode(StateIdleGraph, "StateIdle_1");
                }
                else
                {
                    TransitionToNode(StateIdleGraph, "StateIdle_2");
                }
            }
        }
        else if (StringEquals(Active->Name, "StateIdle_1"))
        {
            if (Active->Animation.Time >= Active->Animation.Clip->Duration)
            {
                TransitionToNode(StateIdleGraph, "StateIdle_0");
            }
        }
        else if (StringEquals(Active->Name, "StateIdle_2"))
        {
            if (Active->Animation.Time >= Active->Animation.Clip->Duration)
            {
                TransitionToNode(StateIdleGraph, "StateIdle_0");
            }
        }
        else
        {
            Assert(!"Invalid state");
        }

        // External transitions
        if (Input->Crouch.IsActivated)
        {
            StateIdleGraph->Time = 0.f;
            TransitionToNode(Graph, "StateDancing");
        }

        if (MoveMaginute > 0.f)
        {
            StateIdleGraph->Time = 0.f;
            TransitionToNode(Graph, "StateWalking");
        }
    }
    else if (StringEquals(Active->Name, "StateWalking"))
    {
        if (MoveMaginute < EPSILON)
        {
            TransitionToNode(Graph, "StateIdle");
        }
    }
    else if (StringEquals(Active->Name, "StateDancing"))
    {
        if (Input->Crouch.IsActivated && MoveMaginute < EPSILON)
        {
            TransitionToNode(Graph, "StateIdle");
        }
    }
    else
    {
        Assert(!"Invalid state");
    }
}
