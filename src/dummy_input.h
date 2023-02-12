#pragma once

struct platform_button_state
{
    bool32 IsPressed;
    bool32 WasPressed;
    bool32 Toggle;
};

struct platform_input_keyboard
{
    union
    {
        struct
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
            platform_button_state Backspace;

            platform_button_state Plus;
            platform_button_state Minus;

            platform_button_state One;
            platform_button_state Two;
            platform_button_state Three;
            platform_button_state Zero;
        };

        platform_button_state Buttons[32];
    };
};

struct platform_input_mouse
{
    i32 x;
    i32 y;

    i32 dx;
    i32 dy;

    i32 WheelDelta;

    union
    {
        struct
        {
            platform_button_state LeftButton;
            platform_button_state RightButton;
        };

        platform_button_state Buttons[2];
    };
};

struct platform_input_xbox_controller
{
    vec2 LeftStick;
    vec2 RightStick;

    f32 LeftTrigger;
    f32 RightTrigger;

    union
    {
        struct
        {
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

        platform_button_state Buttons[12];
    };
};

struct game_input_action
{
    bool32 IsActivated;
};

struct game_input_state
{
    bool32 IsActive;
};

struct game_input_range
{
    vec2 Range;
};

struct game_input
{
    game_input_range Move;
    game_input_action Jump;
    game_input_action Dance;
    game_input_action Activate;
    game_input_action LightAttack;
    game_input_action StrongAttack;

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

inline bool32
IsButtonActivated(platform_button_state *Button)
{
    bool32 Result = Button->IsPressed && (Button->IsPressed != Button->WasPressed);
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
    for (u32 ButtonIndex = 0; ButtonIndex < ArrayCount(XboxControllerInput->Buttons); ++ButtonIndex)
    {
        SavePrevButtonState(&XboxControllerInput->Buttons[ButtonIndex]);
    }
}

inline void
EndProcessXboxControllerInput(platform_input_xbox_controller *XboxControllerInput)
{
    for (u32 ButtonIndex = 0; ButtonIndex < ArrayCount(XboxControllerInput->Buttons); ++ButtonIndex)
    {
        UpdateToggleButtonState(&XboxControllerInput->Buttons[ButtonIndex]);
    }
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
        f32 StickSensitivity = 0.01f;
        GameInput->Camera.Range = XboxControllerInput->RightStick * StickSensitivity;
    }

    ProcessInputAction(&GameInput->Menu, &XboxControllerInput->Start);
    ProcessInputAction(&GameInput->ChoosePrevHero, &XboxControllerInput->DradLeft);
    ProcessInputAction(&GameInput->ChooseNextHero, &XboxControllerInput->DradRight);
    ProcessInputAction(&GameInput->LightAttack, &XboxControllerInput->X);
    ProcessInputAction(&GameInput->StrongAttack, &XboxControllerInput->Y);
    ProcessInputAction(&GameInput->Dance, &XboxControllerInput->B);
    ProcessInputAction(&GameInput->Jump, &XboxControllerInput->A);

    ProcessInputState(&GameInput->HighlightBackground, &XboxControllerInput->Back);

    if (GameInput->ZoomDelta == 0.f)
    {
        f32 TriggerSensitivity = 0.1f;

        if (XboxControllerInput->LeftTrigger > 0.f)
        {
            GameInput->ZoomDelta = -XboxControllerInput->LeftTrigger * TriggerSensitivity;
        }

        if (XboxControllerInput->RightTrigger > 0.f)
        {
            GameInput->ZoomDelta = XboxControllerInput->RightTrigger * TriggerSensitivity;
        }
    }
}

inline void
BeginProcessKeyboardInput(platform_input_keyboard *KeyboardInput)
{
    for (u32 ButtonIndex = 0; ButtonIndex < ArrayCount(KeyboardInput->Buttons); ++ButtonIndex)
    {
        SavePrevButtonState(&KeyboardInput->Buttons[ButtonIndex]);
    }
}

inline void
EndProcessKeyboardInput(platform_input_keyboard *KeyboardInput)
{
    for (u32 ButtonIndex = 0; ButtonIndex < ArrayCount(KeyboardInput->Buttons); ++ButtonIndex)
    {
        UpdateToggleButtonState(&KeyboardInput->Buttons[ButtonIndex]);
    }
}

inline void
BeginProcessMouseInput(platform_input_mouse *MouseInput)
{
    for (u32 ButtonIndex = 0; ButtonIndex < ArrayCount(MouseInput->Buttons); ++ButtonIndex)
    {
        SavePrevButtonState(&MouseInput->Buttons[ButtonIndex]);
    }
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

    ProcessInputAction(&GameInput->Jump, &KeyboardInput->Space);
    ProcessInputAction(&GameInput->Dance, &KeyboardInput->C);
    ProcessInputAction(&GameInput->Activate, &KeyboardInput->E);
    ProcessInputAction(&GameInput->Reset, &KeyboardInput->Backspace);
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
        f32 MouseSensitivity = 0.0005f;
        vec2 MouseMovement = vec2((f32) MouseInput->dx, (f32) MouseInput->dy) * MouseSensitivity;

        GameInput->Camera.Range = vec2(MouseMovement.x, -MouseMovement.y);
    }

    if (GameInput->ZoomDelta == 0.f)
    {
        GameInput->ZoomDelta = (f32) MouseInput->WheelDelta;
        MouseInput->WheelDelta = 0;
    }

    GameInput->MouseCoords = vec2((f32) MouseInput->x, (f32) MouseInput->y);

    ProcessInputAction(&GameInput->LeftClick, &MouseInput->LeftButton);
    ProcessInputAction(&GameInput->LightAttack, &MouseInput->LeftButton);
    ProcessInputAction(&GameInput->StrongAttack, &MouseInput->RightButton);

    ProcessInputState(&GameInput->EnableFreeCameraMovement, &MouseInput->RightButton);
}