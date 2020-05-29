@echo off

call :main
exit /b 0

:main
set build_dir=".\build\external\assimp"
set libs_dir=".\build\external\libs"

call :build_assimp Debug
call :build_assimp Release

if not exist %libs_dir% mkdir %libs_dir%
pushd %libs_dir%
copy ..\assimp\lib\Debug\assimp-vc142-mtd.lib .\
copy ..\assimp\code\Debug\assimp-vc142-mtd.pdb .\
copy ..\assimp\lib\Release\assimp-vc142-mt.lib .\

copy ..\assimp\lib\Debug\IrrXMLd.lib .\
copy ..\assimp\lib\Release\IrrXML.lib .\

copy ..\assimp\lib\Debug\zlibstaticd.lib .\
copy ..\assimp\lib\Release\zlibstatic.lib .\
popd
exit /b 0

:build_assimp 
if not exist %build_dir% mkdir %build_dir%
pushd %build_dir%
cmake -D BUILD_SHARED_LIBS=OFF -D ASSIMP_NO_EXPORT=ON -D ASSIMP_BUILD_ASSIMP_TOOLS=OFF -D ASSIMP_BUILD_TESTS=OFF -D ASSIMP_INSTALL_PDB=ON ..\..\..\src\external\assimp -G "Visual Studio 16 2019" -A x64
devenv .\ASSIMP.sln /rebuild %~1 
popd
exit /b 0
