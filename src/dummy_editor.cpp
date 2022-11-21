#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui.cpp>
#include <imgui_draw.cpp>
#include <imgui_widgets.cpp>
#include <imgui_tables.cpp>

#include <ImGuizmo.h>
#include <ImGuizmo.cpp>

#include <imgui_impl_win32.h>
#include <imgui_impl_win32.cpp>

#if NDEBUG
#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM <release/glad.h>
#else
#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM <debug/glad.h>
#endif

#include <imgui_impl_opengl3.h>
#include <imgui_impl_opengl3.cpp>

#include "dummy.cpp"
#include "dummy_editor.h"

internal void
Win32InitEditor(win32_platform_state *PlatformState, editor_state *EditorState)
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

    // Init State
    EditorState->AssetType = AssetType_Model;
}

internal void
Win32ShutdownEditor()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

inline const char *
GetAnimationNodeTypeName(animation_node_type NodeType)
{
    switch (NodeType)
    {
        case AnimationNodeType_Clip: return "Clip";
        case AnimationNodeType_BlendSpace: return "BlendSpace";
        case AnimationNodeType_Graph: return "Graph";
        default: return "";
    }
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

    ImGui::Text("%sAnimator: %s", Prefix, Graph->Animator);
    ImGui::Text("%sActive Node: %s", Prefix, Graph->Active->Name);

    ImGui::NewLine();

    for (u32 NodeIndex = 0; NodeIndex < Graph->NodeCount; ++NodeIndex)
    {
        animation_node *Node = Graph->Nodes + NodeIndex;

        ImGui::Text("%sNode Type: %s\n", Prefix, GetAnimationNodeTypeName(Node->Type));
        ImGui::Text("%sNode Weight: %.6f\n", Prefix, Node->Weight);

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
                    ImGui::Text("%s\tWeight: %.6f", Prefix, Value->Weight);

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
RenderEntityInfo(game_state *GameState, game_entity *Entity, model *Model)
{
    ImVec2 WindowSize = ImGui::GetWindowSize();

    ImGui::Text("Id: %d", Entity->Id);
    ImGui::Text("Min Grid Coords: %d, %d, %d", Entity->GridCellCoords[0].x, Entity->GridCellCoords[0].y, Entity->GridCellCoords[0].z);
    ImGui::Text("Max Grid Coords: %d, %d, %d", Entity->GridCellCoords[1].x, Entity->GridCellCoords[1].y, Entity->GridCellCoords[1].z);
    ImGui::NewLine();

    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::InputFloat3("Position (T)", Entity->Transform.Translation.Elements);
        ImGui::InputFloat3("Scale (Y)", Entity->Transform.Scale.Elements);
        ImGui::InputFloat4("Rotation (R)", Entity->Transform.Rotation.Elements);
    }

    ImGui::NewLine();

    if (Model)
    {
        if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("Meshes: %d", Model->MeshCount);
            ImGui::Text("Materials: %d", Model->MaterialCount);
            ImGui::Text("\n");

            u32 TotalVertexCount = 0;
            u32 TotalIndexCount = 0;

            for (u32 MeshIndex = 0; MeshIndex < Model->MeshCount; ++MeshIndex)
            {
                mesh *Mesh = Model->Meshes + MeshIndex;

                char CheckboxLabel[32];
                FormatString(CheckboxLabel, "Visible (Mesh Id: %d)", Mesh->Id);
                ImGui::Checkbox(CheckboxLabel, (bool *) &Mesh->Visible);

                ImGui::Text("Vertices: %d", Mesh->VertexCount);
                ImGui::Text("Indices: %d", Mesh->IndexCount);

                ImGui::NewLine();

                TotalVertexCount += Mesh->VertexCount;
                TotalIndexCount += Mesh->IndexCount;
            }

            ImGui::Text("Total Vertices: %d", TotalVertexCount);
            ImGui::Text("Total Indices: %d", TotalIndexCount);

            ImGui::NewLine();
        }
    }

    if (Entity->Collider)
    {
        if (ImGui::CollapsingHeader("Collider", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // todo:
            Assert(Entity->Collider->Type == Collider_Box);

            ImGui::InputFloat3("Center", Entity->Collider->BoxCollider.Center.Elements);
            ImGui::InputFloat3("Size", Entity->Collider->BoxCollider.Size.Elements);
            ImGui::NewLine();
        }
    }

    if (Entity->Body)
    {
        if (ImGui::CollapsingHeader("Ridig Body", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::InputFloat3("Position", Entity->Body->Position.Elements);
            ImGui::InputFloat4("Orientation", Entity->Body->Orientation.Elements);
            ImGui::NewLine();
        }
    }

    if (Entity->Animation)
    {
        if (ImGui::CollapsingHeader("Animation Graph", ImGuiTreeNodeFlags_DefaultOpen))
        {
            RenderAnimationGraphInfo(Entity->Animation);
        }
    }

    if (Entity->PointLight)
    {
        if (ImGui::CollapsingHeader("PointLight", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::ColorEdit3("Color", Entity->PointLight->Color.Elements);
        }
    }

    if (ImGui::ButtonEx("Remove (X)", ImVec2(WindowSize.x, 0)))
    {
        RemoveGameEntity(Entity);
        GameState->SelectedEntity = 0;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_X))
    {
        RemoveGameEntity(Entity);
        GameState->SelectedEntity = 0;
    }
}

internal void
Win32RenderEditor(win32_platform_state *PlatformState, opengl_state *RendererState, game_memory *GameMemory, game_parameters *GameParameters, editor_state *EditorState)
{
    memory_arena *Arena = &EditorState->Arena;

    scoped_memory ScopedMemory(Arena);

    game_state *GameState = GetGameState(GameMemory);
    platform_api *Platform = GameMemory->Platform;
    render_commands *RenderCommands = GetRenderCommands(GameMemory);

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

    const ImGuiViewport *Viewport = ImGui::GetMainViewport();
    ImGuiIO &io = ImGui::GetIO();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(Viewport->Size.x, ImGui::GetFrameHeight()));

    ImGuiWindowFlags Flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_MenuBar;

    game_camera *Camera = GameState->Mode == GameMode_World
        ? &GameState->GameCamera
        : &GameState->EditorCamera;

    if (ImGui::Begin("MenuBar", 0, Flags))
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Game"))
            {
                if (ImGui::BeginMenu("Stats"))
                {
                    if (GameState->Player->Body)
                    {
                        ImGui::Text("Player Position: x: %.1f, y: %.1f, z: %.1f", GameState->Player->Body->Position.x, GameState->Player->Body->Position.y, GameState->Player->Body->Position.z);
                    }

                    ImGui::Text("Camera Position: x: %.1f, y: %.1f, z: %.1f", Camera->Transform.Translation.x, Camera->Transform.Translation.y, Camera->Transform.Translation.z);

                    ImGui::Checkbox("(| - _ - |)", (bool *) &GameState->DanceMode);

                    ImGui::EndMenu();
                }

                if (ImGui::MenuItem("Save Area"))
                {
                    scoped_memory ScopedMemory(&EditorState->Arena);

                    wchar WideFilePath[256] = L"";
                    Platform->SaveFileDialog(WideFilePath, ArrayCount(WideFilePath));

                    if (!StringEquals(WideFilePath, L""))
                    {
                        char FilePath[256];
                        ConvertToString(WideFilePath, FilePath);

                        SaveArea(GameState, FilePath, Platform, ScopedMemory.Arena);
                    }

                }

                if (ImGui::MenuItem("Load Area"))
                {
                    wchar WideFilePath[256] = L"";
                    Platform->OpenFileDialog(WideFilePath, ArrayCount(WideFilePath));

                    if (!StringEquals(WideFilePath, L""))
                    {
                        char FilePath[256];
                        ConvertToString(WideFilePath, FilePath);

                        LoadArea(GameState, FilePath, Platform, RenderCommands, &GameState->PermanentArena);
                        GameState->Options.ShowGrid = false;
                    }
                }

                if (ImGui::MenuItem("Exit (Esc)"))
                {
                    PlatformState->IsGameRunning = false;
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Settings"))
            {
                if (ImGui::BeginMenu("Graphics"))
                {
                    ImGui::ColorEdit3("Dir Color", (f32 *) &GameState->DirectionalLight.Color);
                    ImGui::SliderFloat3("Dir Direction", (f32 *) &GameState->DirectionalLight.Direction, -1.f, 1.f);
                    GameState->DirectionalLight.Direction = Normalize(GameState->DirectionalLight.Direction);

                    if (ImGui::BeginTable("Graphics toggles", 2))
                    {
                        ImGui::TableNextColumn();
                        ImGui::Checkbox("Show Camera", (bool *) &GameState->Options.ShowCamera);
                        ImGui::TableNextColumn();
                        ImGui::Checkbox("Show Cascades", (bool *) &GameState->Options.ShowCascades);

                        ImGui::TableNextColumn();
                        ImGui::Checkbox("Show Bounding Volumes", (bool *) &GameState->Options.ShowBoundingVolumes);
                        ImGui::TableNextColumn();
                        ImGui::Checkbox("Show Skeletons", (bool *) &GameState->Options.ShowSkeletons);

                        ImGui::TableNextColumn();
                        ImGui::Checkbox("Show Grid", (bool *) &GameState->Options.ShowGrid);

                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Checkbox("FullScreen", (bool *) &PlatformState->IsFullScreen);

                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Checkbox("VSync", (bool *) &PlatformState->VSync);

                        ImGui::EndTable();
                    }

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Audio"))
                {
                    ImGui::SliderFloat("Master Volume", &GameState->MasterVolume, 0.f, 1.f);

                    ImGui::EndMenu();
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Profiler"))
            {
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

                ImGui::EndMenu();
            }

            char Text[256];
            FormatString(
                Text, "%.3f ms/frame (%.1f FPS) | Renderable Entities: %d | Total Active Entities: %d", 
                GameParameters->Delta * 1000.f / PlatformState->TimeRate, PlatformState->TimeRate / GameParameters->Delta, GameState->RenderableEntityCount, GameState->ActiveEntitiesCount
            );

            ImVec2 TextSize = ImGui::CalcTextSize(Text);
            ImGui::SetCursorPosX(Viewport->Size.x - TextSize.x - 10);
            ImGui::Text(Text);

            ImGui::EndMenuBar();
        }

        ImGui::End();
    }

    ImGui::SetNextWindowPos(ImVec2(Viewport->Pos.x, ImGui::GetFrameHeight() + 5 ));

#if 0
    // todo: find a better place to put it
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

    bool SelectedEntity = true;

    if (GameState->SelectedEntity)
    {
        game_entity *Entity = GameState->SelectedEntity;

        ImGui::SetNextWindowPos(ImVec2(Viewport->Size.x - 500, ImGui::GetFrameHeight() + 5));
        ImGui::SetNextWindowSize(ImVec2(500, 0));

        ImGui::Begin("Entity", &SelectedEntity);
        RenderEntityInfo(GameState, Entity, Entity->Model);
        ImGui::End();

        // Gizmos
        ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

        mat4 WorldToCamera = Transpose(GetCameraTransform(Camera));
        mat4 WorldProjection = Transpose(FrustrumProjection(Camera->FieldOfView, Camera->AspectRatio, Camera->NearClipPlane, Camera->FarClipPlane));
        mat4 EntityTransform = Transpose(Transform(Entity->Transform));

        b32 Snap = io.KeyCtrl;

        f32 SnapValue = 0.5f;
        if (EditorState->CurrentGizmoOperation == ImGuizmo::OPERATION::ROTATE)
        {
            SnapValue = 45.f;
        }

        f32 SnapValues[] = { SnapValue, SnapValue, SnapValue };

        ImGuizmo::Manipulate(
            (f32 *) WorldToCamera.Elements, 
            (f32 *) WorldProjection.Elements, 
            (ImGuizmo::OPERATION) EditorState->CurrentGizmoOperation,
            ImGuizmo::LOCAL, (f32 *) EntityTransform.Elements,
            0,
            Snap ? SnapValues : 0
        );

        if (ImGuizmo::IsUsing())
        {
            vec3 Translation;
            vec3 Rotation;
            vec3 Scale;
            ImGuizmo::DecomposeMatrixToComponents((f32 *) EntityTransform.Elements, Translation.Elements, Rotation.Elements, Scale.Elements);

            // todo: probably should update colliders or smth
            if (Entity->Body)
            {
                Entity->Body->Position = Translation;
                Entity->Body->Orientation = Euler2Quat(RADIANS(Rotation.z), RADIANS(Rotation.y), RADIANS(Rotation.x));
            }
            else
            {
                Entity->Transform.Translation = Translation;
                Entity->Transform.Rotation = Euler2Quat(RADIANS(Rotation.z), RADIANS(Rotation.y), RADIANS(Rotation.x));
            }

            if (Entity->Collider)
            {
                UpdateColliderPosition(Entity->Collider, Translation);
            }

            Entity->Transform.Scale = Scale;
        }
    }

    if (!SelectedEntity)
    {
        GameState->SelectedEntity = 0;
    }

    //
#if 1
    game_assets *Assets = &GameState->Assets;
    char *Search = EditorState->Search;
    u32 SearchSize = ArrayCount(EditorState->Search);

    ImGui::Begin("Assets");

    ImGui::RadioButton("Models", &EditorState->AssetType, AssetType_Model);
    ImGui::SameLine();
    ImGui::RadioButton("Fonts", &EditorState->AssetType, AssetType_Font);
    ImGui::SameLine();
    ImGui::RadioButton("Audio", &EditorState->AssetType, AssetType_AudioClip);

    ImGui::InputText("Search", Search, SearchSize);

    if (ImGui::BeginListBox("##empty", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing())))
    {
        switch (EditorState->AssetType)
        {
            case AssetType_Model:
            {
                for (u32 ModelIndex = 0; ModelIndex < Assets->Models.Count; ++ModelIndex)
                {
                    model *Model = Assets->Models.Values + ModelIndex;
                    char *Key = Model->Key;
                    b32 Selected = EditorState->SelectedAssetIndex == ModelIndex;

                    if (StringLength(Search) > 0)
                    {
                        if (StringIncludes(Model->Key, Search))
                        {
                            if (ImGui::Selectable(Model->Key, Selected))
                            {
                                EditorState->SelectedAssetIndex = ModelIndex;
                            }
                        }
                    }
                    else if (!IsSlotEmpty(Model->Key))
                    {
                        if (ImGui::Selectable(Model->Key, Selected))
                        {
                            EditorState->SelectedAssetIndex = ModelIndex;
                            EditorState->SelectedModel = Model;

                            // todo: move out to utility function
                            game_entity *Entity = CreateGameEntity(GameState);

                            Entity->Transform = CreateTransform(vec3(0.f), vec3(1.f), quat(0.f, 0.f, 0.f, 1.f));
                            AddModel(GameState, Entity, &GameState->Assets, Model->Key, RenderCommands, &GameState->PermanentArena);

                            bounds ModelBounds = GetEntityBounds(Entity);
                            vec3 Size = ModelBounds.Max - ModelBounds.Min;
                            AddBoxCollider(Entity, Size, &GameState->PermanentArena);

                            AddToSpacialGrid(&GameState->SpatialGrid, Entity);
                            
                            GameState->SelectedEntity = Entity;

                            EditorState->CurrentGizmoOperation = ImGuizmo::TRANSLATE;
                            //
                        }
                    }
                }

                break;
            }
            case AssetType_Font:
            {
                for (u32 FontIndex = 0; FontIndex < Assets->Fonts.Count; ++FontIndex)
                {
                    font *Font = Assets->Fonts.Values + FontIndex;
                    char *Key = Font->Key;
                    b32 Selected = EditorState->SelectedAssetIndex == FontIndex;

                    if (StringLength(Search) > 0)
                    {
                        if (StringIncludes(Key, Search))
                        {
                            if (ImGui::Selectable(Key, Selected))
                            {
                                EditorState->SelectedAssetIndex = FontIndex;
                            }
                        }
                    }
                    else if (!IsSlotEmpty(Key))
                    {
                        if (ImGui::Selectable(Key, Selected))
                        {
                            EditorState->SelectedAssetIndex = FontIndex;
                        }
                    }
                }

                break;
            }
            case AssetType_AudioClip:
            {
                for (u32 AudioClipIndex = 0; AudioClipIndex < Assets->AudioClips.Count; ++AudioClipIndex)
                {
                    audio_clip *AudioClip = Assets->AudioClips.Values + AudioClipIndex;
                    char *Key = AudioClip->Key;
                    b32 Selected = EditorState->SelectedAssetIndex == AudioClipIndex;

                    if (StringLength(Search) > 0)
                    {
                        if (StringIncludes(Key, Search))
                        {
                            if (ImGui::Selectable(Key, Selected))
                            {
                                EditorState->SelectedAssetIndex = AudioClipIndex;
                            }
                        }
                    }
                    else if (!IsSlotEmpty(Key))
                    {
                        if (ImGui::Selectable(Key, Selected))
                        {
                            EditorState->SelectedAssetIndex = AudioClipIndex;
                        }
                    }
                }

                break;
            }
        }

        ImGui::EndListBox();
    }

    ImGui::End();

#endif
    //

    if (GameState->Mode == GameMode_Editor)
    {
        if (ImGui::IsKeyPressed(ImGuiKey_T))
        {
            EditorState->CurrentGizmoOperation = ImGuizmo::TRANSLATE;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Y))
        {
            EditorState->CurrentGizmoOperation = ImGuizmo::SCALE;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_R))
        {
            EditorState->CurrentGizmoOperation = ImGuizmo::ROTATE;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_U))
        {
            EditorState->CurrentGizmoOperation = 0;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Z))
        {
            if (EditorState->SelectedModel)
            {
                // todo: move out to utility function
                game_entity *Entity = CreateGameEntity(GameState);

                Entity->Transform = CreateTransform(vec3(0.f), vec3(1.f), quat(0.f, 0.f, 0.f, 1.f));
                AddModel(GameState, Entity, &GameState->Assets, EditorState->SelectedModel->Key, RenderCommands, &GameState->PermanentArena);

                bounds ModelBounds = GetEntityBounds(Entity);
                vec3 Size = ModelBounds.Max - ModelBounds.Min;
                AddBoxCollider(Entity, Size, &GameState->PermanentArena);

                AddToSpacialGrid(&GameState->SpatialGrid, Entity);

                GameState->SelectedEntity = Entity;

                EditorState->CurrentGizmoOperation = ImGuizmo::TRANSLATE;
                //
            }
        }
    }

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
