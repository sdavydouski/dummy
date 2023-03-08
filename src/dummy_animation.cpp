#include "dummy.h"

inline transform *
GetRootLocalJointPose(skeleton_pose *Pose)
{
    transform *Result = Pose->LocalJointPoses + ROOT_POSE_INDEX;
    return Result;
}

inline transform *
GetRootTranslationLocalJointPose(skeleton_pose *Pose)
{
    transform *Result = Pose->LocalJointPoses + ROOT_TRANSLATION_POSE_INDEX;
    return Result;
}

inline transform
Lerp(transform From, f32 t, transform To)
{
    transform Result = {};

    Result.Translation = Lerp(From.Translation, t, To.Translation);
    Result.Rotation = Slerp(From.Rotation, t, To.Rotation);
    Result.Scale = Lerp(From.Scale, t, To.Scale);

    Assert(IsFinite(Result));

    return Result;
}

inline transform
Accumulate(transform Transform, transform AdditiveTransform)
{
    transform Result = {};

    Result.Rotation = AdditiveTransform.Rotation * Transform.Rotation;
    Result.Translation = AdditiveTransform.Translation + Transform.Translation;
    Result.Scale = (AdditiveTransform.Scale + vec3(1.f)) * Transform.Scale;

    Assert(IsFinite(Result));

    return Result;
}

inline transform
Accumulate(transform Transform, transform AdditiveTransform, f32 BlendWeight)
{
    Assert(BlendWeight >= 0.f && BlendWeight <= 1.f);

    transform AccumulatedTransform = Accumulate(Transform, AdditiveTransform);
    transform Result = Lerp(Transform, BlendWeight, AccumulatedTransform);

    return Result;
}

dummy_internal void
Lerp(skeleton_pose *From, f32 t, skeleton_pose *To, skeleton_pose *Dest)
{
    u32 JointCount = Dest->Skeleton->JointCount;

    Assert(From->Skeleton->JointCount == JointCount);
    Assert(To->Skeleton->JointCount == JointCount);

    for (u32 JointIndex = 0; JointIndex < JointCount; ++JointIndex)
    {
        transform FromPose = From->LocalJointPoses[JointIndex];
        transform ToPose = To->LocalJointPoses[JointIndex];
        transform *DestPose = Dest->LocalJointPoses + JointIndex;

        *DestPose = Lerp(FromPose, t, ToPose);
    }
}

dummy_internal void
Accumulate(skeleton_pose *PoseA, skeleton_pose *PoseB, f32 BlendWeight, skeleton_pose *Dest)
{
    u32 JointCount = Dest->Skeleton->JointCount;

    Assert(PoseA->Skeleton->JointCount == JointCount);
    Assert(PoseB->Skeleton->JointCount == JointCount);

    for (u32 JointIndex = 0; JointIndex < JointCount; ++JointIndex)
    {
        transform TransformA = PoseA->LocalJointPoses[JointIndex];
        transform TransformB = PoseB->LocalJointPoses[JointIndex];
        transform *TransformDest = Dest->LocalJointPoses + JointIndex;

        *TransformDest = Accumulate(TransformA, TransformB, BlendWeight);
    }
}

dummy_internal void
Blend(animation_blend_mode BlendMode, f32 BlendWeight, f32 AnimationWeight, skeleton_pose *PoseA, skeleton_pose *PoseB, skeleton_pose *Dest)
{
    switch (BlendMode)
    {
        case BlendMode_Normal:
        {
            Lerp(PoseA, BlendWeight, PoseB, Dest);
            break;
        }
        case BlendMode_Additive:
        {
            Accumulate(PoseA, PoseB, AnimationWeight, Dest);
            break;
        }
        default:
        {
            Assert(!"Invalid blend mode");
            break;
        }
    }
}

dummy_internal mat4
CalculateGlobalJointPose(joint *CurrentJoint, transform *CurrentJointPose, skeleton_pose *Pose)
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

dummy_internal transform
CalculateAdditiveTransform(transform TargetTransform, transform BaseTransform)
{
    transform Result = {};

    Result.Rotation = Normalize(TargetTransform.Rotation * Inverse(BaseTransform.Rotation));
    Result.Translation = TargetTransform.Translation - BaseTransform.Translation;
    // In order to support blending between different additive scales, we save [(target scale)/(source scale) - 1.f], 
    // and this can blend with other delta scale value
    Result.Scale = TargetTransform.Scale * SafeReciprocal(BaseTransform.Scale) - vec3(1.f);

    Assert(IsFinite(Result));

    return Result;
}

inline bool32
AnimationClipFinished(animation_state Animation)
{
    bool32 Result = (Animation.Time >= Animation.Clip->Duration);
    return Result;
}

inline bool32
AdditiveAnimationsFinished(animation_node *Node)
{
    bool32 Result = true;

    for (u32 AdditiveAnimationIndex = 0; AdditiveAnimationIndex < Node->AdditiveAnimationCount; ++AdditiveAnimationIndex)
    {
        additive_animation *Additive = Node->AdditiveAnimations + AdditiveAnimationIndex;

        if (!AnimationClipFinished(Additive->Animation))
        {
            Result = false;
            break;
        }
    }

    return Result;
}

// todo: optimize
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

dummy_internal void
FindClosestKeyFrames(animation_sample *PoseSample, f32 CurrentTime, key_frame **Prev, key_frame **Next)
{
    if (PoseSample->KeyFrameCount > 1)
    {
        bool32 Found = false;

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
            *Prev = *Next = Last(PoseSample->KeyFrames, PoseSample->KeyFrameCount);
        }
    }
    else
    {
        *Prev = *Next = First(PoseSample->KeyFrames);
    }
}

dummy_internal void
AnimateSkeletonPose(animation_graph *Graph, skeleton_pose *SkeletonPose, animation_state *Animation)
{
    transform *RootTranslationPose = GetRootTranslationLocalJointPose(SkeletonPose);

    vec3 TranslationBefore = Animation->PrevTranslation;

    for (u32 JointIndex = 0; JointIndex < SkeletonPose->Skeleton->JointCount; ++JointIndex)
    {
        transform *LocalJointPose = SkeletonPose->LocalJointPoses + JointIndex;

        animation_sample *PoseSample = GetAnimationSampleByJointIndex(Animation->Clip, JointIndex);

        if (PoseSample)
        {
            f32 Time = Animation->Time;

            key_frame *PrevKeyFrame = 0;
            key_frame *NextKeyFrame = 0;
            FindClosestKeyFrames(PoseSample, Time, &PrevKeyFrame, &NextKeyFrame);

            f32 t = 0.f;

            if (PoseSample->KeyFrameCount > 1 && PrevKeyFrame != NextKeyFrame)
            {
                t = (Time - PrevKeyFrame->Time) / (Abs(NextKeyFrame->Time - PrevKeyFrame->Time));
            }

            Assert(t >= 0.f && t <= 1.f);
            
            *LocalJointPose = Lerp(PrevKeyFrame->Pose, t, NextKeyFrame->Pose);

            Assert(IsFinite(*LocalJointPose));
        }
    }

    for (u32 EventIndex = 0; EventIndex < Animation->Clip->EventCount; ++EventIndex)
    {
        animation_event *Event = Animation->Clip->Events + EventIndex;

        // Reset events when animation starts over
        if ((Animation->Time - Animation->PrevTime) < 0.f)
        {
            Event->IsFired = false;
        }

        if (Animation->Time >= Event->Time && !Event->IsFired)
        {
            Event->IsFired = true;

            animation_footstep_event *FootstepEvent = PushType(&Graph->EventList->Arena, animation_footstep_event);
            FootstepEvent->EntityId = Graph->EntityId;
            FootstepEvent->Weight = Animation->Weight;
            PublishEvent(Graph->EventList, Event->Name, FootstepEvent);
        }
    }

    vec3 TranslationAfter = RootTranslationPose->Translation;

    if (Animation->EnableRootMotion)
    {
        vec3 RootMotion = TranslationAfter - TranslationBefore;

        // Loop case
        if ((Animation->Time - Animation->PrevTime) < 0.f)
        {
            animation_sample* PoseSample = GetAnimationSampleByJointIndex(Animation->Clip, ROOT_TRANSLATION_POSE_INDEX);
            key_frame* FirstKeyFrame = First(PoseSample->KeyFrames);
            key_frame* LastKeyFrame = Last(PoseSample->KeyFrames, PoseSample->KeyFrameCount);

            vec3 FirstTranslation = FirstKeyFrame->Pose.Translation;
            vec3 LastTranslation = LastKeyFrame->Pose.Translation;

            vec3 d1 = LastTranslation - TranslationBefore;
            vec3 d2 = TranslationAfter - FirstTranslation;

            RootMotion = d1 + d2;
        }

        SkeletonPose->RootMotion = RootMotion;

        Animation->PrevTranslation = TranslationAfter;

        RootTranslationPose->Translation.x = 0.f;
        RootTranslationPose->Translation.z = 0.f;
    }
}

inline void
EnableAnimationNode(animation_node *Node)
{
    Node->Weight = 1.f;
}

dummy_internal void
DisableAnimationNode(animation_node *Node, animation_node *ToNode = 0)
{
    Node->Weight = 0.f;

    switch (Node->Type)
    {
        case AnimationNodeType_Clip:
        {
            Node->Animation.Time = 0.f;

            break;
        }
        case AnimationNodeType_BlendSpace:
        {
            for (u32 ValueIndex = 0; ValueIndex < Node->BlendSpace->ValueCount; ++ValueIndex)
            {
                blend_space_1d_value *Value = Node->BlendSpace->Values + ValueIndex;

                Value->Weight = 0.f;
                Value->AnimationState.Time = 0.f;
            }

            break;
        }
        case AnimationNodeType_Reference:
        {
            if (Node->Reference != ToNode)
            {
                DisableAnimationNode(Node->Reference);
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
            Node->Graph->State = {};
            Node->Graph->Active = Node->Graph->Entry;
            EnableAnimationNode(Node->Graph->Entry);

            break;
        }
    }

    for (u32 AdditiveAnimationIndex = 0; AdditiveAnimationIndex < Node->AdditiveAnimationCount; ++AdditiveAnimationIndex)
    {
        additive_animation *Additive = Node->AdditiveAnimations + AdditiveAnimationIndex;
        Additive->Animation.Time = 0.f;
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

inline void
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

            EnableAnimationNode(Mixer->FadeIn);

            Mixer->FadeIn = 0;
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

            Mixer->FadeOut = 0;
        }
    }
}

inline animation_transition *
GetAnimationTransition(animation_node *Node, const char *Name)
{
    animation_transition *Result = 0;

    for (u32 TransitionIndex = 0; TransitionIndex < Node->TransitionCount; ++TransitionIndex)
    {
        animation_transition *Transition = Node->Transitions + TransitionIndex;

        if (StringEquals(Transition->To->Name, Name))
        {
            Result = Transition;
            break;
        }
    }

    Assert(Result);

    return Result;
}

inline bool32
AllowTransition(animation_graph *Graph, const char *NodeName)
{
    bool32 Result = true;

    if (Graph->Mixer.FadeIn && Graph->Mixer.FadeOut)
    {
        char *FromNodeName = Graph->Mixer.FadeOut->Name;
        char *ToNodeName = Graph->Mixer.FadeIn->Name;

        Result = StringEquals(NodeName, FromNodeName) || StringEquals(NodeName, ToNodeName);
    }

    return Result;
}

dummy_internal void
TransitionToNode(animation_graph *Graph, const char *NodeName)
{
    if (AllowTransition(Graph, NodeName))
    {
        animation_node *FromNode = Graph->Active;
        animation_transition *Transition = GetAnimationTransition(FromNode, NodeName);

        switch (Transition->Type)
        {
            case AnimationTransitionType_Immediate:
            {
                animation_node *ToNode = Transition->To;
                Graph->Active = ToNode;

                DisableAnimationNode(FromNode, ToNode);
                EnableAnimationNode(ToNode);

                break;
            }
            case AnimationTransitionType_Crossfade:
            {
                animation_node *ToNode = Transition->To;
                Graph->Active = ToNode;

                CrossFade(&Graph->Mixer, FromNode, ToNode, Transition->Duration);

                break;
            }
            case AnimationTransitionType_Transitional:
            {
                animation_node *ToNode = Transition->TransitionNode;
                Graph->Active = ToNode;

                CrossFade(&Graph->Mixer, FromNode, ToNode, Transition->Duration);

                break;
            }
        }
    }
}

dummy_internal void
AnimationStatePerFrameUpdate(animation_state *AnimationState, f32 Delta)
{
    AnimationState->PrevTime = AnimationState->Time;
    AnimationState->Time += Delta;

    if (AnimationState->Time > AnimationState->Clip->Duration)
    {
        if (AnimationState->IsLooping)
        {
            AnimationState->Time = AnimationState->Time - AnimationState->Clip->Duration;
        }
        else
        {
            AnimationState->Time = AnimationState->Clip->Duration;
        }
    }
}

dummy_internal void
LinearLerpBlend(blend_space_1d *BlendSpace)
{
    Assert(BlendSpace->ValueCount > 1);

    blend_space_1d_value *BlendValueFrom = 0;
    blend_space_1d_value *BlendValueTo = 0;

    for (u32 AnimationBlendValueIndex = 0; AnimationBlendValueIndex < BlendSpace->ValueCount - 1; ++AnimationBlendValueIndex)
    {
        blend_space_1d_value *CurrentAnimationBlendValue = BlendSpace->Values + AnimationBlendValueIndex;
        blend_space_1d_value *NextAnimationBlendValue = BlendSpace->Values + AnimationBlendValueIndex + 1;

        if (InRange(BlendSpace->Parameter, CurrentAnimationBlendValue->Value, NextAnimationBlendValue->Value))
        {
            BlendValueFrom = CurrentAnimationBlendValue;
            BlendValueTo = NextAnimationBlendValue;
            break;
        }
    }

    Assert(BlendValueFrom && BlendValueTo);

    f32 t = (BlendSpace->Parameter - BlendValueFrom->Value) / (BlendValueTo->Value - BlendValueFrom->Value);

    Assert(t >= 0.f && t <= 1.f);

    if (NearlyEqual(t, 0.f))
    {
        t = 0.f;
    }
    else if (NearlyEqual(t, 1.f))
    {
        t = 1.f;
    }

    BlendValueFrom->Weight = 1.f - t;
    BlendValueTo->Weight = t;
}

dummy_internal void
AnimationBlendSpacePerFrameUpdate(blend_space_1d *BlendSpace, f32 Delta)
{
    // Clearing weights first
    for (u32 AnimationBlendValueIndex = 0; AnimationBlendValueIndex < BlendSpace->ValueCount; ++AnimationBlendValueIndex)
    {
        blend_space_1d_value *AnimationBlendValue = BlendSpace->Values + AnimationBlendValueIndex;
        AnimationBlendValue->Weight = 0.f;
    }

    LinearLerpBlend(BlendSpace);

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
        BlendSpace->NormalizedTime = BlendSpace->NormalizedTime - 1.f;
    }

    for (u32 AnimationIndex = 0; AnimationIndex < BlendSpace->ValueCount; ++AnimationIndex)
    {
        blend_space_1d_value *Value = BlendSpace->Values + AnimationIndex;

        if (Value->Weight > 0.f)
        {
            animation_state *AnimationState = &Value->AnimationState;
            AnimationState->PrevTime = AnimationState->Time;
            AnimationState->Time = BlendSpace->NormalizedTime * AnimationState->Clip->Duration;
        }
    }
}

void AnimationNodePerFrameUpdate(animation_node *Node, f32 Delta);
void AnimationGraphPerFrameUpdate(animation_graph *Graph, f32 Delta);

dummy_internal void
AnimationNodePerFrameUpdate(animation_node *Node, f32 Delta)
{
    switch (Node->Type)
    {
        case AnimationNodeType_Clip:
        {
            AnimationStatePerFrameUpdate(&Node->Animation, Delta);
            break;
        }
        case AnimationNodeType_BlendSpace:
        {
            AnimationBlendSpacePerFrameUpdate(Node->BlendSpace, Delta);
            break;
        }
        case AnimationNodeType_Reference:
        {
            AnimationNodePerFrameUpdate(Node->Reference, Delta);
            break;
        }
        case AnimationNodeType_Graph:
        {
            AnimationGraphPerFrameUpdate(Node->Graph, Delta);
            break;
        }
    }

    for (u32 AdditiveAnimationIndex = 0; AdditiveAnimationIndex < Node->AdditiveAnimationCount; ++AdditiveAnimationIndex)
    {
        additive_animation *Additive = Node->AdditiveAnimations + AdditiveAnimationIndex;

        if (Additive->Weight > 0.f)
        {
            AnimationStatePerFrameUpdate(&Additive->Animation, Delta);
        }
    }
}

dummy_internal void
AnimationGraphPerFrameUpdate(animation_graph *Graph, f32 Delta)
{
    AnimationMixerPerFrameUpdate(&Graph->Mixer, Delta);

    for (u32 NodeIndex = 0; NodeIndex < Graph->NodeCount; ++NodeIndex)
    {
        animation_node *Node = Graph->Nodes + NodeIndex;

        if (Node->Weight > 0.f)
        {
            AnimationNodePerFrameUpdate(Node, Delta);
        }
    }
}

dummy_internal animation_clip *
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
CreateAnimationState(animation_clip *Clip, bool32 IsLooping, bool32 EnableRootMotion, animation_blend_mode BlendMode = BlendMode_Normal)
{
    animation_state Result = {};
    Result.Clip = Clip;
    Result.IsLooping = IsLooping;
    Result.EnableRootMotion = EnableRootMotion;
    Result.BlendMode = BlendMode;

    return Result;
}

inline void
BuildAnimationNode(animation_node *Node, const char *Name, animation_clip *Clip, bool32 IsLooping, bool32 EnableRootMotion)
{
    CopyString(Name, Node->Name);
    Node->Type = AnimationNodeType_Clip;
    Node->Animation = CreateAnimationState(Clip, IsLooping, EnableRootMotion);
}

inline void
BuildAdditiveAnimation(additive_animation *Additive, animation_clip *Target, animation_clip *Base, u32 BaseKeyFrameIndex, bool32 IsLooping, memory_arena *Arena)
{
    Assert(Target->PoseSampleCount == Base->PoseSampleCount);

    Additive->Target = Target;
    Additive->Base = Base;
    Additive->BaseKeyFrameIndex = BaseKeyFrameIndex;

    animation_clip *AdditiveClip = PushType(Arena, animation_clip);

    char AdditiveClipName[256];
    FormatString(AdditiveClipName, "%s::additive", Target->Name);
    CopyString(AdditiveClipName, AdditiveClip->Name);

    AdditiveClip->Duration = Target->Duration;
    AdditiveClip->PoseSampleCount = Target->PoseSampleCount;
    AdditiveClip->PoseSamples = PushArray(Arena, AdditiveClip->PoseSampleCount, animation_sample);

    Additive->Weight = 1.f;
    Additive->Animation = CreateAnimationState(AdditiveClip, IsLooping, false, BlendMode_Additive);

    for (u32 PoseSampleIndex = 0; PoseSampleIndex < AdditiveClip->PoseSampleCount; ++PoseSampleIndex)
    {
        animation_sample *AdditiveSample = AdditiveClip->PoseSamples + PoseSampleIndex;
        animation_sample *TargetSample = Target->PoseSamples + PoseSampleIndex;
        animation_sample *BaseSample = Base->PoseSamples + PoseSampleIndex;

        AdditiveSample->JointIndex = TargetSample->JointIndex;
        AdditiveSample->KeyFrameCount = TargetSample->KeyFrameCount;
        AdditiveSample->KeyFrames = PushArray(Arena, AdditiveSample->KeyFrameCount, key_frame);

        for (u32 KeyFrameIndex = 0; KeyFrameIndex < AdditiveSample->KeyFrameCount; ++KeyFrameIndex)
        {
            key_frame *AdditiveKeyFrame = AdditiveSample->KeyFrames + KeyFrameIndex;
            key_frame *TargetKeyFrame = TargetSample->KeyFrames + KeyFrameIndex;
            key_frame *BaseKeyFrame = BaseSample->KeyFrames + BaseKeyFrameIndex;

            AdditiveKeyFrame->Time = TargetKeyFrame->Time;
            AdditiveKeyFrame->Pose = CalculateAdditiveTransform(TargetKeyFrame->Pose, BaseKeyFrame->Pose);
        }
    }
}

inline void
BuildAnimationNode(animation_node *Node, const char *Name, blend_space_1d *BlendSpace)
{
    CopyString(Name, Node->Name);
    Node->Type = AnimationNodeType_BlendSpace;
    Node->BlendSpace = BlendSpace;
}

inline void
BuildAnimationNode(animation_node *Node, const char *Name, animation_node *Reference)
{
    CopyString(Name, Node->Name);
    Node->Type = AnimationNodeType_Reference;
    Node->Reference = Reference;
}

inline void
BuildAnimationNode(animation_node *Node, const char *Name, animation_graph *Graph)
{
    CopyString(Name, Node->Name);
    Node->Type = AnimationNodeType_Graph;
    Node->Graph = Graph;
}

inline void
BuildAnimationTransition(
    animation_transition *Transition, 
    animation_node *From, 
    animation_node *To
)
{
    Transition->Type = AnimationTransitionType_Immediate;
    Transition->From = From;
    Transition->To = To;
}

inline void
BuildAnimationTransition(
    animation_transition *Transition,
    animation_node *From,
    animation_node *To,
    f32 Duration
)
{
    Transition->Type = AnimationTransitionType_Crossfade;
    Transition->From = From;
    Transition->To = To;
    Transition->Duration = Duration;
}

inline void
BuildAnimationTransition(
    animation_transition *Transition,
    animation_node *From,
    animation_node *To,
    animation_node *TransitionNode,
    f32 Duration
)
{
    Transition->Type = AnimationTransitionType_Transitional;
    Transition->From = From;
    Transition->To = To;
    Transition->TransitionNode = TransitionNode;
    Transition->Duration = Duration;
}

inline animation_node *
GetAnimationNode(animation_graph *Graph, const char *NodeName)
{
    animation_node *Result = 0;

    for (u32 NodeIndex = 0; NodeIndex < Graph->NodeCount; ++NodeIndex)
    {
        animation_node *Node = Graph->Nodes + NodeIndex;

        if (StringEquals(Node->Name, NodeName))
        {
            Result = Node;
            break;
        }
    }

    Assert(Result);

    return Result;
}

dummy_internal void
BuildAnimationGraph(animation_graph *Root, animation_graph *Graph, animation_graph_asset *Asset, model *Model, u32 EntityId, game_event_list *EventList, memory_arena *Arena)
{
    Graph->NodeCount = Asset->NodeCount;
    Graph->Nodes = PushArray(Arena, Graph->NodeCount, animation_node);
    CopyString(Asset->Animator, Graph->Animator);

    for (u32 NodeIndex = 0; NodeIndex < Asset->NodeCount; ++NodeIndex)
    {
        animation_node *Node = Graph->Nodes + NodeIndex;
        animation_node_asset *NodeAsset = Asset->Nodes + NodeIndex;

        switch (NodeAsset->Type)
        {
            case AnimationNodeType_Clip:
            {
                BuildAnimationNode(
                    Node,
                    NodeAsset->Name,
                    GetAnimationClip(Model, NodeAsset->Animation->AnimationClipName),
                    NodeAsset->Animation->IsLooping,
                    NodeAsset->Animation->EnableRootMotion
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
                    Value->AnimationState = CreateAnimationState(GetAnimationClip(Model, ValueAsset->AnimationClipName), true, ValueAsset->EnableRootMotion);
                }

                BuildAnimationNode(Node, NodeAsset->Name, BlendSpace);

                break;
            }
            case AnimationNodeType_Reference:
            {
                CopyString(NodeAsset->Name, Node->Name);
                Node->Type = AnimationNodeType_Reference;
                Node->Reference = GetAnimationNode(Graph, NodeAsset->Reference->NodeName);

                break;
            }
            case AnimationNodeType_Graph:
            {
                animation_graph *NodeGraph = PushType(Arena, animation_graph);

                BuildAnimationGraph(Root, NodeGraph, NodeAsset->Graph, Model, EntityId, EventList, Arena);
                BuildAnimationNode(Node, NodeAsset->Name, NodeGraph);

                break;
            }
        }
    }

    
    for (u32 NodeIndex = 0; NodeIndex < Graph->NodeCount; ++NodeIndex)
    {
        animation_node *Node = Graph->Nodes + NodeIndex;
        animation_node_asset *NodeAsset = Asset->Nodes + NodeIndex;

        // Transitions
        Node->TransitionCount = NodeAsset->TransitionCount;
        Node->Transitions = PushArray(Arena, Node->TransitionCount, animation_transition);

        for (u32 TransitionIndex = 0; TransitionIndex < NodeAsset->TransitionCount; ++TransitionIndex)
        {
            animation_transition *Transition = Node->Transitions + TransitionIndex;
            animation_transition_asset *TransitionAsset = NodeAsset->Transitions + TransitionIndex;

            animation_node *From = GetAnimationNode(Graph, TransitionAsset->From);
            animation_node *To = GetAnimationNode(Graph, TransitionAsset->To);

            switch (TransitionAsset->Type)
            {
                case AnimationTransitionType_Immediate:
                {
                    BuildAnimationTransition(Transition, From, To);
                    break;
                }
                case AnimationTransitionType_Crossfade:
                {
                    BuildAnimationTransition(Transition, From, To, TransitionAsset->Duration);
                    break;
                }
                case AnimationTransitionType_Transitional:
                {
                    animation_node *TransitionNode = GetAnimationNode(Graph, TransitionAsset->TransitionNode);
                    BuildAnimationTransition(Transition, From, To, TransitionNode, TransitionAsset->Duration);

                    break;
                }
            }
        }

        // Additive animations
        Node->AdditiveAnimationCount = NodeAsset->AdditiveAnimationCount;
        Node->AdditiveAnimations = PushArray(Arena, Node->AdditiveAnimationCount, additive_animation);

        for (u32 AdditiveAnimationIndex = 0; AdditiveAnimationIndex < NodeAsset->AdditiveAnimationCount; ++AdditiveAnimationIndex)
        {
            additive_animation *Additive = Node->AdditiveAnimations + AdditiveAnimationIndex;
            additive_animation_asset *AdditiveAsset = NodeAsset->AdditiveAnimations + AdditiveAnimationIndex;

            BuildAdditiveAnimation(
                Additive, 
                GetAnimationClip(Model, AdditiveAsset->TargetClipName), 
                GetAnimationClip(Model, AdditiveAsset->BaseClipName),
                AdditiveAsset->BaseKeyFrameIndex,
                AdditiveAsset->IsLooping,
                Arena
            );
        }
    }

    animation_node *Entry = GetAnimationNode(Graph, Asset->Entry);
    EnableAnimationNode(Entry);

    Graph->Entry = Entry;
    Graph->Active = Entry;

    Graph->EntityId = EntityId;
    Graph->EventList = EventList;
}

dummy_internal u32 GetActiveAnimationCount(animation_graph *Graph);
dummy_internal void GetActiveAnimations(animation_graph *Graph, animation_state **ActiveAnimations, u32 &ActiveAnimationIndex, f32 GraphWeight);

dummy_internal u32
GetActiveAnimationCount(animation_node *Node, f32 Weight = 0.f)
{
    f32 NodeWeight = Max(Node->Weight, Weight);

    Assert(NodeWeight > 0);

    u32 Result = 0;

    switch (Node->Type)
    {
        case AnimationNodeType_Clip:
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
        case AnimationNodeType_Reference:
        {
            Result += GetActiveAnimationCount(Node->Reference, 1.f);

            break;
        }
        case AnimationNodeType_Graph:
        {
            Result += GetActiveAnimationCount(Node->Graph);
            break;
        }
    }

    for (u32 AdditiveAnimationIndex = 0; AdditiveAnimationIndex < Node->AdditiveAnimationCount; ++AdditiveAnimationIndex)
    {
        additive_animation *Additive = Node->AdditiveAnimations + AdditiveAnimationIndex;

        if (Additive->Weight > 0.f)
        {
            Result += 1;
        }
    }

    return Result;
}

dummy_internal u32
GetActiveAnimationCount(animation_graph *Graph)
{
    u32 Result = 0;

    for (u32 NodeIndex = 0; NodeIndex < Graph->NodeCount; ++NodeIndex)
    {
        animation_node *Node = Graph->Nodes + NodeIndex;

        if (Node->Weight > 0.f)
        {
            Result += GetActiveAnimationCount(Node);
        }
    }

    return Result;
}

dummy_internal void
GetActiveAnimations(animation_node *Node, animation_state **ActiveAnimations, u32 &ActiveAnimationIndex, f32 GraphWeight, f32 NodeWeight = 0.f)
{
    f32 NodeWeightToUse = Max(Node->Weight, NodeWeight);

    Assert(NodeWeightToUse > 0.f)

    switch (Node->Type)
    {
        case AnimationNodeType_Clip:
        {
            animation_state **ActiveAnimationState = ActiveAnimations + ActiveAnimationIndex++;

            *ActiveAnimationState = &Node->Animation;
            (*ActiveAnimationState)->Weight = GraphWeight * NodeWeightToUse;

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
                    (*ActiveAnimationState)->Weight = GraphWeight * Value->Weight * NodeWeightToUse;
                }
            }

            break;
        }
        case AnimationNodeType_Reference:
        {
            GetActiveAnimations(Node->Reference, ActiveAnimations, ActiveAnimationIndex, GraphWeight * NodeWeightToUse, 1.f);

            break;
        }
        case AnimationNodeType_Graph:
        {
            GetActiveAnimations(Node->Graph, ActiveAnimations, ActiveAnimationIndex, GraphWeight * NodeWeightToUse);

            break;
        }
    }

    for (u32 AdditiveAnimationIndex = 0; AdditiveAnimationIndex < Node->AdditiveAnimationCount; ++AdditiveAnimationIndex)
    {
        additive_animation *Additive = Node->AdditiveAnimations + AdditiveAnimationIndex;

        if (Additive->Weight > 0.f)
        {
            animation_state **ActiveAnimationState = ActiveAnimations + ActiveAnimationIndex++;
            *ActiveAnimationState = &Additive->Animation;
            (*ActiveAnimationState)->Weight = GraphWeight * NodeWeightToUse * Additive->Weight;
        }
    }
}

dummy_internal void
GetActiveAnimations(animation_graph *Graph, animation_state **ActiveAnimations, u32 &ActiveAnimationIndex, f32 GraphWeight = 1.f)
{
    for (u32 NodeIndex = 0; NodeIndex < Graph->NodeCount; ++NodeIndex)
    {
        animation_node *Node = Graph->Nodes + NodeIndex;

        if (Node->Weight > 0.f)
        {
            GetActiveAnimations(Node, ActiveAnimations, ActiveAnimationIndex, GraphWeight);
        }
    }
}

dummy_internal void
CalculateSkeletonPose(animation_graph *Graph, skeleton_pose *BindPose, skeleton_pose *DestPose, memory_arena *Arena)
{
    u32 ActiveAnimationCount = GetActiveAnimationCount(Graph);

    if (ActiveAnimationCount > 0)
    {
        scoped_memory ScopedMemory(Arena);

        skeleton_pose *SkeletonPoses = PushArray(ScopedMemory.Arena, ActiveAnimationCount, skeleton_pose);
        animation_state **ActiveAnimations = PushArray(ScopedMemory.Arena, ActiveAnimationCount, animation_state *);
        
        u32 ActiveAnimationIndex = 0;
        GetActiveAnimations(Graph, ActiveAnimations, ActiveAnimationIndex);
        
#if 1
        // Checking weights (not necessary)
        f32 TotalWeight = 0.f;
        for (u32 ActiveAnimationIndex = 0; ActiveAnimationIndex < ActiveAnimationCount; ++ActiveAnimationIndex)
        {
            animation_state *Animation = ActiveAnimations[ActiveAnimationIndex];

            if (Animation->BlendMode == BlendMode_Normal)
            {
                TotalWeight += Animation->Weight;
            }
        }
        Assert(NearlyEqual(TotalWeight, 1.f));
#endif
        
        // Creating skeleton pose for each input clip
        for (u32 SkeletonPoseIndex = 0; SkeletonPoseIndex < ActiveAnimationCount; ++SkeletonPoseIndex)
        {
            skeleton_pose *SkeletonPose = SkeletonPoses + SkeletonPoseIndex;
            animation_state *Animation = ActiveAnimations[SkeletonPoseIndex];

            SkeletonPose->Skeleton = BindPose->Skeleton;
            SkeletonPose->LocalJointPoses = PushArray(ScopedMemory.Arena, BindPose->Skeleton->JointCount, transform);

            for (u32 JointIndex = 0; JointIndex < BindPose->Skeleton->JointCount; ++JointIndex)
            {
                transform *LocalJointPose = SkeletonPose->LocalJointPoses + JointIndex;
                transform *SkeletonLocalJointPose = BindPose->LocalJointPoses + JointIndex;

                switch (Animation->BlendMode)
                {
                    case BlendMode_Normal:
                    {
                        *LocalJointPose = *SkeletonLocalJointPose;
                        
                        break;
                    }
                    case BlendMode_Additive:
                    {
                        LocalJointPose->Rotation = quat(0.f, 0.f, 0.f, 1.f);
                        LocalJointPose->Translation = vec3(0.f);
                        LocalJointPose->Scale = vec3(0.f);

                        break;
                    }
                }
            }
        }

        // Extracting skeleton pose from each input clip
        for (u32 AnimationIndex = 0; AnimationIndex < ActiveAnimationCount; ++AnimationIndex)
        {
            animation_state *AnimationState = ActiveAnimations[AnimationIndex];
            skeleton_pose *SkeletonPose = SkeletonPoses + AnimationIndex;

            AnimateSkeletonPose(Graph, SkeletonPose, AnimationState);
        }

        // Averaging root motion
        vec3 RootMotion = vec3(0.f);

        for (u32 AnimationIndex = 0; AnimationIndex < ActiveAnimationCount; ++AnimationIndex)
        {
            animation_state *AnimationState = ActiveAnimations[AnimationIndex];
            skeleton_pose *SkeletonPose = SkeletonPoses + AnimationIndex;

            RootMotion = RootMotion + SkeletonPose->RootMotion * AnimationState->Weight;
        }

        DestPose->RootMotion = RootMotion;

        // Blending between all skeleton poses
        skeleton_pose *Pose = First(SkeletonPoses);
        animation_state *Animation = *First(ActiveAnimations);

        Blend(Animation->BlendMode, 0.f, Animation->Weight, Pose, Pose, DestPose);

        f32 AccumulatedWeight = Animation->Weight;
        u32 CurrentPoseIndex = 1;

        while (CurrentPoseIndex < ActiveAnimationCount)
        {
            skeleton_pose *NextPose = SkeletonPoses + CurrentPoseIndex;
            animation_state *NextAnimation = ActiveAnimations[CurrentPoseIndex];

            f32 NextWeight = NextAnimation->Weight;

            CurrentPoseIndex += 1;
            AccumulatedWeight += NextWeight;

            f32 t = NextWeight / AccumulatedWeight;

            Assert(t >= 0.f && t <= 1.f);

            Blend(NextAnimation->BlendMode, t, NextAnimation->Weight, Pose, NextPose, DestPose);

            Pose = DestPose;
            Animation = NextAnimation;
        }
    }
}

dummy_internal void
UpdateGlobalJointPoses(skeleton_pose *Pose)
{
    for (u32 JointIndex = 0; JointIndex < Pose->Skeleton->JointCount; ++JointIndex)
    {
        joint *Joint = Pose->Skeleton->Joints + JointIndex;
        transform *LocalJointPose = Pose->LocalJointPoses + JointIndex;
        mat4 *GlobalJointPose = Pose->GlobalJointPoses + JointIndex;

        *GlobalJointPose = CalculateGlobalJointPose(Joint, LocalJointPose, Pose);
    }
}

dummy_internal void
UpdateSkinningMatrices(skinning_data *Skinning)
{
    skeleton *Skeleton = Skinning->Pose->Skeleton;

    for (u32 JointIndex = 0; JointIndex < Skeleton->JointCount; ++JointIndex)
    {
        joint *Joint = Skeleton->Joints + JointIndex;
        mat4 *GlobalJointPose = Skinning->Pose->GlobalJointPoses + JointIndex;
        mat4 *SkinningMatrix = Skinning->SkinningMatrices + JointIndex;

        *SkinningMatrix = *GlobalJointPose * Joint->InvBindTranform;
    }
}

dummy_internal void
AnimatorPerFrameUpdate(animator *Animator, animation_graph *Animation, void *Params, f32 Delta)
{
    animator_controller *Controller = HashTableLookup(&Animator->Controllers, Animation->Animator);

    if (Controller->Func)
    {
        Controller->Func(Animation, Params, Delta);
    }
    else
    {
        Assert(!"No controller is found for this animation graph");
    }
}
