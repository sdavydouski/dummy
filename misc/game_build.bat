@echo off

set compiler_flags=-MDd -nologo -Od -W3 -Zi -EHsc
set linker_flags= -incremental:no -pdb:%random%_sandbox.pdb
set build_dir=".\build\x64\Debug\"
set lock_file_name="handmade_lock.tmp"

if not exist %build_dir% mkdir %build_dir%

pushd %build_dir%
del *sandbox.pdb > nul 2> nul
echo "Waiting for pdb..." > %lock_file_name%
cl %compiler_flags% "..\..\..\src\handmade.cpp" -LDd /link %linker_flags%
del %lock_file_name%
popd
