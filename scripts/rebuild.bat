@echo off

echo "Cleaning solution..."
rd /s /q "%~dp0..\build"

echo "Building MSVC solution..."
%~dp0/../premake5.exe --file="%~dp0/../premake5.lua" vs2022

echo "Building projects..."
msbuild /m /p:Configuration=DebugEditor /p:Platform=Win64 ../build/Triad.sln