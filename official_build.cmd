@echo off

echo Confirming arguments / variables...

  if "%1" == "" goto Usage
  if "%2" == "" goto Usage

  set svnroot=%cd%
  set serial=%1
  set registrant=%2

echo Checking for devenv...
  where devenv.exe >nul
  if %errorlevel% gtr 0 goto DevenvNotFound

echo Checking for local changes...
  md %svnroot%\temp 2>nul
  rem Create a temp file (SubWCRev needs to process files, but we don't need them now... so just make throwaways)
  echo. >%svnroot%\temp\tempver.in
  "%svnroot%\tools\SubWCRev.exe" "%svnroot%" "%svnroot%\temp\tempver.out" "%svnroot%\temp\tempver.in" -n >nil
  if %errorlevel% gtr 0 goto LocalMods

echo Generating output directory...
  md %svnroot%\distro &>nul
  set outdir=%svnroot%\distro\%serial%
  md %outdir% &>nul

echo Setting up version info...

echo Building...
  devenv

echo Watermarking...
  cd /d %outputdir%
  %svnroot%\tools\PEWaterMark.exe test.exe /mark=%serial%

echo Building Installer...
  rem.

echo All done!  SUCCESS.
  goto End



:Usage
echo * Usage:
echo * official_build.cmd [serial] [registrant]
echo * You must run this from the svn.screenie\root directory (the directory that contains this file)
goto End

:DevenvNotFound
echo !!Devenv.exe was not found in your path.  Stopping.
goto End

:LocalMods
echo There are local modifications; you can't do an official build.

:End
echo.
pause