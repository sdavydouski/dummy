//#pragma warning(push, 0)
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui.cpp>
#include <imgui_draw.cpp>
#include <imgui_widgets.cpp>
#include <imgui_tables.cpp>

#include "imgui_impl_win32.h"
#include "imgui_impl_win32.cpp"

#if NDEBUG
#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM <release/glad.h>
#else
#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM <debug/glad.h>
#endif

#include <imgui_impl_opengl3.h>
#include <imgui_impl_opengl3.cpp>
//#pragma warning(pop)

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
Win32ShutdownImGui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

internal void
RenderAnimationGraphInfo(animation_graph *Graph, u32 Depth = 0)
{
    if (!Graph) return;

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
            case AnimationNodeType_Clip:
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
RenderEntityInfo(game_entity *Entity, model *Model)
{
    ImGui::Text("Selected Entity:");

    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::InputFloat3("Position", Entity->Transform.Translation.Elements);
        ImGui::InputFloat3("Scale", Entity->Transform.Scale.Elements);
        ImGui::InputFloat4("Rotation", Entity->Transform.Rotation.Elements);
    }

    if (Model)
    {
        if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("Meshes: %d", Model->MeshCount);
            ImGui::Text("Materials: %d", Model->MaterialCount);
            ImGui::Text("\n");

            for (u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
            {
                mesh *Mesh = Model->Meshes + MeshIndex;

                ImGui::Text("Mesh %d\n", MeshIndex);
                ImGui::Text("Vertices: %d", Mesh->VertexCount);
                ImGui::Text("Indices: %d", Mesh->IndexCount);
                ImGui::Text("\n");
            }
        }
    }

    if (Entity->Body)
    {
        if (ImGui::CollapsingHeader("Ridig Body", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::InputFloat3("Position", Entity->Body->Position.Elements);
            ImGui::InputFloat3("HalfSize", Entity->Body->HalfSize.Elements);
            ImGui::InputFloat4("Orientation", Entity->Body->Orientation.Elements);
            ImGui::Text("\n");
        }
    }
}

internal void
Win32RenderDebugInfo(win32_platform_state *PlatformState, opengl_state *RendererState, game_memory *GameMemory, game_parameters *GameParameters, memory_arena *Arena)
{
    scoped_memory ScopedMemory(Arena);

    game_state *GameState = GetGameState(GameMemory);

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

#if 0
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

        if (ImGui::BeginMenu("Assets"))
        {
            if (ImGui::MenuItem("Viewer"))
            {

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
#endif

    ImGui::Begin("Stats");

    ImGui::Text("%.3f ms/frame (%.1f FPS)", GameParameters->Delta * 1000.f / PlatformState->TimeRate, PlatformState->TimeRate / GameParameters->Delta);
    ImGui::Text("Window Size: %d, %d", PlatformState->WindowWidth, PlatformState->WindowHeight);
    ImGui::Text("Time Rate: %.3f", PlatformState->TimeRate);
    ImGui::Checkbox("FullScreen", (bool *)&PlatformState->IsFullScreen);
    ImGui::Checkbox("VSync", (bool *)&PlatformState->VSync);

    ImGui::End();

    ImGui::Begin("Game State");

    ImGui::Text("Total Entities: %d", GameState->EntityCount);
    ImGui::Text("Renderable Entities: %d", GameState->RenderableEntityCount);

    if (GameState->Player->Body)
    {
        ImGui::Text("Player Position: x: %.1f, y: %.1f, z: %.1f", GameState->Player->Body->Position.x, GameState->Player->Body->Position.y, GameState->Player->Body->Position.z);
    }

    game_camera *Camera = GameState->Mode == GameMode_World
        ? &GameState->PlayerCamera
        : &GameState->FreeCamera;

    ImGui::Text("Camera Position: x: %.1f, y: %.1f, z: %.1f", Camera->Transform.Translation.x, Camera->Transform.Translation.y, Camera->Transform.Translation.z);

    ImGui::End();

    ImGui::Begin("Render Settings");
    
    ImGui::ColorEdit3("Dir Color", (f32 *) &GameState->DirectionalLight.Color);
    ImGui::SliderFloat3("Dir Direction", (f32 *) &GameState->DirectionalLight.Direction, -1.f, 1.f);
    GameState->DirectionalLight.Direction = Normalize(GameState->DirectionalLight.Direction);

    if (ImGui::BeginTable("Render toggles", 2))
    {
        ImGui::TableNextColumn();
        ImGui::Checkbox("Show Camera", (bool *) &GameState->Options.ShowCamera);
        ImGui::TableNextColumn();
        ImGui::Checkbox("Show Cascades", (bool *) &GameState->Options.ShowCascades);

        ImGui::TableNextColumn();
        ImGui::Checkbox("Show Rigid Bodies", (bool *) &GameState->Options.ShowRigidBodies);
        ImGui::TableNextColumn();
        ImGui::Checkbox("Show Skeletons", (bool *) &GameState->Options.ShowSkeletons);

        ImGui::EndTable();
    }

    ImGui::End();

    ImGui::Begin("Animation Graph");
    RenderAnimationGraphInfo(GameState->Player->Animation);
    ImGui::End();

#if 1
    ImGui::Begin("Cascaded Shadow Maps");
    {
        ImVec2 WindowSize = ImVec2(512, 512);

        if (ImGui::BeginTable("Shadow Maps", 4, ImGuiTableFlags_ScrollX))
        {
            for (u32 CascadeIndex = 0; CascadeIndex < ArrayCount(RendererState->CascadeShadowMaps); ++CascadeIndex)
            {
                ImGui::TableNextColumn();
                ImGui::Image((ImTextureID)(umm)RendererState->CascadeShadowMaps[CascadeIndex], WindowSize, ImVec2(0, 1), ImVec2(1, 0));
            }

            ImGui::EndTable();
        }
    }
    ImGui::End();
#endif

    ImGui::Begin("Debug");

    for (u32 EntityIndex = 0; EntityIndex < GameState->EntityCount; ++EntityIndex)
    {
        game_entity *Entity = GameState->Entities + EntityIndex;

        if (Entity->DebugView)
        {
            RenderEntityInfo(Entity, Entity->Model);
        }
    }

    ImGui::End();

    ImGui::Begin("Profiler");

    platform_profiler *Profiler = GameMemory->Profiler;
    profiler_frame_samples *FrameSamples = ProfilerGetPreviousFrameSamples(Profiler);

    f32 *ElapsedMillisecondsValues = PushArray(ScopedMemory.Arena, FrameSamples->SampleCount, f32);

    for (u32 SampleIndex = 0; SampleIndex < FrameSamples->SampleCount; ++SampleIndex)
    {
        profiler_sample *Sample = FrameSamples->Samples + SampleIndex;
        f32 *ElapsedMilliseconds = ElapsedMillisecondsValues + SampleIndex;
        *ElapsedMilliseconds = Sample->ElapsedMilliseconds;
    }

    ImGui::PlotLines("Frame timing", ElapsedMillisecondsValues, FrameSamples->SampleCount);

    if (ImGui::BeginTable("Profiler stats", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("Block", 0, 3.f);
        ImGui::TableSetupColumn("Ticks", 0, 1.f);
        ImGui::TableSetupColumn("Milliseconds", 0, 1.f);
        ImGui::TableHeadersRow();

        u64 TotalTicks = 0;
        f32 TotalMilliseconds = 0.f;

        for (u32 SampleIndex = 0; SampleIndex < FrameSamples->SampleCount; ++SampleIndex)
        {
            profiler_sample *Sample = FrameSamples->Samples + SampleIndex;

            ImVec4 Color = ImVec4(0.f, 1.f, 0.f, 1.f);

            if (Sample->ElapsedMilliseconds >= 1.0f)
            {
                Color = ImVec4(1.f, 0.f, 0.f, 1.f);
            }
            else if (0.1f <= Sample->ElapsedMilliseconds && Sample->ElapsedMilliseconds < 1.f)
            {
                Color = ImVec4(1.f, 1.f, 0.f, 1.f);
            }

            ImGui::TableNextColumn();
            ImGui::TextColored(Color, "%s", Sample->Name);

            ImGui::TableNextColumn();
            ImGui::TextColored(Color, "%d ticks", Sample->ElapsedTicks);

            ImGui::TableNextColumn();
            ImGui::TextColored(Color, "%.3f ms", Sample->ElapsedMilliseconds);

            TotalTicks += Sample->ElapsedTicks;
            TotalMilliseconds += Sample->ElapsedMilliseconds;
        }

        ImVec4 Color = ImVec4(0.f, 1.f, 0.f, 1.f);

        if (TotalMilliseconds >= 4.0f)
        {
            Color = ImVec4(1.f, 0.f, 0.f, 1.f);
        }
        else if (1.f <= TotalMilliseconds && TotalMilliseconds < 4.f)
        {
            Color = ImVec4(1.f, 1.f, 0.f, 1.f);
        }

        ImGui::TableNextColumn();
        ImGui::TextColored(Color, "Total");

        ImGui::TableNextColumn();
        ImGui::TextColored(Color, "%d ticks", TotalTicks);

        ImGui::TableNextColumn();
        ImGui::TextColored(Color, "%.3f ms", TotalMilliseconds);

        ImGui::EndTable();
    }

    ImGui::End();

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
