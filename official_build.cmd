@echo off

echo Confirming arguments / variables...
  if "%1" == "" goto Usage
  if "%2" == "" goto Usage

  set svnroot=%cd%
  set serial=%1
  set registrant=%2

echo Checking for tools / files...
  where makensis.exe >nul
  if %errorlevel% gtr 0 goto NsisNotFound
  if not exist "%svnroot%\bin-release\screenie.exe" goto NotBuilt

echo Generating output directory...
  md "%svnroot%\distro" 2>nul
  set outdir=%svnroot%\distro\%serial%
  md "%outdir%" 2>nul

echo Copying program binaries...
  copy "%svnroot%\bin-release\screenie.exe" "%outdir%" >nul

echo Generating version information for "%svnroot%\source\client"
  "%svnroot%\tools\SubWCRev.exe" "%svnroot%\source\client" "%svnroot%\distro\ver_in.xml" "%outdir%\ver_out.xml" -n
  if %errorlevel% gtr 0 goto LocalMods

echo Setting up version info...
  "%svnroot%\tools\veredit.exe" "%outdir%\screenie.exe" /xml "%outdir%\ver_out.xml" /string RegisteredTo="%registrant%"
  if %errorlevel% gtr 0 goto VereditError

echo Watermarking...
  rem cd /d "%outdir%"
  rem "%svnroot%\tools\PEWaterMark.exe" "%outdir%\screenie.exe" /mark=%serial%

echo Building Installer...
  rem makensis.exe "%svnroot%\source\installer\screenie.nsi" /O"%outdir%\nsislog.txt" /Dregistrant="%registrant%" /Dserial="%serial%"

echo Copying final installer...
  rem copy "%svnroot%\bin-installer\ScreenieSetup.exe" "%outdir%" >nul

echo All done!  SUCCESS.
  cd /d %svnroot%
  goto End



:Usage
echo * Usage:
echo * official_build.cmd [serial] [registrant]
echo * Also, You must run this from the svn.screenie\root directory (the directory that contains this file)
goto End

:NsisNotFound
echo !!makensis.exe was not found in your path.  Make sure you have nsis installed, and it's location is in your path environment variable. Stopping.
goto End

:VereditError
echo !!veredit returned an error.  Stopping.
goto End

:NotBuilt
echo !!screenie.exe has not been built.  Stopping.
goto End

:LocalMods
echo There are local modifications; you can't do an official build.

:End
echo.
pause