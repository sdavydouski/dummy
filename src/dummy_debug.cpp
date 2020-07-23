#include "dummy_collision.h"
#include "dummy_physics.h"
#include "dummy_assets.h"
#include "dummy.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui.cpp>
#include <imgui_draw.cpp>
#include <imgui_widgets.cpp>

#include "imgui_impl_win32.h"
#include "imgui_impl_win32.cpp"

#include <imgui_impl_opengl3.h>
#include <imgui_impl_opengl3.cpp>

#define WIN32_IMGUI_WND_PROC_HANDLER if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam)) { return true; }

internal void
Win32InitImGui(win32_platform_state *PlatformState)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplWin32_Init(PlatformState->WindowHandle);
    ImGui_ImplOpenGL3_Init();

    // Load Fonts
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Consola.ttf", 24);
}

internal void
RenderDebugInfo(win32_platform_state *PlatformState, game_memory *GameMemory, game_parameters *GameParameters, platform_input_mouse *MouseInput)
{
    game_state *GameState = (game_state *)GameMemory->PermanentStorage;

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Stats");

    ImGui::Text("%.3f ms/frame (%.1f FPS)", GameParameters->Delta * 1000.f, 1.f / GameParameters->Delta);
    ImGui::Text("Window Size: %d, %d", PlatformState->WindowWidth, PlatformState->WindowHeight);
    ImGui::Checkbox("FullScreen", (bool *)&PlatformState->IsFullScreen);
    ImGui::Checkbox("VSync", (bool *)&PlatformState->VSync);

    ImGui::End();

    f32 Margin = 10.f;
    ImVec2 ContainerSize = ImVec2(600.f, 400.f);
    ImGui::SetNextWindowSize(ContainerSize);
    ImGui::SetNextWindowPos(ImVec2(PlatformState->WindowWidth - ContainerSize.x - Margin, Margin));

    ImGui::Begin("Game State");

    ImGui::Text("Animations:");

    for (u32 AnimationIndex = 0; AnimationIndex < GameState->DummyModel.AnimationCount; ++AnimationIndex)
    {
        animation_clip *Animation = GameState->DummyModel.Animations + AnimationIndex;

        if (ImGui::RadioButton(Animation->Name, Animation == GameState->DummyModel.CurrentAnimation))
        {
            GameState->DummyModel.CurrentAnimation = GameState->DummyModel.Animations + AnimationIndex;
            GameState->DummyModel.CurrentTime = 0.f;
        }
    }

    ImGui::SliderFloat("Playback Rate", &GameState->DummyModel.PlaybackRate, 0.1f, 2.f);

    ImGui::End();

    ImGui::Begin("Input");

    ImGui::Text("Mouse Position: %d, %d", MouseInput->x, MouseInput->y);

    ImGui::End();

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
