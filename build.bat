@echo off
setlocal

pushd %~dp0

if not exist "build" mkdir "build"

cmake -S . -B build
cmake --build build --config Release

popd
endlocal
pause