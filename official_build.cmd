@echo off

echo * Usage:
echo * official_build.cmd [serial] [registrant] {binary-dir} {installer-name}
echo * Also, You must run this from the svn.screenie\root directory (the directory that contains this file)
echo * {binary-dir} is relative to root\ directory.  if it's omitted,
echo * then bin-release will be used.
echo * {installer-name} may also be omitted... "Screenie" is the default.
echo.  

echo Confirming arguments / variables...
  set svnroot=%cd%
  set serial=%1
  set registrant=%2

  if "%1" == "" set /P serial=Enter the serial number: 
  if "%2" == "" set /P registrant=Enter the registrant: 

  set bindir=%3
  if "%3" == "" set bindir=bin-release
  if "%3" == "" set /P bindir=Enter the binaries directory, or leave it blank to use bin-release:

  set installname=%4
  if "%4" == "" set installname=Screenie
  if "%4" == "" set /P installname=Enter the installer name, or leave it blank to use "Screenie":
  
  echo   svnroot     =%svnroot%
  echo   serial      =%serial%
  echo   registrant  =%registrant%
  echo   bindir      =%bindir%
  echo   installname =%installname%
  echo.

echo Checking for tools / files...
  where makensis.exe >nul
  if %errorlevel% gtr 0 goto NsisNotFound
  if not exist "%svnroot%\%bindir%\screenie.exe" goto NotBuilt

echo Generating output directory...
  md "%svnroot%\distro" 2>nul
  set outdir=%svnroot%\distro\%serial%
  md "%outdir%" 2>nul

echo Copying program binaries...
  copy "%svnroot%\%bindir%\screenie.exe" "%outdir%" >nul

echo Generating version information for "%svnroot%\source\client"
  "%svnroot%\tools\SubWCRev.exe" "%svnroot%\source\client" "%svnroot%\distro\ver_in.xml" "%outdir%\ver_out.xml" -n
  if %errorlevel% gtr 0 goto LocalMods
  
  "%svnroot%\tools\textreplace.exe" "%outdir%\ver_out.xml" "[serial]"="%serial%" "[registrant]"="%registrant%"
  if %errorlevel% gtr 0 goto TextReplaceFailed
  
echo Setting up version info...
  "%svnroot%\tools\veredit.exe" "%outdir%\screenie.exe" /xml "%outdir%\ver_out.xml"
  if %errorlevel% gtr 0 goto VereditError

echo Watermarking screenie.exe...
  cd /d "%outdir%"
  "%svnroot%\tools\PEWaterMark.exe" "%outdir%\screenie.exe" /mark=%serial% >nul

echo Building Installer...
  rem /O"%outdir%\nsislog.txt" 
  makensis.exe /Dinstallname="%installname%" /Dregistrant="%registrant%" /Dserial="%serial%" /Dinfile="%svnroot%\%bindir%\screenie.exe" /Doutfile="%outdir%\ScreenieSetup.exe" "%svnroot%\source\installer\screenie.nsi"
  if %errorlevel% gtr 0 goto NSISError
  
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

:VereditError
echo !!veredit returned an error.  Stopping.
goto End

:NotBuilt
echo !!screenie.exe has not been built.  Stopping.
goto End

:TextReplaceFailed
echo !!textreplace.exe failed.  Stopping.
goto End

:LocalMods
echo There are local modifications; you can't do an official build.

:End
echo.
pause
