@echo off
echo Installing debug dependencies...
conan install . -pr:a conan/windows_debug --build=missing %*
if %errorlevel% neq 0 exit /b %errorlevel%

echo.
echo Installing release dependencies...
conan install . -pr:a conan/windows_release --build=missing %*
if %errorlevel% neq 0 exit /b %errorlevel%

echo.
echo Done! You can now open the project in Visual Studio.
