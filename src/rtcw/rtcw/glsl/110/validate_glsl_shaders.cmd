@echo off

echo:
echo ========================================
echo Validate GLSL (v1.10) shaders
echo ========================================

glslangValidator.exe -S frag -d --no-link tess_fs.txt
if %errorlevel% neq 0 goto l_exit

glslangValidator.exe -S vert -d --no-link tess_vs.txt
if %errorlevel% neq 0 goto l_exit

echo:
echo ========================================
echo SUCCEEDED
echo ========================================
goto l_exit

:l_failed
echo:
echo ========================================
echo FAILED
echo ========================================

:l_exit
