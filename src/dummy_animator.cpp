#include "dummy.h"

// Bot
inline void
GameInput2BotAnimatorParams(game_state *State, game_input *Input, game_entity *Entity, bot_animator_params *Params)
{
    Params->TargetMoveMagnitude = Clamp(Magnitude(State->TargetMove), 0.f, 2.f);
    Params->CurrentMoveMagnitude = Clamp(Magnitude(State->CurrentMove), 0.f, 2.f);

    Params->IsGrounded = Entity->IsGrounded;
    Params->IsPlayer = (State->Player == Entity);
    Params->IsDanceMode = State->DanceMode.Value;

    if (Entity->Body)
    {
        Params->Velocity = Entity->Body->Velocity;
        Params->PrevVelocity = Entity->Body->PrevVelocity;
    }

    Params->MaxActionIdleTime = 5.f;
    Params->ActionIdleRandomTransition = Random01(&State->GeneralEntropy);
}

inline void
GameLogic2BotAnimatorParams(game_state *State, game_entity *Entity, bot_animator_params *Params)
{
    Params->IsGrounded = Entity->IsGrounded;
    Params->IsPlayer = (State->Player == Entity);
    Params->IsDanceMode = State->DanceMode.Value;

    if (Entity->Body)
    {
        Params->Velocity = Entity->Body->Velocity;
        Params->PrevVelocity = Entity->Body->PrevVelocity;
    }

    Params->MaxActionIdleTime = 5.f;
    Params->ActionIdleRandomTransition = Random01(&State->GeneralEntropy);
}

dummy_internal void
BotLocomotionNode(animation_graph *Root, animation_graph *Locomotion, bot_animator_params *Params, f32 Delta)
{
    animation_node *LocomotionActive = Locomotion->Active;

    if (!Params->IsPlayer)
    {
        TransitionToNode(Root, "StandingIdle");
    }

    if (Params->IsDanceMode)
    {
        TransitionToNode(Root, "Dance");
    }

    if (!Params->IsGrounded)
    {
        if (Params->Velocity.y > 0.f)
        {
            TransitionToNode(Root, "Jump_Start");
        }
        else
        {
            TransitionToNode(Root, "Jump_Idle");
        }
    }

    switch (SID(LocomotionActive->Name))
    {
        case SID("ActionIdle"):
        {
            animation_graph *ActionIdleGraph = LocomotionActive->Graph;
            animation_node *ActionIdleGraphActive = ActionIdleGraph->Active;

            if (Params->TargetMoveMagnitude > EPSILON)
            {
                TransitionToNode(Locomotion, "Moving");
            }

            switch (SID(ActionIdleGraphActive->Name))
            {
                case SID("ActionIdle_0"):
                {
                    ActionIdleGraph->AnimatorState.Time += Delta;

                    if (ActionIdleGraph->AnimatorState.Time >= Params->MaxActionIdleTime)
                    {
                        ActionIdleGraph->AnimatorState.Time = 0.f;

                        if (Params->ActionIdleRandomTransition <= 0.5)
                        {
                            TransitionToNode(ActionIdleGraph, "ActionIdle_1");
                        }
                        else
                        {
                            TransitionToNode(ActionIdleGraph, "ActionIdle_2");
                        }
                    }

                    break;
                }
                case SID("ActionIdle_1"):
                {
                    if (AnimationClipFinished(ActionIdleGraphActive->Animation))
                    {
                        TransitionToNode(ActionIdleGraph, "ActionIdle_0");
                    }

                    break;
                }
                case SID("ActionIdle_2"):
                {
                    if (AnimationClipFinished(ActionIdleGraphActive->Animation))
                    {
                        TransitionToNode(ActionIdleGraph, "ActionIdle_0");
                    }

                    break;
                }
                default:
                {
                    Assert(!"Invalid state");
                    break;
                }
            }

            break;
        }
        case SID("Moving"):
        {
            LocomotionActive->BlendSpace->Parameter = Params->CurrentMoveMagnitude;

            if (Params->TargetMoveMagnitude < EPSILON)
            {
                TransitionToNode(Locomotion, "ActionIdle");
            }

            break;
        }
        default: 
        {
            Assert(!"Invalid state");
            break;
        }
    }
}

ANIMATOR_CONTROLLER(BotAnimatorController)
{
    bot_animator_params *BotParams = (bot_animator_params *) Params;

    animation_node *Active = Graph->Active;

    switch (SID(Active->Name))
    {
        case SID("StandingIdle"):
        {
            if (BotParams->IsPlayer)
            {
                TransitionToNode(Graph, "Locomotion");
            }

            if (BotParams->IsDanceMode)
            {
                TransitionToNode(Graph, "Dance");
            }

            break;
        }
        case SID("Locomotion"):
        {
            BotLocomotionNode(Graph, Active->Graph, BotParams, Delta);

            break;
        }
        case SID("Jump_Start"):
        {
            if (AnimationClipFinished(Active->Animation))
            {
                TransitionToNode(Graph, "Jump_Idle");
            }

            break;
        }
        case SID("Jump_Idle"):
        {
            if (BotParams->IsGrounded)
            {
                animation_node *JumpLandNode = GetAnimationNode(Graph, "Jump_Land");

                f32 Velocity = Abs(BotParams->PrevVelocity.y);
                // todo:
                f32 VelocityMin = 0.f;
                f32 VelocityMax = 14.f;
                f32 FallImpact = NormalizeRange(Velocity, VelocityMin, VelocityMax);

                additive_animation *Landing = GetAdditiveAnimation(JumpLandNode, "falling_to_landing::additive");
                Landing->Weight = FallImpact;

                TransitionToNode(Graph, "Jump_Land");
            }
            break;
        }
        case SID("Jump_Land"):
        {
            BotLocomotionNode(Graph, Active->Reference->Graph, BotParams, Delta);

            if (AdditiveAnimationsFinished(Active))
            {
                TransitionToNode(Graph, "Locomotion");
            }

            break;
        }
        case SID("Dance"):
        {
            if (!BotParams->IsDanceMode)
            {
                TransitionToNode(Graph, "Locomotion");
            }

            break;
        }
        case SID("StandingIdleToLocomotion"):
        {
            if (!BotParams->IsPlayer)
            {
                TransitionToNode(Graph, "StandingIdle");
            }

            if (AnimationClipFinished(Active->Animation))
            {
                TransitionToNode(Graph, "Locomotion");
            }

            break;
        }
        case SID("LocomotionToStandingIdle"):
        {
            if (BotParams->IsPlayer)
            {
                TransitionToNode(Graph, "Locomotion");
            }

            if (AnimationClipFinished(Active->Animation))
            {
                TransitionToNode(Graph, "StandingIdle");
            }

            break;
        }
        default:
        {
            Assert(!"Invalid state");
            break;
        }
    }
}

// Paladin
inline void
GameInput2PaladinAnimatorParams(game_state *State, game_input *Input, game_entity *Entity, paladin_animator_params *Params)
{
    Params->TargetMoveMagnitude = Clamp(Magnitude(State->TargetMove), 0.f, 1.f);
    Params->CurrentMoveMagnitude = Clamp(Magnitude(State->CurrentMove), 0.f, 1.f);

    Params->IsDanceMode = State->DanceMode.Value;

    Params->MaxActionIdleTime = 5.f;
    Params->ActionIdleRandomTransition = Random01(&State->GeneralEntropy);

    Params->LightAttack = Input->LightAttack.IsActivated;
    Params->StrongAttack = Input->StrongAttack.IsActivated;
}

inline void
GameLogic2PaladinAnimatorParams(game_state *State, game_entity *Entity, paladin_animator_params *Params)
{
    Params->IsDanceMode = State->DanceMode.Value;

    Params->MaxActionIdleTime = 5.f;
    Params->ActionIdleRandomTransition = Random01(&State->GeneralEntropy);
}

dummy_internal void
PaladinActionIdlePerFrameUpdate(animation_graph *Graph, paladin_animator_params *Params, f32 Delta)
{
    animation_node *Active = Graph->Active;

    switch (SID(Active->Name))
    {
        case SID("sword and shield idle (4)"):
        {
            Graph->AnimatorState.Time += Delta;

            if (Graph->AnimatorState.Time >= Params->MaxActionIdleTime)
            {
                Graph->AnimatorState.Time = 0.f;

                if (Params->ActionIdleRandomTransition >= 0.f && Params->ActionIdleRandomTransition < 0.33f)
                {
                    TransitionToNode(Graph, "sword and shield idle");
                }
                else if (Params->ActionIdleRandomTransition >= 0.33f && Params->ActionIdleRandomTransition < 0.66f)
                {
                    TransitionToNode(Graph, "sword and shield idle (2)");
                }
                else
                {
                    TransitionToNode(Graph, "sword and shield idle (3)");
                }
            }

            break;
        }
        case SID("sword and shield idle"):
        {
            if (AnimationClipFinished(Active->Animation))
            {
                TransitionToNode(Graph, "sword and shield idle (4)");
            }

            break;
        }
        case SID("sword and shield idle (2)"):
        {
            if (AnimationClipFinished(Active->Animation))
            {
                TransitionToNode(Graph, "sword and shield idle (4)");
            }

            break;
        }
        case SID("sword and shield idle (3)"):
        {
            if (AnimationClipFinished(Active->Animation))
            {
                TransitionToNode(Graph, "sword and shield idle (4)");
            }

            break;
        }
        default:
        {
            Assert(!"Invalid state");
            break;
        }
    }
}

ANIMATOR_CONTROLLER(PaladinAnimatorController)
{
    paladin_animator_params *PaladinParams = (paladin_animator_params *) Params;
    animation_node *Active = Graph->Active;

    switch (SID(Active->Name))
    {
        case SID("ActionIdle"):
        {
            PaladinActionIdlePerFrameUpdate(Active->Graph, PaladinParams, Delta);

            if (PaladinParams->TargetMoveMagnitude > 0.f)
            {
                TransitionToNode(Graph, "Moving");
            }

            if (PaladinParams->LightAttack)
            {
                TransitionToNode(Graph, "LightAttack");
            }

            if (PaladinParams->StrongAttack)
            {
                TransitionToNode(Graph, "StrongAttack");
            }

            if (PaladinParams->IsDanceMode)
            {
                TransitionToNode(Graph, "Dancing");
            }

            break;
        }
        case SID("Moving"):
        {
            Active->BlendSpace->Parameter = PaladinParams->CurrentMoveMagnitude;

            if (PaladinParams->TargetMoveMagnitude < EPSILON)
            {
                TransitionToNode(Graph, "ActionIdle");
            }

            if (PaladinParams->TargetMoveMagnitude <= 0.5f)
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

            break;
        }
        case SID("LightAttack"):
        {
            if (AnimationClipFinished(Active->Animation))
            {
                TransitionToNode(Graph, "ActionIdle");
            }

            break;
        }
        case SID("StrongAttack"):
        {
            if (AnimationClipFinished(Active->Animation))
            {
                TransitionToNode(Graph, "ActionIdle");
            }

            break;
        }
        case SID("LightAttackMoving"):
        {
            if (AnimationClipFinished(Active->Animation))
            {
                TransitionToNode(Graph, "Moving");
            }

            break;
        }
        case SID("StrongAttackMoving"):
        {
            if (AnimationClipFinished(Active->Animation))
            {
                TransitionToNode(Graph, "Moving");
            }

            break;
        }
        case SID("Dancing"):
        {
            if (!PaladinParams->IsDanceMode)
            {
                TransitionToNode(Graph, "ActionIdle");
            }

            break;
        }
        default:
        {
            Assert(!"Invalid state");
            break;
        }
    }
}

// Monstar
inline void
GameInput2MonstarAnimatorParams(game_state *State, game_input *Input, game_entity *Entity, monstar_animator_params *Params)
{
    Params->TargetMoveMagnitude = Clamp(Magnitude(State->TargetMove), 0.f, 1.f);
    Params->CurrentMoveMagnitude = Clamp(Magnitude(State->CurrentMove), 0.f, 1.f);
    Params->IsDanceMode = State->DanceMode.Value;
    Params->Attack = Input->LightAttack.IsActivated;
}

inline void
GameLogic2MonstarAnimatorParams(game_state *State, game_entity *Entity, monstar_animator_params *Params)
{
    Params->IsDanceMode = State->DanceMode.Value;
}

ANIMATOR_CONTROLLER(MonstarAnimatorController)
{
    monstar_animator_params *MonstarParams = (monstar_animator_params *) Params;
    animation_node *Active = Graph->Active;

    switch (SID(Active->Name))
    {
        case SID("Idle"):
        {
            if (MonstarParams->TargetMoveMagnitude > 0.f)
            {
                TransitionToNode(Graph, "Moving");
            }

            if (MonstarParams->Attack)
            {
                TransitionToNode(Graph, "Attack");
            }

            if (MonstarParams->IsDanceMode)
            {
                TransitionToNode(Graph, "Dancing");
            }

            break;
        }
        case SID("Moving"):
        {
            Active->BlendSpace->Parameter = MonstarParams->CurrentMoveMagnitude;

            if (MonstarParams->TargetMoveMagnitude < EPSILON)
            {
                TransitionToNode(Graph, "Idle");
            }

            break;
        }
        case SID("Attack"):
        {
            if (AnimationClipFinished(Active->Animation))
            {
                TransitionToNode(Graph, "Idle");
            }

            break;
        }
        case SID("Dancing"):
        {
            if (!MonstarParams->IsDanceMode)
            {
                TransitionToNode(Graph, "Idle");
            }

            break;
        }
        default:
        {
            Assert(!"Invalid state");
            break;
        }
    }
}

ANIMATOR_CONTROLLER(SimpleAnimatorController)
{
    animation_node *Active = Graph->Active;

    // no transitions
}
