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
RenderDebugInfo(win32_platform_state *PlatformState, game_memory *GameMemory, game_parameters *GameParameters)
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

    ImGui::Begin("Game State");

#if 1
    ImGui::Text("Player:");

    ImGui::Text("Position: x: %.3f, y: %.3f, z: %.3f", GameState->Player.RigidBody->Position.x, GameState->Player.RigidBody->Position.y, GameState->Player.RigidBody->Position.z);
    ImGui::Text("State: %d", GameState->Player.State);

    skeleton *Skeleton = GameState->Player.Model->Skeleton;
    joint_pose *RootLocalJointPose = Skeleton->LocalJointPoses + 0;
    joint_pose *RootTranslationLocalJointPose = Skeleton->LocalJointPoses + 1;
    ImGui::SliderFloat3("Root", (f32 *)&RootLocalJointPose->Translation, -10.f, 10.f);
    ImGui::SliderFloat3("Root Translation", (f32 *)&RootTranslationLocalJointPose->Translation, -10.f, 10.f);

    //ImGui::SliderFloat2("Blend", (f32 *)&GameState->Blend, 0.f, 1.f);

    static int e = 0;
    ImGui::RadioButton("YBot", &e, 0);
    ImGui::RadioButton("Mutant", &e, 1);
    ImGui::RadioButton("Arissa", &e, 2);

    if (e == 0)
    {
        GameState->Player.Model = &GameState->YBotModel;

        for (u32 AnimationStateIndex = 0; AnimationStateIndex < GameState->PlayerAnimationStateSet.AnimationStateCount; ++AnimationStateIndex)
        {
            animation_clip_state *AnimationState = GameState->PlayerAnimationStateSet.AnimationStates + AnimationStateIndex;
            AnimationState->Animation = GameState->Player.Model->Animations + AnimationStateIndex;
        }
    }
    else if (e == 1)
    {
        GameState->Player.Model = &GameState->MutantModel;

        for (u32 AnimationStateIndex = 0; AnimationStateIndex < GameState->PlayerAnimationStateSet.AnimationStateCount; ++AnimationStateIndex)
        {
            animation_clip_state *AnimationState = GameState->PlayerAnimationStateSet.AnimationStates + AnimationStateIndex;
            AnimationState->Animation = GameState->Player.Model->Animations + AnimationStateIndex;
        }
    }
    else if (e == 2)
    {
        GameState->Player.Model = &GameState->ArissaModel;

        for (u32 AnimationStateIndex = 0; AnimationStateIndex < GameState->PlayerAnimationStateSet.AnimationStateCount; ++AnimationStateIndex)
        {
            animation_clip_state *AnimationState = GameState->PlayerAnimationStateSet.AnimationStates + AnimationStateIndex;
            AnimationState->Animation = GameState->Player.Model->Animations + AnimationStateIndex;
        }
    }

    ImGui::Text("--------------");
    ImGui::Text("Animations:");

    for (u32 AnimationStateIndex = 0; AnimationStateIndex < GameState->PlayerAnimationStateSet.AnimationStateCount; ++AnimationStateIndex)
    {
        animation_clip_state *AnimationState = GameState->PlayerAnimationStateSet.AnimationStates + AnimationStateIndex;

        ImGui::Text("Name: %s", AnimationState->Animation->Name);
        ImGui::Text("Time: %.3f", AnimationState->Time);

        // todo: replace with radio buttons
        char CheckboxLabel[256];
        FormatString(CheckboxLabel, sizeof(CheckboxLabel), "Enabled: #%d", AnimationStateIndex);
        ImGui::Checkbox(CheckboxLabel, (bool *)&AnimationState->IsEnabled);

        char PlaybackRateLabel[256];
        FormatString(PlaybackRateLabel, sizeof(PlaybackRateLabel), "PlaybackRate: #%d", AnimationStateIndex);
        ImGui::SliderFloat(PlaybackRateLabel, &AnimationState->PlaybackRate, 0.1f, 2.f);

        ImGui::Text("--------------");
    }
#endif

    ImGui::End();

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
