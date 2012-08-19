@echo off
cd ../Wwise
"%WWISESDK%/../Authoring/Win32/Release/bin/WwiseCLI.exe" TemplateGame.wproj -GenerateSoundBanks -Platform Windows -Language English(US) -HeaderFile
pause
