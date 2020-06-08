@echo off

if not "%VSCMD_ARG_HOST_ARCH%"=="x64" (
    echo.Run this batch from VS x64 native tools command prompt
    exit /b 1
)

setlocal
call setenv.cmd

set QMAKE_EXE=%QT_MSVC_W64_BIN%\qmake.exe
echo.Generating x86_64-win32 makefiles...
call gen-makefiles-byname-win32.cmd
if errorlevel 1 goto error

exit /b 0

:error
set err=errorlevel
popd
echo.FAILED
exit /b %err%
