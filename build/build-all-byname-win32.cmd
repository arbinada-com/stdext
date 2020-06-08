@echo off

if "%MAKEFILE_NAME%"=="" (
    echo.Variable MAKEFILE_NAME is not specified
	exit /b 1
)

set NMAKE_CMD=nmake
for %%a in (%*) do (
    if /i "%%a"=="rebuild" set NMAKE_CMD=%NMAKE_CMD% /A
)
echo.NMAKE_CMD: %NMAKE_CMD%

echo.Building stdext library...
pushd ..\stdext > nul
%NMAKE_CMD% /f %MAKEFILE_NAME%.Debug
if errorlevel 1 goto error
%NMAKE_CMD% /f %MAKEFILE_NAME%.Release
if errorlevel 1 goto error

echo.
echo.Generating stdext tests makefiles...
cd ..\stdext-test
%NMAKE_CMD% /f %MAKEFILE_NAME%.Debug
if errorlevel 1 goto error
%NMAKE_CMD% /f %MAKEFILE_NAME%.Release
if errorlevel 1 goto error

popd
exit /b 0

:error
set err=errorlevel
popd
echo.FAILED
exit /b %err%

