#include "dummy_random.h"
#include "dummy_collision.h"
#include "dummy_physics.h"
#include "dummy_animation.h"
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
RenderAnimationGraphInfo(animation_graph *Graph)
{
    ImGui::Text("AnimationGraph:");
    ImGui::Text("---------");

    ImGui::Text("ActiveNodeIndex: %d", Graph->ActiveNodeIndex);

    for (u32 NodeIndex = 0; NodeIndex < Graph->NodeCount; ++NodeIndex)
    {
        animation_node *Node = Graph->Nodes + NodeIndex;

        if (Node->Weight > 0.f)
        {
            ImGui::Text("Node Weight: %.3f\n", Node->Weight);

            switch (Node->Type)
            {
                case AnimationNodeType_SingleMotion:
                {
                    ImGui::Text("--Name: %s", Node->Animation.Clip->Name);
                    ImGui::Text("--Time: %.3f", Node->Animation.Time);
                    ImGui::Text("--Weight: %.3f", Node->Animation.Weight);

                    ImGui::Text("----");

                    break;
                }
                case AnimationNodeType_BlendSpace:
                {
                    for (u32 Index = 0; Index < Node->BlendSpace->BlendSpaceValueCount; ++Index)
                    {
                        blend_space_1d_value *Value = Node->BlendSpace->BlendSpaceValues + Index;

                        if (Value->Weight > 0.f)
                        {
                            ImGui::Text("--Name: %s", Value->AnimationState.Clip->Name);
                            ImGui::Text("--Time: %.3f", Value->AnimationState.Time);
                            ImGui::Text("--Weight: %.3f", Value->AnimationState.Weight);

                            ImGui::Text("----");
                        }
                    }

                    break;
                }
                case AnimationNodeType_Graph:
                {
                    RenderAnimationGraphInfo(Node->Graph);

                    break;
                }
            }
        }
    }
}

internal void
RenderDebugInfo(win32_platform_state *PlatformState, game_memory *GameMemory, game_parameters *GameParameters)
{
    game_state *GameState = (game_state *)GameMemory->PermanentStorage;

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Menu", 0, ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Game"))
        {
            if (ImGui::MenuItem("Close"))
            { 
                PlatformState->IsGameRunning = false;
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Animation"))
        {
            if (ImGui::MenuItem("Viewer"))
            {

            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGui::End();

    ImGui::Begin("Stats");

    ImGui::Text("%.3f ms/frame (%.1f FPS)", GameParameters->Delta * 1000.f / PlatformState->TimeRate, PlatformState->TimeRate / GameParameters->Delta);
    ImGui::Text("Window Size: %d, %d", PlatformState->WindowWidth, PlatformState->WindowHeight);
    ImGui::Checkbox("FullScreen", (bool *)&PlatformState->IsFullScreen);
    ImGui::Checkbox("VSync", (bool *)&PlatformState->VSync);
    ImGui::SliderFloat("Time Rate", &PlatformState->TimeRate, 0.125f, 2.f);

    ImGui::End();

    ImGui::Begin("Game State");

    ImGui::Text("Player:");

    static int e0 = 0;
    ImGui::RadioButton("Show Model", &e0, 0);
    ImGui::RadioButton("Show Skeleton", &e0, 1);

    if (e0 == 0)
    {
        GameState->ShowModel = true;
        GameState->ShowSkeleton = false;
    }
    else if (e0 == 1)
    {
        GameState->ShowModel = false;
        GameState->ShowSkeleton = true;
    }

    ImGui::Text("Position: x: %.1f, y: %.1f, z: %.1f", GameState->Player.RigidBody->Position.x, GameState->Player.RigidBody->Position.y, GameState->Player.RigidBody->Position.z);
    ImGui::Text("CurrentMove: x: %.1f, y: %.1f", GameState->CurrentMove.x, GameState->CurrentMove.y);
    //ImGui::Text("Orientation: x: %.1f, y: %.1f, z: %.1f, w: %.1f", GameState->Player.Orientation.x, GameState->Player.Orientation.y, GameState->Player.Orientation.z, GameState->Player.Orientation.w);

    ImGui::ColorEdit3("Directional Light Color", (f32 *)&GameState->DirectionalColor);

    ImGui::Text("State: %d", GameState->Player.State);

    //joint_pose *RootTranslationLocalJointPose = GetRootTranslationLocalJointPose(GameState->Player.Model->Pose);
    //ImGui::Text("Root Translation: x: %.3f, y: %.3f, z: %.3f", RootTranslationLocalJointPose->Translation.x, RootTranslationLocalJointPose->Translation.y, RootTranslationLocalJointPose->Translation.z);

#if 0
    static int e1 = 0;
    ImGui::RadioButton("YBot", &e1, 0);
    ImGui::RadioButton("Mutant", &e1, 1);
    ImGui::RadioButton("Arissa", &e1, 2);

    if (e1 == 0)
    {
        GameState->Player.Model = &GameState->YBotModel;

        for (u32 AnimationStateIndex = 0; AnimationStateIndex < GameState->PlayerAnimationStateSet.AnimationStateCount; ++AnimationStateIndex)
        {
            animation_state *AnimationState = GameState->PlayerAnimationStateSet.AnimationStates + AnimationStateIndex;
            AnimationState->Clip = GameState->Player.Model->Animations + AnimationStateIndex;
        }
    }
    else if (e1 == 1)
    {
        GameState->Player.Model = &GameState->MutantModel;

        for (u32 AnimationStateIndex = 0; AnimationStateIndex < GameState->PlayerAnimationStateSet.AnimationStateCount; ++AnimationStateIndex)
        {
            animation_state *AnimationState = GameState->PlayerAnimationStateSet.AnimationStates + AnimationStateIndex;
            AnimationState->Clip = GameState->Player.Model->Animations + AnimationStateIndex;
        }
    }
    else if (e1 == 2)
    {
        GameState->Player.Model = &GameState->ArissaModel;

        for (u32 AnimationStateIndex = 0; AnimationStateIndex < GameState->PlayerAnimationStateSet.AnimationStateCount; ++AnimationStateIndex)
        {
            animation_state *AnimationState = GameState->PlayerAnimationStateSet.AnimationStates + AnimationStateIndex;
            AnimationState->Clip = GameState->Player.Model->Animations + AnimationStateIndex;
        }
    }
#endif

    ImGui::Text("--------------");

    RenderAnimationGraphInfo(&GameState->AnimationGraph);

    ImGui::End();

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
