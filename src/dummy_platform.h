#pragma once

#include "dummy_memory.h"
#include "dummy_renderer.h"

struct game_state;

enum mouse_mode
{
    MouseMode_Navigation,
    MouseMode_Cursor
};

struct read_file_result
{
    u32 Size;
    void *Contents;
};

#define PLATFORM_SET_MOUSE_MODE(name) void name(void *PlatformHandle, mouse_mode MouseMode)
typedef PLATFORM_SET_MOUSE_MODE(platform_set_mouse_mode);

#define PLATFORM_READ_FILE(name) read_file_result name(char *FileName, memory_arena *Arena, b32 Text)
typedef PLATFORM_READ_FILE(platform_read_file);

#define PLATFORM_DEBUG_PRINT_STRING(name) i32 name(const char *String, ...)
typedef PLATFORM_DEBUG_PRINT_STRING(platform_debug_print_string);

struct platform_api
{
    void *PlatformHandle;
    platform_set_mouse_mode *SetMouseMode;
    platform_read_file *ReadFile;
    platform_debug_print_string *DebugPrintString;
};

struct game_memory
{
    umm PermanentStorageSize;
    void *PermanentStorage;

    umm TransientStorageSize;
    void *TransientStorage;

    // todo: use TransientStorage for this?
    umm RenderCommandsStorageSize;
    void *RenderCommandsStorage;

    platform_api *Platform;
};

inline game_state *
GetGameState(game_memory *Memory)
{
    game_state *GameState =(game_state *)Memory->PermanentStorage;
    return GameState;
}

inline render_commands *
GetRenderCommands(game_memory *Memory)
{
    render_commands *RenderCommands = (render_commands *)Memory->RenderCommandsStorage;
    return RenderCommands;
}

inline void
ClearRenderCommands(game_memory *Memory)
{
    render_commands *RenderCommands = (render_commands *)Memory->RenderCommandsStorage;
    RenderCommands->MaxRenderCommandsBufferSize = (u32)(Memory->RenderCommandsStorageSize - sizeof(render_commands));
    RenderCommands->RenderCommandsBufferSize = 0;
    RenderCommands->RenderCommandsBuffer = (u8 *)Memory->RenderCommandsStorage + sizeof(render_commands);
}

struct platform_button_state
{
    b32 IsPressed;
    b32 WasPressed;
    b32 Toggle;
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

struct platform_input_keyboard
{
    platform_button_state Up;
    platform_button_state Down;
    platform_button_state Left;
    platform_button_state Right;

    platform_button_state Z;
    platform_button_state C;
    platform_button_state E;

    platform_button_state Tab;
    platform_button_state Ctrl;
    platform_button_state Space;
    platform_button_state Esc;
    platform_button_state Enter;

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

    platform_button_state Start;
    platform_button_state Back;
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
    game_input_action Crouch;
    game_input_action Activate;

    game_input_action ChooseHero1;
    game_input_action ChooseHero2;
    game_input_action ChooseHero3;
    game_input_action ChooseDummy;

    game_input_range Camera;
    game_input_action Menu;
    game_input_action Advance;
    game_input_state HighlightBackground;

    f32 ZoomDelta;
    vec2 MouseCoords;
    game_input_action LeftClick;
    game_input_action EditMode;
    game_input_state EnableFreeCameraMovement;
};

inline void
XboxControllerInput2GameInput(platform_input_xbox_controller *XboxControllerInput, game_input *GameInput)
{
    if (Magnitude(GameInput->Move.Range) == 0.f)
    {
        GameInput->Move.Range = XboxControllerInput->LeftStick;
    }

    if (Magnitude(GameInput->Camera.Range) == 0.f)
    {
        GameInput->Camera.Range = XboxControllerInput->RightStick;
    }

    if (!GameInput->Menu.IsActivated)
    {
        GameInput->Menu.IsActivated = XboxControllerInput->Start.IsPressed && (XboxControllerInput->Start.IsPressed != XboxControllerInput->Start.WasPressed);
    }

    if (!GameInput->HighlightBackground.IsActive)
    {
        GameInput->HighlightBackground.IsActive = XboxControllerInput->Back.IsPressed;
    }

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
    SavePrevButtonState(&KeyboardInput->Tab);
    SavePrevButtonState(&KeyboardInput->Space);
    SavePrevButtonState(&KeyboardInput->Enter);
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
    UpdateToggleButtonState(&KeyboardInput->Tab);
    UpdateToggleButtonState(&KeyboardInput->Space);
    UpdateToggleButtonState(&KeyboardInput->Enter);
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
    f32 Move = KeyboardInput->Z.Toggle ? 1.f : 0.5f;

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

    if (!GameInput->Crouch.IsActivated)
    {
        GameInput->Crouch.IsActivated = IsButtonActivated(&KeyboardInput->C);
    }

    if (!GameInput->Activate.IsActivated)
    {
        GameInput->Activate.IsActivated = IsButtonActivated(&KeyboardInput->E);
    }

    if (!GameInput->Menu.IsActivated)
    {
        GameInput->Menu.IsActivated = IsButtonActivated(&KeyboardInput->Enter);
    }

    if (!GameInput->Advance.IsActivated)
    {
        GameInput->Advance.IsActivated = IsButtonActivated(&KeyboardInput->Space);
    }

    if (!GameInput->EditMode.IsActivated)
    {
        GameInput->EditMode.IsActivated = IsButtonActivated(&KeyboardInput->Tab);
    }

    if (!GameInput->ChooseHero1.IsActivated)
    {
        GameInput->ChooseHero1.IsActivated = IsButtonActivated(&KeyboardInput->One);
    }

    if (!GameInput->ChooseHero2.IsActivated)
    {
        GameInput->ChooseHero2.IsActivated = IsButtonActivated(&KeyboardInput->Two);
    }

    if (!GameInput->ChooseHero3.IsActivated)
    {
        GameInput->ChooseHero3.IsActivated = IsButtonActivated(&KeyboardInput->Three);
    }

    if (!GameInput->ChooseDummy.IsActivated)
    {
        GameInput->ChooseDummy.IsActivated = IsButtonActivated(&KeyboardInput->Zero);
    }

    if (!GameInput->HighlightBackground.IsActive)
    {
        GameInput->HighlightBackground.IsActive = KeyboardInput->Ctrl.IsPressed;
    }
}

inline void
MouseInput2GameInput(platform_input_mouse *MouseInput, game_input *GameInput)
{
    if (Magnitude(GameInput->Camera.Range) == 0.f)
    {
        f32 MouseSensitivity = 0.05f;
        vec2 MouseMovement = vec2((f32)MouseInput->dx, (f32)MouseInput->dy) * MouseSensitivity;

        GameInput->Camera.Range = vec2(MouseMovement.x, -MouseMovement.y);
        GameInput->EnableFreeCameraMovement.IsActive = MouseInput->RightButton.IsPressed;
    }

    if (GameInput->ZoomDelta == 0.f)
    {
        GameInput->ZoomDelta = (f32)MouseInput->WheelDelta;
    }

    GameInput->LeftClick.IsActivated = IsButtonActivated(&MouseInput->LeftButton);
    GameInput->MouseCoords = vec2((f32)MouseInput->x, (f32)MouseInput->y);

    MouseInput->WheelDelta = 0;
}

struct game_parameters
{
    u32 WindowWidth;
    u32 WindowHeight;

    f32 Time;
    f32 Delta;
    f32 UpdateRate;
    f32 UpdateLag;
};

#define GAME_INIT(name) void name(game_memory *Memory)
typedef GAME_INIT(game_init);

#define GAME_PROCESS_INPUT(name) void name(game_memory *Memory, game_parameters *Parameters, game_input *Input)
typedef GAME_PROCESS_INPUT(game_process_input);

#define GAME_UPDATE(name) void name(game_memory *Memory, game_parameters *Parameters)
typedef GAME_UPDATE(game_update);

#define GAME_RENDER(name) void name(game_memory *Memory, game_parameters *Parameters)
typedef GAME_RENDER(game_render);
