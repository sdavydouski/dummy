# Dummy Game Engine

![Screenshot 2024-08-04 122314](https://github.com/user-attachments/assets/83d88bb3-1885-40cd-b30e-f36e10aaf099)

Dummy is a compact C++ 3D game engine prototype built primarily for educational purposes. It explores the core ideas behind engine architecture and real-time rendering: scene organization, rendering, animation, spatial queries, asset pipelines, and editor tooling.

## Overview

The project is a small but broad engine playground: a custom runtime with systems for entities, rendering, animation, collisions, and editor utilities. It is designed to be readable, extensible, and useful as a practical reference for how engine subsystems fit together.

## Features

- Real-time 3D rendering with mesh submission, lighting, material handling, debug drawing, and skybox support
- Skeletal animation with clips, transitions, additive blending, and graph-driven state logic
- Visibility and culling systems based on polyhedra, frustums, and spatial hash queries
- Spatial partitioning for efficient nearby-entity lookup and scene organization
- Collision, bounds, and rigid-body-style simulation primitives
- Job queue and worker-style task scheduling primitives for background work
- Custom math utilities for vectors, matrices, transforms, quaternions, and geometric operations
- Asset loading for models, textures, fonts, and audio clips
- ImGui-based editor tooling with transform controls and node-graph editing support
- Win32 platform layer with OpenGL and Direct3D 12 rendering backends

## Project structure

- `src/` contains the engine core: runtime systems, math utilities, renderer code, animation logic, physics primitives, editor code, shader assets, and platform-specific code
- `assets/` stores project content and generated asset data
- `build/` contains generated build artifacts and intermediate output
- `game/` contains runtime artifacts and shipped executables
- `misc/` contains helper scripts and dependency setup utilities

## Quick start

### Prerequisites

- Windows 10 or 11 (x64)
- Visual Studio 2022 or newer
- CMake (required for the bundled Assimp dependency build)

### Build steps

From a [Visual Studio Developer Command Prompt](https://learn.microsoft.com/en-us/visualstudio/ide/reference/command-prompt-powershell?view=vs-2022):

```bash
git submodule init
git submodule update
misc\assimp_build.bat
```

Then open `src\dummy.sln` in Visual Studio, build the solution, and run the generated executables from the output folder:

```bash
assets_builder.exe
win32_dummy.exe
```

## Platform support

- Windows: primary supported platform
- Linux: not currently supported
- macOS: not currently supported

## Project status

This is a learning-focused engine prototype rather than a production-ready engine. It already includes several foundational systems for a small 3D game project, but its main value is educational: it exposes engine architecture, rendering pipelines, animation logic, spatial systems, math utilities, and editor/debug tooling in a compact and approachable form.

## License

This project is provided as a learning-oriented codebase and may be adapted for experimentation and personal study.
