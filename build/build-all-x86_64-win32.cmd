@echo off

if not "%VSCMD_ARG_TGT_ARCH%"=="x64" (
    echo.Run this batch from VS x64 native tools command prompt
    exit /b 1
)

setlocal
echo.Building x86_64-win32 targets...
set MAKEFILE_NAME=Makefile-x86_64-win32
call build-all-byname-win32.cmd %*
if errorlevel 1 goto error

exit /b 0

:error
set err=errorlevel
echo.FAILED
exit /b %err%
