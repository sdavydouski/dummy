internal void
PelegriniStateIdlePerFrameUpdate(animation_graph *Graph, animator_params Params, f32 Delta)
{
    animation_node *Active = Graph->Active;

    if (StringEquals(Active->Name, "StateIdle_0"))
    {
        Graph->State.Time += Delta;

        if (Graph->State.Time > Params.MaxTime)
        {
            Graph->State.Time = 0.f;

            if (Params.ToStateIdle1)
            {
                TransitionToNode(Graph, "StateIdle_1");
            }

            if (Params.ToStateIdle2)
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

ANIMATOR_CONTROLLER(PelegriniAnimatorController)
{
    animation_node *WalkingNode = GetAnimationNode(Graph, "StateWalking");
    WalkingNode->BlendSpace->Parameter = Params.Move;

    animation_node *Active = Graph->Active;

    if (StringEquals(Active->Name, "StateIdle"))
    {
        animation_graph *StateIdleGraph = Active->Graph;
        PelegriniStateIdlePerFrameUpdate(StateIdleGraph, Params, Delta);

        // External transitions
        if (Params.ToStateDancing)
        {
            StateIdleGraph->State.Time = 0.f;
            TransitionToNode(Graph, "StateDancing");
        }

        if (Params.MoveMagnitude > 0.f)
        {
            StateIdleGraph->State.Time = 0.f;
            TransitionToNode(Graph, "StateWalking");
        }
    }
    else if (StringEquals(Active->Name, "StateWalking"))
    {
        if (Params.MoveMagnitude < EPSILON)
        {
            TransitionToNode(Graph, "StateIdle");
        }
    }
    else if (StringEquals(Active->Name, "StateDancing"))
    {
        if (Params.ToStateIdle && Params.MoveMagnitude < EPSILON)
        {
            TransitionToNode(Graph, "StateIdle");
        }
    }
    else
    {
        Assert(!"Invalid state");
    }
}

// xBot/yBot animator
internal void
ActionIdlePerFrameUpdate(animation_graph *Graph, animator_params Params, f32 Delta)
{
    animation_node *Active = Graph->Active;

    if (StringEquals(Active->Name, "ActionIdle_0"))
    {
        Graph->State.Time += Delta;

        if (Graph->State.Time > Params.MaxTime)
        {
            Graph->State.Time = 0.f;

            if (Params.ToStateActionIdle1)
            {
                TransitionToNode(Graph, "ActionIdle_1");
            }

            if (Params.ToStateActionIdle2)
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

ANIMATOR_CONTROLLER(BotAnimatorController)
{
    animation_node *WalkingNode = GetAnimationNode(Graph, "Moving");
    WalkingNode->BlendSpace->Parameter = Params.Move;

    animation_node *Active = Graph->Active;

    if (StringEquals(Active->Name, "StandingIdle"))
    {
        if (Params.ToStateActionIdle)
        {
            TransitionToNode(Graph, "ActionIdle");
        }
    }
    else if (StringEquals(Active->Name, "ActionIdle"))
    {
        animation_graph *StateIdleGraph = Active->Graph;
        ActionIdlePerFrameUpdate(StateIdleGraph, Params, Delta);

        if (Params.ToStateStandingIdle)
        {
            StateIdleGraph->State.Time = 0.f;
            TransitionToNode(Graph, "StandingIdle");
        }

        if (Params.ToStateDancing)
        {
            StateIdleGraph->State.Time = 0.f;
            TransitionToNode(Graph, "Dancing");
        }

        if (Params.MoveMagnitude > 0.f)
        {
            StateIdleGraph->State.Time = 0.f;
            TransitionToNode(Graph, "Moving");
        }
    }
    else if (StringEquals(Active->Name, "Moving"))
    {
        if (Params.MoveMagnitude < EPSILON)
        {
            TransitionToNode(Graph, "ActionIdle");
        }
    }
    else if (StringEquals(Active->Name, "Dancing"))
    {
        if (Params.ToStateActionIdleFromDancing && Params.MoveMagnitude < EPSILON)
        {
            TransitionToNode(Graph, "ActionIdle");
        }
    }
    else if (StringEquals(Active->Name, "StandingIdleToActionIdle"))
    {
        if (Params.ToStateStandingIdle)
        {
            TransitionToNode(Graph, "StandingIdle");
        }

        if (Active->Animation.Time >= Active->Animation.Clip->Duration)
        {
            TransitionToNode(Graph, "ActionIdle");
        }
    }
    else if (StringEquals(Active->Name, "ActionIdleToStandingIdle"))
    {
        if (Params.ToStateActionIdle)
        {
            TransitionToNode(Graph, "ActionIdle");
        }

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