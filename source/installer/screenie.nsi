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

; The name of the installer
Name "Screenie"
InstallDir "$PROGRAMFILES\Screenie"
OutFile ${outfile}


Page directory

Page components

Page instfiles

VIProductVersion 0.1.0.${serial}
VIAddVersionKey "ProductName" "Screenie Installer"
VIAddVersionKey "CompanyName" "Carl Corcoran & Roger Clark"
VIAddVersionKey "LegalCopyright" "©2005 Carl Corcoran & Roger Clark.  All Rights Reserved."
VIAddVersionKey "FileDescription" "Screenie Installer.  http://screenie.net"
VIAddVersionKey "FileVersion" 0.1.0.${serial}
VIAddVersionKey "FileProduct" 0.1.0.${serial}
VIAddVersionKey "RegisteredTo" ${registrant}

; The stuff to install
Section "Screenie Program Files (required)"
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
  File ${infile}
  WriteUninstaller $INSTDIR\Uninst.exe
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "Screenie" $INSTDIR\screenie.exe
  
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Screenie" "DisplayName" "Screenie"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Screenie" "UninstallString" "$INSTDIR\Uninst.exe"
SectionEnd

; optional section
Section "Start Menu Shortcuts"
  CreateDirectory "$SMPROGRAMS\Screenie"
  CreateShortCut "$SMPROGRAMS\Screenie\Uninstall.lnk" "$INSTDIR\Uninst.exe" "" "$INSTDIR\Uninst.exe" 0
  CreateShortCut "$SMPROGRAMS\Screenie\Screenie.lnk" "$INSTDIR\screenie.exe" "" "$INSTDIR\screenie.exe" 0
SectionEnd


Section "Uninstall"
  Delete $INSTDIR\Uninst.exe
  Delete $INSTDIR\screenie.exe
  RMDir $SMPROGRAMS\Screenie
  RMDir $INSTDIR
  DeleteRegKey HKCU SOFTWARE\Screenie2
  DeleteRegValue HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "Screenie"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Screenie"
SectionEnd
