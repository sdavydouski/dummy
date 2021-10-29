inline void
SetPelegriniAnimationParameters(animation_graph *Graph, f32 Move)
{
    animation_node *WalkingNode = GetAnimationNode(Graph, "StateWalking");
    WalkingNode->BlendSpace->Parameter = Move;
}

// todo: duplicated?
// Pelegrini animator
internal void
PelegriniStateIdlePerFrameUpdate(animation_graph *Graph, random_sequence *Entropy, f32 MaxTime, f32 Delta)
{
    animation_node *Active = Graph->Active;

    if (StringEquals(Active->Name, "StateIdle_0"))
    {
        Graph->State.Time += Delta;

        if (Graph->State.Time > MaxTime)
        {
            Graph->State.Time = 0.f;

            if (Random01(Entropy) > 0.5f)
            {
                TransitionToNode(Graph, "StateIdle_1");
            }
            else
            {
                TransitionToNode(Graph, "StateIdle_2");
            }
        }
    }
    else if (StringEquals(Active->Name, "StateIdle_1"))
    {
        if (Active->Animation.Time >= Active->Animation.Clip->Duration)
        {
            TransitionToNode(Graph, "StateIdle_0");
        }
    }
    else if (StringEquals(Active->Name, "StateIdle_2"))
    {
        if (Active->Animation.Time >= Active->Animation.Clip->Duration)
        {
            TransitionToNode(Graph, "StateIdle_0");
        }
    }
    else
    {
        Assert(!"Invalid state");
    }
}

internal void
PelegriniAnimatorPerFrameUpdate(animation_graph *Graph, game_input *Input, random_sequence *Entropy, f32 MaxTime, f32 Delta)
{
    animation_node *Active = Graph->Active;

    f32 MoveMaginute = Clamp(Magnitude(Input->Move.Range), 0.f, 1.f);

    if (StringEquals(Active->Name, "StateIdle"))
    {
        animation_graph *StateIdleGraph = Active->Graph;
        PelegriniStateIdlePerFrameUpdate(StateIdleGraph, Entropy, MaxTime, Delta);

        // External transitions
        if (Input->Crouch.IsActivated)
        {
            StateIdleGraph->State.Time = 0.f;
            TransitionToNode(Graph, "StateDancing");
        }

        if (MoveMaginute > 0.f)
        {
            StateIdleGraph->State.Time = 0.f;
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

inline void
SetBotAnimationParameters(animation_graph *Graph, f32 Move)
{
    animation_node *WalkingNode = GetAnimationNode(Graph, "Moving");
    WalkingNode->BlendSpace->Parameter = Move;
}

// xBot/yBot animator
internal void
ActionIdlePerFrameUpdate(animation_graph *Graph, random_sequence *Entropy, f32 MaxTime, f32 Delta)
{
    animation_node *Active = Graph->Active;

    if (StringEquals(Active->Name, "ActionIdle_0"))
    {
        Graph->State.Time += Delta;

        if (Graph->State.Time > MaxTime)
        {
            Graph->State.Time = 0.f;

            if (Random01(Entropy) > 0.5f)
            {
                TransitionToNode(Graph, "ActionIdle_1");
            }
            else
            {
                TransitionToNode(Graph, "ActionIdle_2");
            }
        }
    }
    else if (StringEquals(Active->Name, "ActionIdle_1"))
    {
        if (Active->Animation.Time >= Active->Animation.Clip->Duration)
        {
            TransitionToNode(Graph, "ActionIdle_0");
        }
    }
    else if (StringEquals(Active->Name, "ActionIdle_2"))
    {
        if (Active->Animation.Time >= Active->Animation.Clip->Duration)
        {
            TransitionToNode(Graph, "ActionIdle_0");
        }
    }
    else
    {
        Assert(!"Invalid state");
    }
}

internal void
BotAnimatorPerFrameUpdate(animation_graph *Graph, game_input *Input, random_sequence *Entropy, f32 MaxTime, f32 Delta)
{
    animation_node *Active = Graph->Active;

    f32 MoveMaginute = Clamp(Magnitude(Input->Move.Range), 0.f, 1.f);

    if (StringEquals(Active->Name, "StandingIdle"))
    {
        if (Input->Activate.IsActivated)
        {
            TransitionToNode(Graph, "ActionIdle");
        }
    }
    else if (StringEquals(Active->Name, "ActionIdle"))
    {
        animation_graph *StateIdleGraph = Active->Graph;
        ActionIdlePerFrameUpdate(StateIdleGraph, Entropy, MaxTime, Delta);

        if (Input->Activate.IsActivated)
        {
            StateIdleGraph->State.Time = 0.f;
            TransitionToNode(Graph, "StandingIdle");
        }

        if (Input->Crouch.IsActivated)
        {
            StateIdleGraph->State.Time = 0.f;
            TransitionToNode(Graph, "Dancing");
        }

        if (MoveMaginute > 0.f)
        {
            StateIdleGraph->State.Time = 0.f;
            TransitionToNode(Graph, "Moving");
        }
    }
    else if (StringEquals(Active->Name, "Moving"))
    {
        if (MoveMaginute < EPSILON)
        {
            TransitionToNode(Graph, "ActionIdle");
        }
    }
    else if (StringEquals(Active->Name, "Dancing"))
    {
        if (Input->Crouch.IsActivated && MoveMaginute < EPSILON)
        {
            TransitionToNode(Graph, "ActionIdle");
        }
    }
    else if (StringEquals(Active->Name, "StandingIdleToActionIdle"))
    {
        if (Active->Animation.Time >= Active->Animation.Clip->Duration)
        {
            TransitionToNode(Graph, "ActionIdle");
        }
    }
    else if (StringEquals(Active->Name, "ActionIdleToStandingIdle"))
    {
        if (Active->Animation.Time >= Active->Animation.Clip->Duration)
        {
            TransitionToNode(Graph, "StandingIdle");
        }
    }
    else
    {
        Assert(!"Invalid state");
    }
}