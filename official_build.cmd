@echo off


echo * Usage:
echo * official_build.cmd {binary-dir} {installer-name}
echo * {binary-dir} is relative to root\ directory.  if it's omitted,
echo * then bin-release will be used.
echo * {installer-name} may also be omitted... "Screenie" is the default.
echo.  

  pushd %~dp0
  set path=%ProgramFiles%\nsis;%ProgramFiles(X86)%\nsis;%path%

echo Confirming arguments / variables...
  set svnroot=%cd%

  set bindir=%1
  if "%1" == "" set bindir=bin-release
  if "%1" == "" set /P bindir=Enter the binaries directory, or leave it blank to use bin-release:

  set installname=%2
  if "%2" == "" set installname=Screenie
  if "%2" == "" set /P installname=Enter the installer name, or leave it blank to use "Screenie":
  
  echo   svnroot     =%svnroot%
  echo   bindir      =%bindir%
  echo   installname =%installname%
  echo.

echo Checking for tools / files...
  tools\where.exe makensis.exe >nul
  if %errorlevel% gtr 0 goto NsisNotFound
  if not exist "%svnroot%\%bindir%\screenie.exe" goto NotBuilt

echo Generating output directory...
  md "%svnroot%\distro" 2>nul
  set outdir=%svnroot%\distro\Out
  md "%outdir%" 2>nul

echo Copying program binaries...
  copy "%svnroot%\%bindir%\screenie.exe" "%outdir%" >nul

echo Generating version information for "%svnroot%\source\client"
  "%svnroot%\tools\SubWCRev.exe" "%svnroot%\source\client" "%svnroot%\source\installer\screenie.nsi" "%outdir%\installer.nsi"
  "%svnroot%\tools\SubWCRev.exe" "%svnroot%\source\client" "%svnroot%\distro\ver_in.xml" "%outdir%\ver_out.xml"
  REM The thing will return an error if there are local modifications.
  REM if %errorlevel% == 0 goto SubWCRevError

echo Setting up version info...
  "%svnroot%\tools\veredit.exe" "%outdir%\screenie.exe" /xml "%outdir%\ver_out.xml"
  if %errorlevel% gtr 0 goto VereditError

echo Building Installer...
  rem /O"%outdir%\nsislog.txt" 
  makensis.exe /Dinstallname="%installname%" /Dinfile="%outdir%\screenie.exe" /Doutfile="%outdir%\ScreenieSetup.exe" "%outdir%\installer.nsi"
  if %errorlevel% gtr 0 goto NSISError

echo Zipping it...
  rem -j = don't store dir names
  rem -9 = compress better.
  zip -j -9 "%outdir%\ScreenieSetup.zip" "%outdir%\ScreenieSetup.exe"
  
echo Opening the output folder
  explorer "%outdir%"

echo All done!  SUCCESS.
  cd /d %svnroot%
  goto End


:NsisNotFound
echo !!makensis.exe was not found in your path.  Make sure you have nsis installed, and it's location is in your path environment variable. Stopping.
goto End

:NSISError
echo !!NSIS returned an error.  Stopping.
goto End

:SubWCRevError
echo !!SubWCRev returned an error. Stopping
goto End

:VereditError
echo !!veredit returned an error.  Stopping.
goto End

:NotBuilt
echo !!screenie.exe has not been built.  Stopping.
goto End

:TextReplaceFailed
echo !!textreplace.exe failed.  Stopping.
goto End

:End
echo.
pause
