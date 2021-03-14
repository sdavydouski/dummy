#include "dummy_random.h"
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
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Consola.ttf", 16);
}

internal void
RenderAnimationGraphInfo(animation_graph *Graph, u32 Depth = 0)
{
    char Prefix[8];

    for (u32 DepthLevel = 0; DepthLevel < Depth; ++DepthLevel)
    {
        Prefix[DepthLevel] = '\t';
    }

    Prefix[Depth] = 0;

    ImGui::Text("%sActive Node: %s", Prefix, Graph->Active->Name);

    ImGui::NewLine();

    for (u32 NodeIndex = 0; NodeIndex < Graph->NodeCount; ++NodeIndex)
    {
        animation_node *Node = Graph->Nodes + NodeIndex;

        ImGui::Text("%sNode Type: %d\n", Prefix, Node->Type);
        ImGui::Text("%sNode Weight: %.3f\n", Prefix, Node->Weight);

        switch (Node->Type)
        {
            case AnimationNodeType_SingleMotion:
            {
                ImGui::Text("%s\tName: %s", Prefix, Node->Animation.Clip->Name);
                ImGui::Text("%s\tTime: %.3f", Prefix, Node->Animation.Time);

                break;
            }
            case AnimationNodeType_BlendSpace:
            {
                for (u32 Index = 0; Index < Node->BlendSpace->ValueCount; ++Index)
                {
                    blend_space_1d_value *Value = Node->BlendSpace->Values + Index;

                    ImGui::Text("%s\tName: %s", Prefix, Value->AnimationState.Clip->Name);
                    ImGui::Text("%s\tTime: %.3f", Prefix, Value->AnimationState.Time);
                    ImGui::Text("%s\tWeight: %.3f", Prefix, Value->Weight);

                    ImGui::NewLine();
                }

                break;
            }
            case AnimationNodeType_Graph:
            {
                RenderAnimationGraphInfo(Node->Graph, Depth + 1);

                break;
            }
        }

        ImGui::NewLine();
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

    ImGui::Text("Ray Direction: x: %.3f, y: %.3f, z: %.3f", GameState->Ray.Direction.x, GameState->Ray.Direction.y, GameState->Ray.Direction.z);


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
    //ImGui::Text("Orientation: x: %.1f, y: %.1f, z: %.1f, w: %.1f", GameState->Player.Orientation.x, GameState->Player.Orientation.y, GameState->Player.Orientation.z, GameState->Player.Orientation.w);

    ImGui::ColorEdit3("Directional Light Color", (f32 *)&GameState->DirectionalColor);

    ImGui::End();

    ImGui::Begin("Animation Graph");
    RenderAnimationGraphInfo(&GameState->Player.Animation);
    ImGui::End();

    ImGui::Begin("Debug");

    for (u32 EntityIndex = 0; EntityIndex < GameState->Batch.EntityCount; ++EntityIndex)
    {
        entity *Entity = GameState->Batch.Entities + EntityIndex;

        if (Entity->IsSelected)
        {
            ImGui::Text("Selected Entity:");
            ImGui::Text("Position: x: %.1f, y: %.1f, z: %.1f", Entity->Body->Position.x, Entity->Body->Position.y, Entity->Body->Position.z);
            ImGui::Text("Size: x: %.1f, y: %.1f, z: %.1f", 2.f * Entity->Body->HalfSize.x, 2.f * Entity->Body->HalfSize.y, 2.f * Entity->Body->HalfSize.z);
            ImGui::Text("\n");
        }
    }

    ImGui::End();

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
