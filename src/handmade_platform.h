#pragma once

#include "handmade_memory.h"
#include "handmade_renderer.h"

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

#define PLATFORM_READ_FILE(name) read_file_result name(void *PlatformHandle, char *FileName, memory_arena *Arena)
typedef PLATFORM_READ_FILE(platform_read_file);

struct platform_api
{
    void *PlatformHandle;
    platform_set_mouse_mode *SetMouseMode;
    platform_read_file *ReadFile;
};

struct game_memory
{
	umm PermanentStorageSize;
	void *PermanentStorage;

	umm TransientStorageSize;
	void *TransientStorage;

	umm RenderCommandsStorageSize;
	void *RenderCommandsStorage;

    platform_api *Platform;
};

inline render_commands *
GetRenderCommandsFromMemory(game_memory *Memory)
{
	render_commands *RenderCommands = (render_commands *)Memory->RenderCommandsStorage;
	RenderCommands->MaxRenderCommandsBufferSize = (u32)(Memory->RenderCommandsStorageSize - sizeof(render_commands));
	RenderCommands->RenderCommandsBufferSize = 0;
	RenderCommands->RenderCommandsBuffer = (u8 *)Memory->RenderCommandsStorage + sizeof(render_commands);

	return RenderCommands;
}

struct platform_button_state
{
	b32 IsPressed;
	b32 WasPressed;
};

inline void
SavePrevButtonState(platform_button_state *Button)
{
	Button->WasPressed = Button->IsPressed;
}

struct platform_input_keyboard
{
	platform_button_state Up;
	platform_button_state Down;
	platform_button_state Left;
	platform_button_state Right;

	platform_button_state Tab;
	platform_button_state Ctrl;
	platform_button_state Space;
};

struct platform_input_mouse
{
    union
    {
        struct
        {
            i32 x;
            i32 y;
        };

        struct
        {
            i32 dx;
            i32 dy;
        };
    };

    platform_button_state LeftButton;
    platform_button_state RightButton;
};

struct platform_input_xbox_controller
{
	vec2 LeftStick;
    vec2 RightStick;

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
    game_input_range Camera;
	game_input_action Menu;
    game_input_action Advance;
	game_input_state HighlightBackground;
    game_input_state EnableFreeCameraMovement;
};

inline void
XboxControllerInput2GameInput(platform_input_xbox_controller *XboxControllerInput, game_input *GameInput)
{
    GameInput->Move.Range = XboxControllerInput->LeftStick;
    GameInput->Camera.Range = XboxControllerInput->RightStick;
    GameInput->Menu.IsActivated = XboxControllerInput->Start.IsPressed && (XboxControllerInput->Start.IsPressed != XboxControllerInput->Start.WasPressed);
    GameInput->HighlightBackground.IsActive = XboxControllerInput->Back.IsPressed;
}

internal void
KeyboardInput2GameInput(platform_input_keyboard *KeyboardInput, game_input *GameInput)
{
    if (KeyboardInput->Left.IsPressed && KeyboardInput->Right.IsPressed)
    {
        GameInput->Move.Range.x = 0.f;
    }
    else if (KeyboardInput->Left.IsPressed)
    {
        GameInput->Move.Range.x = -1.f;
    }
    else if (KeyboardInput->Right.IsPressed)
    {
        GameInput->Move.Range.x = 1.f;
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
        GameInput->Move.Range.y = 1.f;
    }
    else if (KeyboardInput->Down.IsPressed)
    {
        GameInput->Move.Range.y = -1.f;
    }
    else
    {
        GameInput->Move.Range.y = 0.f;
    }

    // todo:
    GameInput->Menu.IsActivated = KeyboardInput->Tab.IsPressed && (KeyboardInput->Tab.IsPressed != KeyboardInput->Tab.WasPressed);
    GameInput->Advance.IsActivated = KeyboardInput->Space.IsPressed && (KeyboardInput->Space.IsPressed != KeyboardInput->Space.WasPressed);

    GameInput->HighlightBackground.IsActive = KeyboardInput->Ctrl.IsPressed;
}

inline void
MouseInput2GameInput(platform_input_mouse *MouseInput, game_input *GameInput, f32 Delta)
{
    if (Delta > 0.f)
    {
        f32 MouseSensitivity = (1.f / Delta) * 0.0001f;
        vec2 MouseMovement = vec2((f32)MouseInput->dx, (f32)MouseInput->dy) * MouseSensitivity;

        GameInput->Camera.Range = vec2(MouseMovement.x, -MouseMovement.y);
        GameInput->EnableFreeCameraMovement.IsActive = MouseInput->RightButton.IsPressed;
    }
}

struct game_parameters
{
	u32 WindowWidth;
	u32 WindowHeight;

	f32 Time;
	f32 Delta;
    f32 UpdateRate;
};

#define GAME_INIT(name) void name(game_memory *Memory)
typedef GAME_INIT(game_init);

#define GAME_PROCESS_INPUT(name) void name(game_memory *Memory, game_parameters *Parameters, game_input *Input)
typedef GAME_PROCESS_INPUT(game_process_input);

#define GAME_UPDATE(name) void name(game_memory *Memory, game_parameters *Parameters)
typedef GAME_UPDATE(game_update);

#define GAME_RENDER(name) void name(game_memory *Memory, game_parameters *Parameters)
typedef GAME_RENDER(game_render);
