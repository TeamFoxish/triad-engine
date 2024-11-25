@echo off

echo "Cleaning solution..."
rd /s /q "%~dp0..\build"

echo "Cleaning build cache..."
rd /s /q "%~dp0..\intermediate"

echo "Cleaning builded artifacts..."
rd /s /q "%~dp0..\bin"

echo "Building MSVC solution..."
%~dp0/../premake5.exe --file="%~dp0/../premake5.lua" vs2022

echo "Building projects..."
msbuild /m /p:Configuration=DebugEditor /p:Platform=Win64 ../build/Triad.sln
