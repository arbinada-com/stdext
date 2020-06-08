@echo off

if "%QMAKE_EXE%"=="" (
    echo.Variable QMAKE_EXE is not specified
	exit /b 1
)

echo.Generating stdext library makefiles...
pushd ..\stdext > nul
"%QMAKE_EXE%" stdext.pro -spec win32-msvc "CONFIG+=debug stdext_gen_makefile" -nocache -nodepend
if errorlevel 1 goto error
"%QMAKE_EXE%" stdext.pro -spec win32-msvc "CONFIG+=stdext_gen_makefile" -nocache -nodepend
if errorlevel 1 goto error

echo.
echo.Generating stdext tests makefiles...
cd ..\stdext-test
"%QMAKE_EXE%" stdext-test.pro -spec win32-msvc "CONFIG+=debug stdext_gen_makefile" -nocache -nodepend
if errorlevel 1 goto error
"%QMAKE_EXE%" stdext-test.pro -spec win32-msvc "CONFIG+=stdext_gen_makefile" -nocache -nodepend
if errorlevel 1 goto error

popd
exit /b 0

:error
set err=errorlevel
popd
echo.FAILED
exit /b %err%
