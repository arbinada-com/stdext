@echo off

if not "%VSCMD_ARG_HOST_ARCH%"=="x86" (
    echo.Run this batch from VS x86 native tools command prompt
    exit /b 1
)

echo.Building x86_64-win32 targets...
set MAKEFILE_NAME=Makefile-i386-win32
call build-all-byname-win32.cmd %*
if errorlevel 1 goto error

exit /b 0

:error
set err=errorlevel
popd
echo.FAILED
exit /b %err%

