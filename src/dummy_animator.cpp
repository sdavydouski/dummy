struct bot_animator_params
{
    f32 MaxTime;
    f32 Move;
    f32 MoveMagnitude;

    b32 ToStateIdle1;
    b32 ToStateIdle2;

    b32 ToStateActionIdle1;
    b32 ToStateActionIdle2;

    b32 ToStateActionIdle;
    b32 ToStateStandingIdle;

    b32 ToStateDancing;
    b32 ToStateActionIdleFromDancing;
};

struct paladin_animator_params
{
    f32 MaxTime;
    f32 Move;
    f32 MoveMagnitude;

    b32 ToStateActionIdle1;
    b32 ToStateActionIdle2;
    b32 ToStateActionIdle3;

    b32 ToStateDancing;
    b32 ToStateActionIdleFromDancing;

    b32 LightAttack;
    b32 StrongAttack;
};

internal void
BotActionIdlePerFrameUpdate(animation_graph *Graph, bot_animator_params *Params, f32 Delta)
{
    animation_node *Active = Graph->Active;

    if (StringEquals(Active->Name, "ActionIdle_0"))
    {
        Graph->State.Time += Delta;

        if (Graph->State.Time > Params->MaxTime)
        {
            Graph->State.Time = 0.f;

            if (Params->ToStateActionIdle1)
            {
                TransitionToNode(Graph, "ActionIdle_1");
            }

            if (Params->ToStateActionIdle2)
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
    bot_animator_params *BotParams = (bot_animator_params *) Params;

    animation_node *WalkingNode = GetAnimationNode(Graph, "Moving");
    WalkingNode->BlendSpace->Parameter = BotParams->Move;

    animation_node *Active = Graph->Active;

    if (StringEquals(Active->Name, "StandingIdle"))
    {
        if (BotParams->ToStateActionIdle)
        {
            TransitionToNode(Graph, "ActionIdle");
        }

        if (BotParams->ToStateDancing)
        {
            TransitionToNode(Graph, "Dancing");
        }
    }
    else if (StringEquals(Active->Name, "ActionIdle"))
    {
        animation_graph *StateIdleGraph = Active->Graph;
        BotActionIdlePerFrameUpdate(StateIdleGraph, BotParams, Delta);

        if (BotParams->ToStateStandingIdle)
        {
            StateIdleGraph->State.Time = 0.f;
            TransitionToNode(Graph, "StandingIdle");
        }

        if (BotParams->ToStateDancing)
        {
            StateIdleGraph->State.Time = 0.f;
            TransitionToNode(Graph, "Dancing");
        }

        if (BotParams->MoveMagnitude > 0.f)
        {
            StateIdleGraph->State.Time = 0.f;
            TransitionToNode(Graph, "Moving");
        }
    }
    else if (StringEquals(Active->Name, "Moving"))
    {
        if (BotParams->MoveMagnitude < EPSILON)
        {
            TransitionToNode(Graph, "ActionIdle");
        }
    }
    else if (StringEquals(Active->Name, "Dancing"))
    {
        if (BotParams->ToStateActionIdleFromDancing && BotParams->MoveMagnitude < EPSILON)
        {
            TransitionToNode(Graph, "ActionIdle");
        }
    }
    else if (StringEquals(Active->Name, "StandingIdleToActionIdle"))
    {
        if (BotParams->ToStateStandingIdle)
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
        if (BotParams->ToStateActionIdle)
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

internal void
PaladinActionIdlePerFrameUpdate(animation_graph *Graph, paladin_animator_params *Params, f32 Delta)
{
    animation_node *Active = Graph->Active;

    if (StringEquals(Active->Name, "sword and shield idle (4)"))
    {
        Graph->State.Time += Delta;

        if (Graph->State.Time > Params->MaxTime)
        {
            Graph->State.Time = 0.f;

            if (Params->ToStateActionIdle1)
            {
                Graph->State.Time = 0.f;
                TransitionToNode(Graph, "sword and shield idle");
            }

            if (Params->ToStateActionIdle2)
            {
                TransitionToNode(Graph, "sword and shield idle (2)");
            }

            if (Params->ToStateActionIdle3)
            {
                TransitionToNode(Graph, "sword and shield idle (3)");
            }
        }
    }
    else if (StringEquals(Active->Name, "sword and shield idle"))
    {
        if (Active->Animation.Time >= Active->Animation.Clip->Duration)
        {
            TransitionToNode(Graph, "sword and shield idle (4)");
        }
    }
    else if (StringEquals(Active->Name, "sword and shield idle (2)"))
    {
        if (Active->Animation.Time >= Active->Animation.Clip->Duration)
        {
            TransitionToNode(Graph, "sword and shield idle (4)");
        }
    }
    else if (StringEquals(Active->Name, "sword and shield idle (3)"))
    {
        if (Active->Animation.Time >= Active->Animation.Clip->Duration)
        {
            TransitionToNode(Graph, "sword and shield idle (4)");
        }
    }
    else
    {
        Assert(!"Invalid state");
    }
}

ANIMATOR_CONTROLLER(PaladinAnimatorController)
{
    paladin_animator_params *PaladinParams = (paladin_animator_params *) Params;

    animation_node *WalkingNode = GetAnimationNode(Graph, "Moving");
    WalkingNode->BlendSpace->Parameter = PaladinParams->Move;

    animation_node *Active = Graph->Active;

    if (StringEquals(Active->Name, "ActionIdle"))
    {
        animation_graph *StateIdleGraph = Active->Graph;
        PaladinActionIdlePerFrameUpdate(StateIdleGraph, PaladinParams, Delta);

        if (PaladinParams->MoveMagnitude > 0.f)
        {
            StateIdleGraph->State.Time = 0.f;
            TransitionToNode(Graph, "Moving");
        }

        if (PaladinParams->LightAttack)
        {
            StateIdleGraph->State.Time = 0.f;
            TransitionToNode(Graph, "LightAttack");
        }

        if (PaladinParams->StrongAttack)
        {
            StateIdleGraph->State.Time = 0.f;
            TransitionToNode(Graph, "StrongAttack");
        }

        if (PaladinParams->ToStateDancing)
        {
            StateIdleGraph->State.Time = 0.f;
            TransitionToNode(Graph, "Dancing");
        }
    }
    else if (StringEquals(Active->Name, "Moving"))
    {
        if (PaladinParams->MoveMagnitude < EPSILON)
        {
            TransitionToNode(Graph, "ActionIdle");
        }

        if (PaladinParams->MoveMagnitude <= 0.5f)
        {
            if (PaladinParams->LightAttack)
            {
                TransitionToNode(Graph, "LightAttack");
            }

            if (PaladinParams->StrongAttack)
            {
                TransitionToNode(Graph, "StrongAttack");
            }
        }
        else
        {
            if (PaladinParams->LightAttack)
            {
                TransitionToNode(Graph, "LightAttackMoving");
            }

            if (PaladinParams->StrongAttack)
            {
                TransitionToNode(Graph, "StrongAttackMoving");
            }
        }
    }
    else if (StringEquals(Active->Name, "LightAttack"))
    {
        if (Active->Animation.Time >= Active->Animation.Clip->Duration)
        {
            TransitionToNode(Graph, "ActionIdle");
        }
    }
    else if (StringEquals(Active->Name, "StrongAttack"))
    {
        if (Active->Animation.Time >= Active->Animation.Clip->Duration)
        {
            TransitionToNode(Graph, "ActionIdle");
        }
    }
    else if (StringEquals(Active->Name, "LightAttackMoving"))
    {
        if (Active->Animation.Time >= Active->Animation.Clip->Duration)
        {
            TransitionToNode(Graph, "Moving");
        }
    }
    else if (StringEquals(Active->Name, "StrongAttackMoving"))
    {
        if (Active->Animation.Time >= Active->Animation.Clip->Duration)
        {
            TransitionToNode(Graph, "Moving");
        }
    }
    else if (StringEquals(Active->Name, "Dancing"))
    {
        if (PaladinParams->ToStateActionIdleFromDancing && PaladinParams->MoveMagnitude < EPSILON)
        {
            TransitionToNode(Graph, "ActionIdle");
        }
    }
    else
    {
        Assert(!"Invalid state");
    }
}

ANIMATOR_CONTROLLER(SimpleAnimatorController)
{
    animation_node *Active = Graph->Active;

    // no transitions
}
