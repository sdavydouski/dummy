# Dummy Game Engine

![Screenshot 2024-08-04 122314](https://github.com/user-attachments/assets/83d88bb3-1885-40cd-b30e-f36e10aaf099)

## Building
### Windows
#### Prerequisites
* Windows 10 or Windows 11 (x64)
* Visual Studio 2022
* CMake
#### How to build
Run commands from [Visual Studio Developer Command Prompt](https://learn.microsoft.com/en-us/visualstudio/ide/reference/command-prompt-powershell?view=vs-2022).
```
git submodule init
git submodule update
misc\assimp_build.bat
```
Visual Studio solution is available at `src\dummy.sln`. After successful build the resulting executables and DLL can be found in `game` directory. Run `assets_builder.exe` to generate engine assets. Run `win32_dummy.exe` to launch the engine.
### Linux
Not supported
### MacOS
Not supported
