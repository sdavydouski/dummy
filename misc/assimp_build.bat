@echo off

call :main
exit /b 0

:main
set build_dir=".\build\external\assimp"
set libs_dir=".\build\external\libs"

if not exist %libs_dir% mkdir %libs_dir%
pushd %libs_dir%
copy ..\assimp\code\Debug\assimp-vc143-mtd.lib .\
copy ..\assimp\code\Release\assimp-vc143-mt.lib .\

copy ..\assimp\contrib\irrXML\Debug\IrrXMLd.lib .\
copy ..\assimp\contrib\irrXML\Release\IrrXML.lib .\

copy ..\assimp\contrib\zlib\Debug\zlibstaticd.lib .\
copy ..\assimp\contrib\zlib\Release\zlibstatic.lib .\

copy ..\assimp\code\Debug\assimp-vc143-mtd.pdb .\
copy ..\assimp\contrib\zlib\Debug\zlibstaticd.pdb .\
copy ..\assimp\contrib\irrXML\Debug\IrrXMLd.pdb .\
popd
exit /b 0

:build_assimp 
if not exist %build_dir% mkdir %build_dir%
pushd %build_dir%
cmake -D BUILD_SHARED_LIBS=OFF -D ASSIMP_NO_EXPORT=ON -D ASSIMP_BUILD_ASSIMP_TOOLS=OFF -D ASSIMP_BUILD_TESTS=OFF -D ASSIMP_INSTALL_PDB=ON ..\..\..\src\external\assimp -G "Visual Studio 17 2022" -A x64
devenv .\ASSIMP.sln /rebuild %~1 
popd
exit /b 0
