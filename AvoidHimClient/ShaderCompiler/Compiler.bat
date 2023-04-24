@echo off

set /p a1="Shader name (without .hlsl) : "
set /p a2="vertex shader : "
set /p b2="pixel shader : "

start /b fxc.exe "%a1%.hlsl" /Od /Zi /T vs_5_0 /E "%a2%" /Fo "%a1%_vs"
start /b fxc.exe "%a1%.hlsl" /Od /Zi /T ps_5_0 /E "%b2%" /Fo "%a1%_ps"