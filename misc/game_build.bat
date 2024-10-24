@echo off

set compiler_flags=-MDd -nologo -Od -W3 -Zi -EHsc -std:c++20
set linker_flags= -incremental:no -pdb:%random%_dummy.pdb
set build_dir=".\build\x64\Debug\"
set lock_file_name="dummy_lock.tmp"

if not exist %build_dir% mkdir %build_dir%

pushd %build_dir%
del *dummy.pdb > nul 2> nul
echo "Waiting for pdb..." > %lock_file_name%
cl %compiler_flags% "..\..\..\src\dummy.cpp" -LDd /link %linker_flags%
del %lock_file_name%
popd
