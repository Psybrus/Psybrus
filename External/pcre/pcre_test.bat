@REM This is a generated file.
@echo off
setlocal
SET srcdir="C:\Dev\Psybrus\External\pcre"
SET pcretest="C:\Dev\Psybrus\External\pcre\DEBUG\pcretest.exe"
if not [%CMAKE_CONFIG_TYPE%]==[] SET pcretest="C:\Dev\Psybrus\External\pcre\%CMAKE_CONFIG_TYPE%\pcretest.exe"
call %srcdir%\RunTest.Bat
if errorlevel 1 exit /b 1
echo RunTest.bat tests successfully completed
