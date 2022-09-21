#pragma once

struct platform_button_state
{
    b32 IsPressed;
    b32 WasPressed;
    b32 Toggle;
};

struct platform_input_keyboard
{
    platform_button_state Up;
    platform_button_state Down;
    platform_button_state Left;
    platform_button_state Right;

    platform_button_state Z;
    platform_button_state C;
    platform_button_state E;
    platform_button_state R;

    platform_button_state Tab;
    platform_button_state Ctrl;
    platform_button_state Space;
    platform_button_state Esc;
    platform_button_state Enter;
    platform_button_state Shift;

    platform_button_state Plus;
    platform_button_state Minus;

    platform_button_state One;
    platform_button_state Two;
    platform_button_state Three;
    platform_button_state Zero;
};

struct platform_input_mouse
{
    i32 x;
    i32 y;

    i32 dx;
    i32 dy;

    i32 WheelDelta;

    platform_button_state LeftButton;
    platform_button_state RightButton;
};

struct platform_input_xbox_controller
{
    vec2 LeftStick;
    vec2 RightStick;

    f32 LeftTrigger;
    f32 RightTrigger;

    platform_button_state DradUp;
    platform_button_state DradDown;
    platform_button_state DradLeft;
    platform_button_state DradRight;

    platform_button_state Start;
    platform_button_state Back;

    platform_button_state LeftThumb;
    platform_button_state RightThumb;

    platform_button_state A;
    platform_button_state B;
    platform_button_state X;
    platform_button_state Y;
};

struct game_input_action
{
    b32 IsActivated;
};

struct game_input_state
{
    b32 IsActive;
};

struct game_input_range
{
    vec2 Range;
};

struct game_input
{
    game_input_range Move;
    game_input_action Dance;
    game_input_action Activate;

    game_input_action ChoosePrevHero;
    game_input_action ChooseNextHero;

    game_input_range Camera;
    game_input_action Menu;
    game_input_action Reset;
    game_input_state HighlightBackground;

    f32 ZoomDelta;
    vec2 MouseCoords;
    game_input_action LeftClick;
    game_input_action EditMode;
    game_input_state EnableFreeCameraMovement;
};

inline b32
IsButtonActivated(platform_button_state *Button)
{
    b32 Result = Button->IsPressed && (Button->IsPressed != Button->WasPressed);
    return Result;
}

inline void
SavePrevButtonState(platform_button_state *Button)
{
    Button->WasPressed = Button->IsPressed;
}

inline void
UpdateToggleButtonState(platform_button_state *Button)
{
    if (IsButtonActivated(Button))
    {
        Button->Toggle = !Button->Toggle;
    }
}

inline void
ProcessInputAction(game_input_action *Action, platform_button_state *Button)
{
    if (!Action->IsActivated)
    {
        Action->IsActivated = IsButtonActivated(Button);
    }
}

inline void
ProcessInputState(game_input_state *State, platform_button_state *Button)
{
    if (!State->IsActive)
    {
        State->IsActive = Button->IsPressed;
    }
}

//#define PROCESS_INPUT_ACTION(Action, Button)  if (!Action.IsActivated) { Action.IsActivated = IsButtonActivated(&Button); }
//#define PROCESS_INPUT_STATE(State, Button)  if (!State.IsActive) { State.IsActive = Button.IsPressed; }

inline void
BeginProcessXboxControllerInput(platform_input_xbox_controller *XboxControllerInput)
{
    SavePrevButtonState(&XboxControllerInput->Start);
    SavePrevButtonState(&XboxControllerInput->Back);

    SavePrevButtonState(&XboxControllerInput->DradUp);
    SavePrevButtonState(&XboxControllerInput->DradDown);
    SavePrevButtonState(&XboxControllerInput->DradLeft);
    SavePrevButtonState(&XboxControllerInput->DradRight);

    SavePrevButtonState(&XboxControllerInput->LeftThumb);
    SavePrevButtonState(&XboxControllerInput->RightThumb);

    SavePrevButtonState(&XboxControllerInput->A);
    SavePrevButtonState(&XboxControllerInput->B);
    SavePrevButtonState(&XboxControllerInput->X);
    SavePrevButtonState(&XboxControllerInput->Y);
}

inline void
EndProcessXboxControllerInput(platform_input_xbox_controller *XboxControllerInput)
{
    UpdateToggleButtonState(&XboxControllerInput->Start);
    UpdateToggleButtonState(&XboxControllerInput->Back);

    UpdateToggleButtonState(&XboxControllerInput->DradUp);
    UpdateToggleButtonState(&XboxControllerInput->DradDown);
    UpdateToggleButtonState(&XboxControllerInput->DradLeft);
    UpdateToggleButtonState(&XboxControllerInput->DradRight);

    UpdateToggleButtonState(&XboxControllerInput->LeftThumb);
    UpdateToggleButtonState(&XboxControllerInput->RightThumb);

    UpdateToggleButtonState(&XboxControllerInput->A);
    UpdateToggleButtonState(&XboxControllerInput->B);
    UpdateToggleButtonState(&XboxControllerInput->X);
    UpdateToggleButtonState(&XboxControllerInput->Y);
}

inline void
XboxControllerInput2GameInput(platform_input_xbox_controller *XboxControllerInput, game_input *GameInput)
{
    if (Magnitude(GameInput->Move.Range) == 0.f)
    {
        GameInput->Move.Range = XboxControllerInput->LeftStick;
    }

    if (XboxControllerInput->LeftThumb.IsPressed)
    {
        GameInput->Move.Range *= 2.f;
    }

    if (Magnitude(GameInput->Camera.Range) == 0.f)
    {
        GameInput->Camera.Range = XboxControllerInput->RightStick;
    }

    ProcessInputAction(&GameInput->Menu, &XboxControllerInput->Start);
    ProcessInputAction(&GameInput->ChoosePrevHero, &XboxControllerInput->DradLeft);
    ProcessInputAction(&GameInput->ChooseNextHero, &XboxControllerInput->DradRight);
    ProcessInputAction(&GameInput->Dance, &XboxControllerInput->X);

    ProcessInputState(&GameInput->HighlightBackground, &XboxControllerInput->Back);

    if (GameInput->ZoomDelta == 0.f)
    {
        if (XboxControllerInput->LeftTrigger > 0.f)
        {
            GameInput->ZoomDelta = -XboxControllerInput->LeftTrigger * 0.1f;
        }

        if (XboxControllerInput->RightTrigger > 0.f)
        {
            GameInput->ZoomDelta = XboxControllerInput->RightTrigger * 0.1f;
        }
    }
}

inline void
BeginProcessKeyboardInput(platform_input_keyboard *KeyboardInput)
{
    SavePrevButtonState(&KeyboardInput->C);
    SavePrevButtonState(&KeyboardInput->Z);
    SavePrevButtonState(&KeyboardInput->E);
    SavePrevButtonState(&KeyboardInput->R);
    SavePrevButtonState(&KeyboardInput->Tab);
    SavePrevButtonState(&KeyboardInput->Space);
    SavePrevButtonState(&KeyboardInput->Enter);
    SavePrevButtonState(&KeyboardInput->Shift);
    SavePrevButtonState(&KeyboardInput->Plus);
    SavePrevButtonState(&KeyboardInput->Minus);
    SavePrevButtonState(&KeyboardInput->One);
    SavePrevButtonState(&KeyboardInput->Two);
    SavePrevButtonState(&KeyboardInput->Three);
    SavePrevButtonState(&KeyboardInput->Zero);
}

inline void
EndProcessKeyboardInput(platform_input_keyboard *KeyboardInput)
{
    UpdateToggleButtonState(&KeyboardInput->C);
    UpdateToggleButtonState(&KeyboardInput->Z);
    UpdateToggleButtonState(&KeyboardInput->E);
    UpdateToggleButtonState(&KeyboardInput->R);
    UpdateToggleButtonState(&KeyboardInput->Tab);
    UpdateToggleButtonState(&KeyboardInput->Space);
    UpdateToggleButtonState(&KeyboardInput->Enter);
    UpdateToggleButtonState(&KeyboardInput->Shift);
    UpdateToggleButtonState(&KeyboardInput->Plus);
    UpdateToggleButtonState(&KeyboardInput->Minus);
    UpdateToggleButtonState(&KeyboardInput->One);
    UpdateToggleButtonState(&KeyboardInput->Two);
    UpdateToggleButtonState(&KeyboardInput->Three);
    UpdateToggleButtonState(&KeyboardInput->Zero);
}

inline void
BeginProcessMouseInput(platform_input_mouse *MouseInput)
{
    SavePrevButtonState(&MouseInput->LeftButton);
}

inline void
EndProcessMouseInput(platform_input_mouse *MouseInput)
{

}

internal void
KeyboardInput2GameInput(platform_input_keyboard *KeyboardInput, game_input *GameInput)
{
    f32 Move = 0.5f;

    if (KeyboardInput->Z.Toggle)
    {
        if (KeyboardInput->Shift.IsPressed)
        {
            Move = 2.f;
        }
        else
        {
            Move = 1.f;
        }
    }

    if (Magnitude(GameInput->Move.Range) == 0.f)
    {
        if (KeyboardInput->Left.IsPressed && KeyboardInput->Right.IsPressed)
        {
            GameInput->Move.Range.x = 0.f;
        }
        else if (KeyboardInput->Left.IsPressed)
        {
            GameInput->Move.Range.x = -Move;
        }
        else if (KeyboardInput->Right.IsPressed)
        {
            GameInput->Move.Range.x = Move;
        }
        else
        {
            GameInput->Move.Range.x = 0.f;
        }

        if (KeyboardInput->Up.IsPressed && KeyboardInput->Down.IsPressed)
        {
            GameInput->Move.Range.y = 0.f;
        }
        else if (KeyboardInput->Up.IsPressed)
        {
            GameInput->Move.Range.y = Move;
        }
        else if (KeyboardInput->Down.IsPressed)
        {
            GameInput->Move.Range.y = -Move;
        }
        else
        {
            GameInput->Move.Range.y = 0.f;
        }
    }

    ProcessInputAction(&GameInput->Dance, &KeyboardInput->C);
    ProcessInputAction(&GameInput->Activate, &KeyboardInput->E);
    ProcessInputAction(&GameInput->Reset, &KeyboardInput->R);
    ProcessInputAction(&GameInput->Menu, &KeyboardInput->Enter);
    ProcessInputAction(&GameInput->EditMode, &KeyboardInput->Tab);
    ProcessInputAction(&GameInput->ChoosePrevHero, &KeyboardInput->One);
    ProcessInputAction(&GameInput->ChooseNextHero, &KeyboardInput->Two);

    ProcessInputState(&GameInput->HighlightBackground, &KeyboardInput->Ctrl);
}

inline void
MouseInput2GameInput(platform_input_mouse *MouseInput, game_input *GameInput)
{
    if (Magnitude(GameInput->Camera.Range) == 0.f)
    {
        f32 MouseSensitivity = 0.05f;
        vec2 MouseMovement = vec2((f32) MouseInput->dx, (f32) MouseInput->dy) * MouseSensitivity;

        GameInput->Camera.Range = vec2(MouseMovement.x, -MouseMovement.y);
        GameInput->EnableFreeCameraMovement.IsActive = MouseInput->RightButton.IsPressed;
    }

    if (GameInput->ZoomDelta == 0.f)
    {
        GameInput->ZoomDelta = (f32) MouseInput->WheelDelta;
    }

    GameInput->LeftClick.IsActivated = IsButtonActivated(&MouseInput->LeftButton);
    GameInput->MouseCoords = vec2((f32) MouseInput->x, (f32) MouseInput->y);

    MouseInput->WheelDelta = 0;
}