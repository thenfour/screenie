!ifndef outfile
!define outfile "..\..\bin-release\ScreenieSetup.exe"
!endif

!ifndef infile
!define infile "..\..\bin-release\screenie.exe"
!endif

!ifndef registrant
!define registrant "[development]"
!endif

!ifndef serial
!define serial "0"
!endif

!ifndef installname
!define installname "Screenie"
!endif

; The name of the installer
Name "${installname}"
InstallDir "$PROGRAMFILES\Screenie"
OutFile "${outfile}"

; Adds an XP manifest to the installer
XPStyle on

Var "Launch"

Page directory

Page components

Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

VIProductVersion 1.$WCREV$.$WCMODS?1:0$.${serial}
VIAddVersionKey "ProductName" "${installname}"
VIAddVersionKey "CompanyName" "Carl Corcoran & Roger Clark"
VIAddVersionKey "LegalCopyright" "©2005 Carl Corcoran & Roger Clark.  All Rights Reserved."
VIAddVersionKey "FileDescription" "Screenie Installer.  http://screenie.net"
VIAddVersionKey "FileVersion" 0.$WCREV$.$WCMODS?1:0$.${serial}
VIAddVersionKey "FileProduct" 0.$WCREV$.$WCMODS?1:0$.${serial}
VIAddVersionKey "RegisteredTo" "${registrant}"

; The stuff to install
Section "Screenie Program Files (required)"

  ;-------------------------
  ; Check for already-installed
  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Screenie" "DisplayName"
  StrCmp $0 "" NotAlreadyInstalled 0
  MessageBox MB_YESNO "Setup has detected that you already have a version of ${installname} installed.  It is recommended that you uninstall the previous version before continuing, or setup may not work.  Do you want to continue anyway?" IDYES true IDNO false
false:
  Abort "You have chosen to stop installation."
true:

NotAlreadyInstalled:

  ;-------------------------
  ; check the windows version.
  ReadRegStr $0 HKLM "SOFTWARE\Microsoft\Windows NT\CurrentVersion" CurrentVersion
  StrCmp $0 "" lbl_VersionError 0
  
  ; We're in NT, but make sure we're 5.0 or later.
  StrCpy $9 $0 1 ; Copy the first digit of the version - this is the major version.
  IntCmp $9 5 0 lbl_VersionError 0
  Goto lbl_VersionOK
  
lbl_VersionError:
  ; ERROR!
  MessageBox MB_OK|MB_ICONSTOP "This application requires Windows 2000 or later."
  Abort "This application requires Windows 2000 or later."

lbl_VersionOK:
  
  SetOutPath $INSTDIR
  Delete ${infile}
  File ${infile}
  WriteUninstaller $INSTDIR\Uninst.exe
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "Screenie" "$INSTDIR\screenie.exe"
  
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Screenie" "DisplayName" "Screenie"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Screenie" "UninstallString" "$INSTDIR\Uninst.exe"
SectionEnd

; optional section
Section "Start Menu Shortcuts"
  CreateDirectory "$SMPROGRAMS\Screenie"
  CreateShortCut "$SMPROGRAMS\Screenie\Uninstall.lnk" "$INSTDIR\Uninst.exe" "" "$INSTDIR\Uninst.exe" 0
  CreateShortCut "$SMPROGRAMS\Screenie\Screenie.lnk" "$INSTDIR\screenie.exe" "" "$INSTDIR\screenie.exe" 0
SectionEnd

; optional section
Section "Launch Screenie"
  StrCpy "$Launch" "Yes"
SectionEnd

Function .onInstSuccess
  StrCmp "$Launch" "Yes" 0 +2
  Exec "$INSTDIR\screenie.exe"
FunctionEnd

Section "Uninstall"
  Delete $INSTDIR\Uninst.exe
  Delete $INSTDIR\screenie.exe
  RMDir $SMPROGRAMS\Screenie
  RMDir $INSTDIR
  DeleteRegKey HKCU SOFTWARE\Screenie2
  DeleteRegValue HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "Screenie"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Screenie"
SectionEnd
