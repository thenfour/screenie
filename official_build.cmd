@echo off

echo Confirming arguments / variables...
  if "%1" == "" goto Usage
  if "%2" == "" goto Usage

  set svnroot=%cd%
  set serial=%1
  set registrant=%2

echo Checking for tools...
  where devenv.exe >nul
  if %errorlevel% gtr 0 goto DevenvNotFound
  where makensis.exe >nul
  if %errorlevel% gtr 0 goto NsisNotFound

echo Checking for local changes...
  rem Create a temp file (SubWCRev needs to process files, but we don't need them now... so just make throwaways)
  echo. >"%temp%\tempver.in"
  "%svnroot%\tools\SubWCRev.exe" "%svnroot%" "%temp%\tempver.out" "%temp%\tempver.in" -n >nul
  echo UNCOMMENT THIS
  rem if %errorlevel% gtr 0 goto LocalMods

echo Generating output directory...
  md "%svnroot%\distro" 2>nul
  set outdir=%svnroot%\distro\%serial%
  md "%outdir%" 2>nul

echo Setting up version info...
  echo NOT DONE

echo Building...
  devenv "%svnroot%\source\client\screenie-2k3.vcproj" /rebuild release /out "%outdir%\buildlog.txt"
  if %errorlevel% gtr 0 goto BuildError

echo Copying program binaries...
  copy "%svnroot%\bin-release\screenie.exe" "%outdir%" >nul

echo Watermarking...
  cd /d "%outdir%"
  "%svnroot%\tools\PEWaterMark.exe" "%outdir%\screenie.exe" /mark=%serial%

echo Building Installer...
  makensis.exe "%svnroot%\source\installer\screenie.nsi" /O"%outdir%\nsislog.txt" /Dregistrant="%registrant%" /Dserial="%serial%"

echo Copying final installer...
  copy "%svnroot%\bin-installer\ScreenieSetup.exe" "%outdir%" >nul

echo All done!  SUCCESS.
  cd /d %svnroot%
  goto End



:Usage
echo * Usage:
echo * official_build.cmd [serial] [registrant]
echo * Also, You must run this from the svn.screenie\root directory (the directory that contains this file)
goto End

:DevenvNotFound
echo !!Devenv.exe was not found in your path.  Stopping.
goto End

:NsisNotFound
echo !!makensis.exe was not found in your path.  Make sure you have nsis installed, and it's location is in your path environment variable. Stopping.
goto End

:BuildError
echo !!Build errors were found.  Stopping.
goto End

:LocalMods
echo There are local modifications; you can't do an official build.

:End
echo.
pause