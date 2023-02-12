// todo:
struct bot_animator_params
{
    f32 MaxTime;
    f32 Move;
    f32 MoveMagnitude;
    bool32 IsGrounded;
    vec3 Velocity;

    bool32 ToStateIdle1;
    bool32 ToStateIdle2;

    bool32 ToStateActionIdle1;
    bool32 ToStateActionIdle2;

    bool32 ToStateActionIdle;
    bool32 ToStateStandingIdle;

    bool32 ToStateDancing;
    bool32 ToStateActionIdleFromDancing;
};

// todo(continue): update this
ANIMATOR_CONTROLLER(BotAnimatorController)
{
    bot_animator_params *BotParams = (bot_animator_params *) Params;

    //animation_node *WalkingNode = GetAnimationNode(Graph, "Moving");
    //WalkingNode->BlendSpace->Parameter = BotParams->Move;

    animation_node *Active = Graph->Active;

    if (StringEquals(Active->Name, "StandingIdle"))
    {
        if (BotParams->ToStateActionIdle)
        {
            TransitionToNode(Graph, "Locomotion");
        }
    }
    else if (StringEquals(Active->Name, "Locomotion"))
    {
        animation_graph *LocomotionGraph = Active->Graph;
        animation_node *LocomotionActive = LocomotionGraph->Active;

        if (BotParams->ToStateStandingIdle)
        {
            TransitionToNode(Graph, "StandingIdle");
        }

        if (!BotParams->IsGrounded)
        {
            if (BotParams->Velocity.y > 0.f)
            {
                TransitionToNode(Graph, "Jump_Start");
            }
            else
            {
                TransitionToNode(Graph, "Jump_Idle");
            }
        }

        if (StringEquals((LocomotionActive->Name), "ActionIdle"))
        {
            animation_graph *ActionIdleGraph = LocomotionActive->Graph;
            animation_node *ActionIdleGraphActive = ActionIdleGraph->Active;

            if (BotParams->MoveMagnitude > EPSILON)
            {
                TransitionToNode(LocomotionGraph, "Moving");
            }

            if (StringEquals(ActionIdleGraphActive->Name, "ActionIdle_0"))
            {
                ActionIdleGraph->State.Time += Delta;

                if (ActionIdleGraph->State.Time > BotParams->MaxTime)
                {
                    ActionIdleGraph->State.Time = 0.f;

                    if (BotParams->ToStateActionIdle1)
                    {
                        TransitionToNode(ActionIdleGraph, "ActionIdle_1");
                    }

                    if (BotParams->ToStateActionIdle2)
                    {
                        TransitionToNode(ActionIdleGraph, "ActionIdle_2");
                    }
                }
            }
            else if (StringEquals(ActionIdleGraphActive->Name, "ActionIdle_1"))
            {
                if (ActionIdleGraphActive->Animation.Time >= ActionIdleGraphActive->Animation.Clip->Duration)
                {
                    TransitionToNode(ActionIdleGraph, "ActionIdle_0");
                }
            }
            else if (StringEquals(ActionIdleGraphActive->Name, "ActionIdle_2"))
            {
                if (ActionIdleGraphActive->Animation.Time >= ActionIdleGraphActive->Animation.Clip->Duration)
                {
                    TransitionToNode(ActionIdleGraph, "ActionIdle_0");
                }
            }
            else
            {
                Assert(!"Invalid state");
            }
        }
        else if (StringEquals(LocomotionActive->Name, "Moving"))
        {
            LocomotionActive->BlendSpace->Parameter = BotParams->Move;

            if (BotParams->MoveMagnitude < EPSILON)
            {
                TransitionToNode(LocomotionGraph, "ActionIdle");
            }
        }
    }
    else if (StringEquals(Active->Name, "Jump_Start"))
    {
        if (Active->Animation.Time >= (Active->Animation.Clip->Duration - 0.f))
        {
            TransitionToNode(Graph, "Jump_Idle");
        }
    }
    else if (StringEquals(Active->Name, "Jump_Idle"))
    {
        if (BotParams->IsGrounded)
        {
            // todo:
            //TransitionToNode(Graph, "Jump_Land");
            TransitionToNode(Graph, "Locomotion");
        }
    }
    else if (StringEquals(Active->Name, "Jump_Land"))
    {
        if (!BotParams->IsGrounded)
        {
            TransitionToNode(Graph, "Jump_Start");
        }

        // todo:
        if (Active->Additive.Base->Animation.Time >= (Active->Additive.Base->Animation.Clip->Duration - 0.f))
        {
            TransitionToNode(Graph, "Locomotion");
        }
    }
    else if (StringEquals(Active->Name, "StandingIdleToLocomotion"))
    {
        if (BotParams->ToStateStandingIdle)
        {
            TransitionToNode(Graph, "StandingIdle");
        }

        if (Active->Animation.Time >= Active->Animation.Clip->Duration)
        {
            TransitionToNode(Graph, "Locomotion");
        }
    }
    else if (StringEquals(Active->Name, "LocomotionToStandingIdle"))
    {
        if (BotParams->ToStateActionIdle)
        {
            TransitionToNode(Graph, "Locomotion");
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

struct paladin_animator_params
{
    f32 MaxTime;
    f32 Move;
    f32 MoveMagnitude;

    bool32 ToStateActionIdle1;
    bool32 ToStateActionIdle2;
    bool32 ToStateActionIdle3;

    bool32 ToStateDancing;
    bool32 ToStateActionIdleFromDancing;

    bool32 LightAttack;
    bool32 StrongAttack;
};

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

struct monstar_animator_params
{
    f32 Move;
    f32 MoveMagnitude;
    bool32 Attack;

    bool32 ToStateDancing;
    bool32 ToStateIdleFromDancing;
};

ANIMATOR_CONTROLLER(MonstarAnimatorController)
{
    monstar_animator_params *MonstarParams = (monstar_animator_params *) Params;

    animation_node *WalkingNode = GetAnimationNode(Graph, "Moving");
    WalkingNode->BlendSpace->Parameter = MonstarParams->Move;

    animation_node *Active = Graph->Active;

    if (StringEquals(Active->Name, "Idle"))
    {
        if (MonstarParams->MoveMagnitude > 0.f)
        {
            TransitionToNode(Graph, "Moving");
        }

        if (MonstarParams->Attack)
        {
            TransitionToNode(Graph, "Attack");
        }

        if (MonstarParams->ToStateDancing)
        {
            TransitionToNode(Graph, "Dancing");
        }
    }
    else if (StringEquals(Active->Name, "Moving"))
    {
        if (MonstarParams->MoveMagnitude < EPSILON)
        {
            TransitionToNode(Graph, "Idle");
        }
    }
    else if (StringEquals(Active->Name, "Attack"))
    {
        if (Active->Animation.Time >= Active->Animation.Clip->Duration)
        {
            TransitionToNode(Graph, "Idle");
        }
    }
    else if (StringEquals(Active->Name, "Dancing"))
    {
        if (MonstarParams->ToStateIdleFromDancing && MonstarParams->MoveMagnitude < EPSILON)
        {
            TransitionToNode(Graph, "Idle");
        }
    }
    else
    {
        Assert(!"Invalid state");
    }
}

struct cleric_animator_params
{
    f32 Move;
    f32 MoveMagnitude;

    bool32 ToStateDancing;
    bool32 ToStateIdleFromDancing;
};

ANIMATOR_CONTROLLER(ClericAnimatorController)
{
    cleric_animator_params *ClericParams = (cleric_animator_params *) Params;

    animation_node *WalkingNode = GetAnimationNode(Graph, "Moving");
    WalkingNode->BlendSpace->Parameter = ClericParams->Move;

    animation_node *Active = Graph->Active;

    if (StringEquals(Active->Name, "ActionIdle"))
    {
        if (ClericParams->MoveMagnitude > 0.f)
        {
            TransitionToNode(Graph, "Moving");
        }

        if (ClericParams->ToStateDancing)
        {
            TransitionToNode(Graph, "Dancing");
        }
    }
    else if (StringEquals(Active->Name, "Moving"))
    {
        if (ClericParams->MoveMagnitude < EPSILON)
        {
            TransitionToNode(Graph, "ActionIdle");
        }
    }
    else if (StringEquals(Active->Name, "Dancing"))
    {
        if (ClericParams->ToStateIdleFromDancing && ClericParams->MoveMagnitude < EPSILON)
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
