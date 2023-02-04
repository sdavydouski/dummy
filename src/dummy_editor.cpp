﻿#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui.cpp>
#include <imgui_draw.cpp>
#include <imgui_widgets.cpp>
#include <imgui_tables.cpp>
#include <imgui_demo.cpp>

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

inline ImVec4
NormalizeRGB(u8 r, u8 g, u8 b)
{
    ImVec4 Result = ImVec4((f32) r / 255.f, (f32) g / 255.f, (f32) b / 255.f, 1.f);
    return Result;
}

internal void
EditorSetupStyle()
{
    ImGuiStyle &Style = ImGui::GetStyle();
    ImVec4 *Colors = Style.Colors;

    const ImVec4 bgColor = NormalizeRGB(37, 37, 38);
    const ImVec4 lightBgColor = NormalizeRGB(82, 82, 85);
    const ImVec4 veryLightBgColor = NormalizeRGB(90, 90, 95);

    const ImVec4 panelColor = NormalizeRGB(51, 51, 55);
    const ImVec4 panelHoverColor = NormalizeRGB(29, 151, 236);
    const ImVec4 panelActiveColor = NormalizeRGB(0, 119, 200);

    const ImVec4 textColor = NormalizeRGB(255, 255, 255);
    const ImVec4 textDisabledColor = NormalizeRGB(151, 151, 151);
    const ImVec4 borderColor = NormalizeRGB(78, 78, 78);

    Colors[ImGuiCol_Text] = textColor;
    Colors[ImGuiCol_TextDisabled] = textDisabledColor;
    Colors[ImGuiCol_TextSelectedBg] = panelActiveColor;
    Colors[ImGuiCol_WindowBg] = bgColor;
    Colors[ImGuiCol_ChildBg] = bgColor;
    Colors[ImGuiCol_PopupBg] = bgColor;
    Colors[ImGuiCol_Border] = borderColor;
    Colors[ImGuiCol_BorderShadow] = borderColor;
    Colors[ImGuiCol_FrameBg] = panelColor;
    Colors[ImGuiCol_FrameBgHovered] = panelHoverColor;
    Colors[ImGuiCol_FrameBgActive] = panelActiveColor;
    Colors[ImGuiCol_TitleBg] = bgColor;
    Colors[ImGuiCol_TitleBgActive] = bgColor;
    Colors[ImGuiCol_TitleBgCollapsed] = bgColor;
    Colors[ImGuiCol_MenuBarBg] = panelColor;
    Colors[ImGuiCol_ScrollbarBg] = panelColor;
    Colors[ImGuiCol_ScrollbarGrab] = lightBgColor;
    Colors[ImGuiCol_ScrollbarGrabHovered] = veryLightBgColor;
    Colors[ImGuiCol_ScrollbarGrabActive] = veryLightBgColor;
    Colors[ImGuiCol_CheckMark] = panelActiveColor;
    Colors[ImGuiCol_SliderGrab] = panelHoverColor;
    Colors[ImGuiCol_SliderGrabActive] = panelActiveColor;
    Colors[ImGuiCol_Button] = panelColor;
    Colors[ImGuiCol_ButtonHovered] = panelHoverColor;
    Colors[ImGuiCol_ButtonActive] = panelHoverColor;
    Colors[ImGuiCol_Header] = panelActiveColor;
    Colors[ImGuiCol_HeaderHovered] = panelHoverColor;
    Colors[ImGuiCol_HeaderActive] = panelActiveColor;
    Colors[ImGuiCol_Separator] = borderColor;
    Colors[ImGuiCol_SeparatorHovered] = borderColor;
    Colors[ImGuiCol_SeparatorActive] = borderColor;
    Colors[ImGuiCol_ResizeGrip] = bgColor;
    Colors[ImGuiCol_ResizeGripHovered] = panelColor;
    Colors[ImGuiCol_ResizeGripActive] = lightBgColor;
    Colors[ImGuiCol_PlotLines] = panelActiveColor;
    Colors[ImGuiCol_PlotLinesHovered] = panelHoverColor;
    Colors[ImGuiCol_PlotHistogram] = panelActiveColor;
    Colors[ImGuiCol_PlotHistogramHovered] = panelHoverColor;
    Colors[ImGuiCol_DragDropTarget] = bgColor;
    Colors[ImGuiCol_NavHighlight] = bgColor;
    Colors[ImGuiCol_Tab] = bgColor;
    Colors[ImGuiCol_TabActive] = panelActiveColor;
    Colors[ImGuiCol_TabUnfocused] = bgColor;
    Colors[ImGuiCol_TabUnfocusedActive] = panelActiveColor;
    Colors[ImGuiCol_TabHovered] = panelHoverColor;

    Style.WindowRounding = 0.0f;
    Style.ChildRounding = 0.0f;
    Style.FrameRounding = 0.0f;
    Style.GrabRounding = 0.0f;
    Style.PopupRounding = 0.0f;
    Style.ScrollbarRounding = 0.0f;
    Style.TabRounding = 0.0f;
}

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

    EditorSetupStyle();

    // Init State
    //EditorState->AssetType = AssetType_Model;
    EditorState->CurrentGizmoOperation = ImGuizmo::TRANSLATE;
    EditorState->LogAutoScroll = true;

    EditorState->CurrentStreamIndex = 0;
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
EditorRenderModelInfo(model *Model)
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
            FormatString(CheckboxLabel, "Visible (Mesh Id: %d)", Mesh->MeshId);
            ImGui::Checkbox(CheckboxLabel, (bool *)&Mesh->Visible);

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

internal void
EditorRenderMaterialsInfo(opengl_state *RendererState, model *Model)
{
    if (ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_DefaultOpen))
    {
        for (u32 MaterialIndex = 0; MaterialIndex < Model->MaterialCount; ++MaterialIndex)
        {
            mesh_material *Material = Model->Materials + MaterialIndex;

            for (u32 PropertyIndex = 0; PropertyIndex < Material->PropertyCount; ++PropertyIndex)
            {
                material_property *Property = Material->Properties + PropertyIndex;

                switch (Property->Type)
                {
                    case MaterialProperty_Float_Shininess:
                    {
                        char Label[64];
                        FormatString(Label, "%s##%d", "Shininess", MaterialIndex);

                        ImGui::InputFloat(Label, &Property->Value);

                        break;
                    }
                    case MaterialProperty_Color_Ambient:
                    {
                        char Label[64];
                        FormatString(Label, "%s##%d", "Ambient Color", MaterialIndex);

                        ImGui::ColorEdit3(Label, Property->Color.Elements);

                        break;
                    }
                    case MaterialProperty_Color_Diffuse:
                    {
                        char Label[64];
                        FormatString(Label, "%s##%d", "Diffuse Color", MaterialIndex);

                        ImGui::ColorEdit3(Label, Property->Color.Elements);

                        break;
                    }
                    case MaterialProperty_Color_Specular:
                    {
                        char Label[64];
                        FormatString(Label, "%s##%d", "Specular Color", MaterialIndex);

                        ImGui::ColorEdit3(Label, Property->Color.Elements);

                        break;
                    }
                    case MaterialProperty_Texture_Diffuse:
                    {
                        ImGui::Text("Diffuse Texture");

                        opengl_texture *Texture = OpenGLGetTexture(RendererState, Property->TextureId);
                        ImGui::Image((ImTextureID)(umm) Texture->Handle, ImVec2(256, 256));

                        break;
                    }
                    case MaterialProperty_Texture_Specular:
                    {
                        ImGui::Text("Specular Texture");

                        opengl_texture *Texture = OpenGLGetTexture(RendererState, Property->TextureId);
                        ImGui::Image((ImTextureID)(umm) Texture->Handle, ImVec2(256, 256));

                        break;
                    }
                    case MaterialProperty_Texture_Shininess:
                    {
                        ImGui::Text("Shininess Texture");

                        opengl_texture *Texture = OpenGLGetTexture(RendererState, Property->TextureId);
                        ImGui::Image((ImTextureID)(umm) Texture->Handle, ImVec2(256, 256));

                        break;
                    }
                    case MaterialProperty_Texture_Normal:
                    {
                        ImGui::Text("Normal Texture");

                        opengl_texture *Texture = OpenGLGetTexture(RendererState, Property->TextureId);
                        ImGui::Image((ImTextureID)(umm) Texture->Handle, ImVec2(256, 256));

                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
        }
    }
}

internal void
EditorRenderColliderInfo(collider *Collider)
{
    if (ImGui::CollapsingHeader("Collider", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // todo:
        Assert(Collider->Type == Collider_Box);

        ImGui::InputFloat3("Center", Collider->BoxCollider.Center.Elements);
        ImGui::InputFloat3("Size", Collider->BoxCollider.Size.Elements);
        ImGui::NewLine();
    }
}

internal void
EditorRenderRigidBodyInfo(rigid_body *Body)
{
    if (ImGui::CollapsingHeader("Rigid Body", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::InputFloat3("Position", Body->Position.Elements);
        ImGui::InputFloat4("Orientation", Body->Orientation.Elements);
        ImGui::NewLine();
    }
}

internal void
EditorRenderAnimationGraphInfo(animation_graph *Graph, u32 Depth = 0)
{
    if (!Graph) return;

    if (ImGui::CollapsingHeader("Animation Graph", ImGuiTreeNodeFlags_DefaultOpen))
    {
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
                    EditorRenderAnimationGraphInfo(Node->Graph, Depth + 1);

                    break;
                }
            }

            ImGui::NewLine();
        }
    }
}

internal void
EditorRenderPointLightInfo(point_light *PointLight)
{
    if (ImGui::CollapsingHeader("PointLight", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::ColorEdit3("Color##PointLight", PointLight->Color.Elements);
        ImGui::InputFloat("Contant##PointLight", &PointLight->Attenuation.Constant);
        ImGui::InputFloat("Linear##PointLight", &PointLight->Attenuation.Linear);
        ImGui::InputFloat("Quadratic##PointLight", &PointLight->Attenuation.Quadratic);

        ImGui::NewLine();
    }
}

internal void
EditorRenderParticleEmitterInfo(particle_emitter *ParticleEmitter)
{
    if (ImGui::CollapsingHeader("ParticleEmitter", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::InputInt("Spawn##ParticleEmitter", (i32 *) &ParticleEmitter->ParticlesSpawn);
        ImGui::InputFloat2("Size##ParticleEmitter", ParticleEmitter->Size.Elements);
        ImGui::ColorEdit4("Color##ParticleEmitter", ParticleEmitter->Color.Elements, ImGuiColorEditFlags_AlphaBar);

        ImGui::NewLine();
    }
}

internal void
EditorRenderEntityInfo(editor_state *EditorState, game_state *GameState, platform_api *Platform, opengl_state *RendererState, game_entity *Entity, render_commands *RenderCommands)
{
    ImVec2 WindowSize = ImGui::GetWindowSize();

    if (Entity != GameState->Player)
    {
        if (ImGui::ButtonEx("Add player", ImVec2(WindowSize.x, 0)))
        {
            GameState->Player = Entity;
        }
    }
    else
    {
        if (ImGui::ButtonEx("Remove player", ImVec2(WindowSize.x, 0)))
        {
            GameState->Player = 0;
        }
    }

    char NameLabel[32];
    FormatString(NameLabel, "Name##%d", Entity->Id);
    ImGui::InputText(NameLabel, Entity->Name, ArrayCount(Entity->Name));

    ImGui::Text("Id: %d", Entity->Id);
    ImGui::Text("Min Grid Coords: %d, %d, %d", Entity->GridCellCoords[0].x, Entity->GridCellCoords[0].y, Entity->GridCellCoords[0].z);
    ImGui::Text("Max Grid Coords: %d, %d, %d", Entity->GridCellCoords[1].x, Entity->GridCellCoords[1].y, Entity->GridCellCoords[1].z);
    ImGui::NewLine();

    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::InputFloat3("Position (T)", Entity->Transform.Translation.Elements);
        ImGui::InputFloat3("Scale (Y)", Entity->Transform.Scale.Elements);
        ImGui::InputFloat4("Rotation (R)", Entity->Transform.Rotation.Elements);

        ImGui::NewLine();
    }

    if (Entity->Model)
    {
        EditorRenderModelInfo(Entity->Model);
        EditorRenderMaterialsInfo(RendererState, Entity->Model);
    }
    else
    {
        if (ImGui::CollapsingHeader("Add Model"))
        {
            game_assets *Assets = &GameState->Assets;

            if (ImGui::BeginListBox("##empty", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing())))
            {
                for (u32 ModelIndex = 0; ModelIndex < Assets->Models.Count; ++ModelIndex)
                {
                    model *Model = Assets->Models.Values + ModelIndex;

                    if (!IsSlotEmpty(Model->Key))
                    {
                        if (ImGui::Selectable(Model->Key))
                        {
                            AddModel(GameState, Entity, Assets, Model->Key, RenderCommands, &GameState->WorldArea.Arena);
                        }
                    }
                }

                ImGui::EndListBox();
            }
        }
    }

    if (Entity->Collider)
    {
        EditorRenderColliderInfo(Entity->Collider);
    }
    else
    {
        if (ImGui::CollapsingHeader("Add Collider"))
        {
            collider_spec *Collider = &EditorState->AddEntity.Collider;

            ImGui::InputFloat3("Size##Collider", Collider->Size.Elements);

            if (ImGui::Button("Add##Collider"))
            {
                AddBoxCollider(Entity, Collider->Size, &GameState->WorldArea.Arena);
                EditorState->AddEntity.Collider = {};
            }
        }
    }

    if (Entity->Body)
    {
        EditorRenderRigidBodyInfo(Entity->Body);
    }
    else
    {
        if (ImGui::CollapsingHeader("Add Rigid Body"))
        {
            rigid_body_spec *RigidBody = &EditorState->AddEntity.RigidBody;

            if (Entity->Animation)
            {
                ImGui::Checkbox("Root Motion##RigidBody", (bool *) &RigidBody->RootMotionEnabled);
            }

            if (ImGui::Button("Add##RigidBody"))
            {
                AddRigidBody(Entity, RigidBody->RootMotionEnabled, &GameState->WorldArea.Arena);
                EditorState->AddEntity.RigidBody = {};
            }
        }
    }

    if (Entity->Animation)
    {
        EditorRenderAnimationGraphInfo(Entity->Animation);
    }

    if (Entity->PointLight)
    {
        EditorRenderPointLightInfo(Entity->PointLight);
    }
    else
    {
        if (ImGui::CollapsingHeader("Add Point Light"))
        {
            point_light_spec *PointLight = &EditorState->AddEntity.PointLight;

            ImGui::ColorEdit3("Color##PointLight", PointLight->Color.Elements);
            ImGui::InputFloat("Contant##PointLight", &PointLight->Attenuation.Constant);
            ImGui::InputFloat("Linear##PointLight", &PointLight->Attenuation.Linear);
            ImGui::InputFloat("Quadratic##PointLight", &PointLight->Attenuation.Quadratic);

            if (ImGui::Button("Add##PointLight"))
            {
                AddPointLight(Entity, PointLight->Color, PointLight->Attenuation, &GameState->WorldArea.Arena);
                EditorState->AddEntity.PointLight = {};
            }
        }
    }

    if (Entity->ParticleEmitter)
    {
        EditorRenderParticleEmitterInfo(Entity->ParticleEmitter);
    }
    else
    {
        if (ImGui::CollapsingHeader("Add Particle Emitter"))
        {
            particle_emitter_spec *ParticleEmitter = &EditorState->AddEntity.ParticleEmitter;

            ImGui::InputInt("Count##ParticleEmitter", (i32 *) &ParticleEmitter->ParticleCount);
            ImGui::InputInt("Spawn##ParticleEmitter", (i32 *) &ParticleEmitter->ParticlesSpawn);
            ImGui::InputFloat2("Size##ParticleEmitter", ParticleEmitter->Size.Elements);
            ImGui::ColorEdit4("Color##ParticleEmitter", ParticleEmitter->Color.Elements, ImGuiColorEditFlags_AlphaBar);

            if (ImGui::Button("Add##ParticleEmitter"))
            {
                AddParticleEmitter(Entity, ParticleEmitter->ParticleCount, ParticleEmitter->ParticlesSpawn, ParticleEmitter->Color, ParticleEmitter->Size, &GameState->WorldArea.Arena);
                EditorState->AddEntity.ParticleEmitter = {};
            }
        }
    }

    ImGui::NewLine();

    if (ImGui::ButtonEx("Remove (X)", ImVec2(WindowSize.x, 0)))
    {
        RemoveGameEntity(Entity);
        GameState->SelectedEntity = 0;
    }

    if (ImGui::ButtonEx("Load...", ImVec2(WindowSize.x, 0)))
    {
        scoped_memory ScopedMemory(&EditorState->Arena);

        wchar WideFilePath[256] = L"";
        Platform->OpenFileDialog(WideFilePath, ArrayCount(WideFilePath));

        if (!StringEquals(WideFilePath, L""))
        {
            char FilePath[256];
            ConvertToString(WideFilePath, FilePath);

            LoadEntityFromFile(GameState, Entity, FilePath, Platform, RenderCommands, ScopedMemory.Arena);
        }
    }

    if (ImGui::ButtonEx("Save...", ImVec2(WindowSize.x, 0)))
    {
        scoped_memory ScopedMemory(&EditorState->Arena);

        wchar WideFilePath[256] = L"";
        Platform->SaveFileDialog(WideFilePath, ArrayCount(WideFilePath));

        if (!StringEquals(WideFilePath, L""))
        {
            char FilePath[256];
            ConvertToString(WideFilePath, FilePath);

            SaveEntityToFile(Entity, FilePath, Platform, ScopedMemory.Arena);
        }
    }

    ImGui::NewLine();

    // todo: shortcuts? https://github.com/ocornut/imgui/issues/456
    if (!ImGui::GetIO().WantCaptureKeyboard && ImGui::IsKeyPressed(ImGuiKey_X))
    {
        RemoveGameEntity(Entity);

        if (GameState->SelectedEntity == GameState->Player)
        {
            GameState->Player = 0;
        }

        GameState->SelectedEntity = 0;
    }
}

internal void
EditorAddEntity(editor_state *EditorState, game_state *GameState)
{
    game_entity *Entity = CreateGameEntity(GameState);

    Entity->Transform = CreateTransform(vec3(0.f), vec3(1.f), quat(0.f, 0.f, 0.f, 1.f));

    //AddToSpacialGrid(&GameState->WorldArea.SpatialGrid, Entity);

    GameState->SelectedEntity = Entity;

    EditorState->CurrentGizmoOperation = ImGuizmo::TRANSLATE;
}

internal void
EditorCopyEntity(editor_state *EditorState, game_state *GameState, render_commands *RenderCommands, game_entity *SourceEntity)
{
    game_entity *DestEntity = CreateGameEntity(GameState);

    CopyGameEntity(GameState, RenderCommands, SourceEntity, DestEntity);

    GameState->SelectedEntity = DestEntity;

    EditorState->CurrentGizmoOperation = ImGuizmo::TRANSLATE;
}

internal void
EditorLogWindow(editor_state *EditorState, u32 StreamCount, stream **Streams, const char **StreamNames)
{
    ImGui::Begin("Output");

    if (ImGui::BeginCombo("Streams", StreamNames[EditorState->CurrentStreamIndex]))
    {
        for (u32 StreamIndex = 0; StreamIndex < StreamCount; ++StreamIndex)
        {
            bool32 Selected = (EditorState->CurrentStreamIndex == StreamIndex);

            if (ImGui::Selectable(StreamNames[StreamIndex], Selected))
            {
                EditorState->CurrentStreamIndex = StreamIndex;
            }
        }

        ImGui::EndCombo();
    }

    stream *Stream = Streams[EditorState->CurrentStreamIndex];

    bool Clear = ImGui::Button("Clear##Log");
    ImGui::SameLine();

    bool Copy = ImGui::Button("Copy##Log");
    ImGui::SameLine();

    ImGui::Checkbox("Auto-scroll##Log", (bool *) &EditorState->LogAutoScroll);
    ImGui::SameLine();

    EditorState->LogFilter.Draw("Filter##Log", -100.0f);

    ImGui::Separator();

    if (ImGui::BeginChild("Scrolling##Log", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar))
    {
        if (Clear)
        {
            ClearStream(Stream);
        }

        if (Copy)
        {
            ImGui::LogToClipboard();
        }

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

        stream_chunk *Chunk = Stream->First;

        while (Chunk)
        {
            char *Text = (char *)Chunk->Contents;
            char *TextEnd = (char *)(Chunk->Contents + Chunk->Size);

            if ((EditorState->LogFilter.IsActive() && EditorState->LogFilter.PassFilter(Text, TextEnd)) || !EditorState->LogFilter.IsActive())
            {
                ImGui::TextUnformatted(Text, TextEnd);
            }

            Chunk = Chunk->Next;
        }

        ImGui::PopStyleVar();

        if (EditorState->LogAutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        {
            ImGui::SetScrollHereY(1.f);
        }

        ImGui::EndChild();
    }

    ImGui::End();
}

internal void
Win32RenderEditor(
    editor_state *EditorState,
    win32_platform_state *PlatformState, 
    opengl_state *RendererState, 
    xaudio2_state *AudioState, 
    game_memory *GameMemory, 
    game_parameters *GameParameters, 
    game_input *GameInput
)
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

    //ImGui::ShowDemoWindow();

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
                if (ImGui::MenuItem("Add Entity (E)"))
                {
                    EditorAddEntity(EditorState, GameState);
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Load Area..."))
                {
                    scoped_memory ScopedMemory(&EditorState->Arena);

                    wchar WideFilePath[256] = L"";
                    Platform->OpenFileDialog(WideFilePath, ArrayCount(WideFilePath));

                    if (!StringEquals(WideFilePath, L""))
                    {
                        char FilePath[256];
                        ConvertToString(WideFilePath, FilePath);

                        ClearWorldArea(GameState);
                        LoadWorldAreaFromFile(GameState, FilePath, Platform, RenderCommands, ScopedMemory.Arena);
                    }
                }

                if (ImGui::MenuItem("Save Area..."))
                {
                    scoped_memory ScopedMemory(&EditorState->Arena);

                    wchar WideFilePath[256] = L"";
                    Platform->SaveFileDialog(WideFilePath, ArrayCount(WideFilePath));

                    if (!StringEquals(WideFilePath, L""))
                    {
                        char FilePath[256];
                        ConvertToString(WideFilePath, FilePath);

                        SaveWorldAreaToFile(GameState, FilePath, Platform, ScopedMemory.Arena);
                    }

                }

                if (ImGui::MenuItem("Clear Area"))
                {
                    ClearWorldArea(GameState);
                }

                ImGui::Separator();

                if (ImGui::BeginMenu("Misc"))
                {
                    ImGui::Checkbox("Dance mode", (bool *)&GameState->DanceMode.Value);

                    ImGui::EndMenu();
                }

                ImGui::Separator();

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

#if 0
            if (ImGui::BeginMenu("Assets"))
            {
                game_assets *Assets = &GameState->Assets;
                char *Search = EditorState->Search;
                u32 SearchSize = ArrayCount(EditorState->Search);

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

                                        // todo(continue): add other components?
                                        /*bounds ModelBounds = GetEntityBounds(Entity);
                                        vec3 Size = ModelBounds.Max - ModelBounds.Min;
                                        AddBoxCollider(Entity, Size, &GameState->PermanentArena);*/

                                        AddToSpacialGrid(&GameState->WorldArea.SpatialGrid, Entity);

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

                ImGui::EndMenu();
            }
#endif

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
                Text, 
                "%.3f ms/frame (%.1f FPS) | Time scale: %.2f",
                GameParameters->UnscaledDelta * 1000.f, 
                1.f / GameParameters->UnscaledDelta,
                GameParameters->TimeScale
            );

            ImVec2 TextSize = ImGui::CalcTextSize(Text);
            ImGui::SetCursorPosX(Viewport->Size.x - TextSize.x - 10);
            ImGui::Text(Text);

            ImGui::EndMenuBar();
        }

        ImGui::End();
    }

    stream *Streams[] = { &GameState->Stream, &PlatformState->Stream, &RendererState->Stream, &AudioState->Stream };
    const char *StreamNames[] = { "Game", "Platform", "Renderer", "Audio"};

    Assert(ArrayCount(Streams) == ArrayCount(StreamNames));

    EditorLogWindow(EditorState, ArrayCount(Streams), Streams, StreamNames);

    stream *Stream = &PlatformState->Stream;

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

    //ImGui::SetNextWindowPos(ImVec2(Viewport->Size.x - 210, ImGui::GetFrameHeight() + 5));
    //ImGui::SetNextWindowSize(ImVec2(200, 0));

#if 1
    ImGui::Begin("Scene");
    
    if (ImGui::BeginListBox("##empty", ImVec2(-FLT_MIN, -FLT_MIN)))
    {
        for (u32 EntityIndex = 0; EntityIndex < GameState->WorldArea.EntityCount; ++EntityIndex)
        {
            game_entity *Entity = GameState->WorldArea.Entities + EntityIndex;

            if (!Entity->Destroyed)
            {
                char EntityName[256];
                FormatString(EntityName, "Id: %d, %s", Entity->Id, Entity->Name);

                if (ImGui::Selectable(EntityName, GameState->SelectedEntity == Entity))
                {
                    GameState->SelectedEntity = Entity;
                }
            }
        }

        ImGui::EndListBox();
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
        EditorRenderEntityInfo(EditorState, GameState, Platform, RendererState, Entity, RenderCommands);
        ImGui::End();

        // Gizmos
        ImGuizmo::Enable(!GameInput->EnableFreeCameraMovement.IsActive);
        ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

        mat4 WorldToCamera = Transpose(GetCameraTransform(Camera));
        mat4 WorldProjection = Transpose(FrustrumProjection(Camera->FieldOfView, Camera->AspectRatio, Camera->NearClipPlane, Camera->FarClipPlane));
        mat4 EntityTransform = Transpose(Transform(Entity->Transform));

        bool32 Snap = io.KeyCtrl;

        f32 SnapValue = 0.5f;
        if (EditorState->CurrentGizmoOperation == ImGuizmo::OPERATION::ROTATE)
        {
            SnapValue = 45.f;
        }

        f32 SnapValues[] = { SnapValue, SnapValue, SnapValue };

        ImGuizmo::Manipulate(
            (f32 *)WorldToCamera.Elements,
            (f32 *)WorldProjection.Elements,
            (ImGuizmo::OPERATION)EditorState->CurrentGizmoOperation,
            ImGuizmo::LOCAL, (f32 *)EntityTransform.Elements,
            0,
            Snap ? SnapValues : 0
        );

        if (ImGuizmo::IsUsing())
        {
            vec3 Translation;
            vec3 Rotation;
            vec3 Scale;
            ImGuizmo::DecomposeMatrixToComponents((f32 *)EntityTransform.Elements, Translation.Elements, Rotation.Elements, Scale.Elements);

            transform EntityTransform = {};

            EntityTransform.Translation = Translation;
            EntityTransform.Rotation = Euler2Quat(RADIANS(Rotation.z), RADIANS(Rotation.y), RADIANS(Rotation.x));
            EntityTransform.Scale = Scale;

            Entity->Transform = EntityTransform;

            // todo: probably should update colliders or smth
            if (Entity->Body)
            {
                Entity->Body->Position = Translation;
                Entity->Body->Orientation = Euler2Quat(RADIANS(Rotation.z), RADIANS(Rotation.y), RADIANS(Rotation.x));
            }

            if (Entity->Collider)
            {
                // todo: rotate collider?
                UpdateColliderPosition(Entity->Collider, Translation);
            }
        }
    }

    if (!SelectedEntity)
    {
        GameState->SelectedEntity = 0;
    }

    if ((GameState->Mode == GameMode_Editor || io.KeyCtrl) && !ImGui::GetIO().WantCaptureKeyboard)
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

        /*if (ImGui::IsKeyPressed(ImGuiKey_U))
        {
            EditorState->CurrentGizmoOperation = 0;
        }*/

        if (ImGui::IsKeyPressed(ImGuiKey_E))
        {
            EditorAddEntity(EditorState, GameState);
        }

        if (ImGui::IsKeyPressed(ImGuiKey_C))
        {
            if (GameState->SelectedEntity)
            {
                EditorCopyEntity(EditorState, GameState, RenderCommands, GameState->SelectedEntity);
            }
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Equal) || ImGui::IsKeyPressed(ImGuiKey_KeypadAdd))
        {
            GameParameters->TimeScale *= 2.f;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Minus) || ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract))
        {
            GameParameters->TimeScale /= 2.f;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Space))
        {
            if (GameParameters->TimeScale == 0.f)
            {
                GameParameters->TimeScale = GameParameters->PrevTimeScale;
            }
            else
            {
                GameParameters->PrevTimeScale = GameParameters->TimeScale;
                GameParameters->TimeScale = 0.f;
            }
        }

#if 0
        if (ImGui::IsKeyPressed(ImGuiKey_Z))
        {
            if (EditorState->SelectedModel)
            {
                // todo: move out to utility function
                game_entity *Entity = CreateGameEntity(GameState);

                Entity->Transform = CreateTransform(vec3(0.f), vec3(1.f), quat(0.f, 0.f, 0.f, 1.f));
                AddModel(GameState, Entity, &GameState->Assets, EditorState->SelectedModel->Key, RenderCommands, &GameState->PermanentArena);

                /*bounds ModelBounds = GetEntityBounds(Entity);
                vec3 Size = ModelBounds.Max - ModelBounds.Min;
                AddBoxCollider(Entity, Size, &GameState->PermanentArena);*/

                AddToSpacialGrid(&GameState->WorldArea.SpatialGrid, Entity);

                GameState->SelectedEntity = Entity;

                EditorState->CurrentGizmoOperation = ImGuizmo::TRANSLATE;
                //
            }
        }
#endif
    }

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}