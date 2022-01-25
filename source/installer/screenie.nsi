; This will run from the distro\out directory, so make the paths relative to there.

!ifndef outfile
!define outfile "..\..\bin-release\ScreenieSetup.exe"
!endif

!ifndef infile
!define infile "..\..\bin-release\screenie.exe"
!endif

!ifndef installname
!define installname "Screenie"
!endif

!include "MUI.nsh"

SetCompressor lzma
XPStyle on

; The name of the installer
Name "${installname}"
InstallDir "$PROGRAMFILES\Screenie"
OutFile "${outfile}"

VIProductVersion 1.$WCLOGCOUNT$.$WCMODS?1:0$.0
VIAddVersionKey "ProductName" "${installname}"
VIAddVersionKey "CompanyName" "Carl Corcoran & Roger Clark"
VIAddVersionKey "LegalCopyright" "©2004-2022 Carl Corcoran & Roger Clark.  All Rights Reserved."
VIAddVersionKey "FileDescription" "Screenie Installer.  https://screenie.net. Built on $WCNOW$, commit $WCREV$, branch $WCBRANCH$"
VIAddVersionKey "FileVersion" 0.$WCLOGCOUNT$.$WCMODS?1:0$.0
VIAddVersionKey "FileProduct" 0.$WCLOGCOUNT$.$WCMODS?1:0$.0

;Interface Settings
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\orange.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\orange.bmp"
!define MUI_HEADERIMAGE_UNBITMAP "${NSISDIR}\Contrib\Graphics\Header\orange-uninstall.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\orange-uninstall.bmp"
!define MUI_COMPONENTSPAGE_CHECKBITMAP "${NSISDIR}\Contrib\Graphics\Checks\simple-round2.bmp"
!define MUI_LICENSEPAGE_RADIOBUTTONS
!define MUI_ABORTWARNING
!define MUI_FINISHPAGE_RUN "$INSTDIR\Screenie.exe"
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\orange-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\orange-uninstall.ico"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\License.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH
 
!insertmacro MUI_LANGUAGE "English"

; ===============================
; SECTIONS
; ===============================

; The stuff to install
Section "Program Files (required)" program_files
  ; Make the details view always visible
  SetDetailsPrint both
  SetDetailsView show

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

  SetOutPath "$INSTDIR"

  ; close it just in case it's open.
  FindWindow $0 "ScreenieMainWnd"
  SendMessage $0 16 0 0
  
  Delete "$INSTDIR\screenie.exe"
  File ${infile}
  File "..\gdiplus.dll"

  WriteUninstaller $INSTDIR\Uninst.exe
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "Screenie" "$INSTDIR\screenie.exe"
  
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Screenie" "DisplayName" "Screenie"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Screenie" "UninstallString" "$INSTDIR\Uninst.exe"
SectionEnd

; optional section
Section "Start Menu Shortcuts" sms
  CreateDirectory "$SMPROGRAMS\Screenie"
  CreateShortCut "$SMPROGRAMS\Screenie\Uninstall.lnk" "$INSTDIR\Uninst.exe" "" "$INSTDIR\Uninst.exe" 0
  CreateShortCut "$SMPROGRAMS\Screenie\Screenie.lnk" "$INSTDIR\screenie.exe" "" "$INSTDIR\screenie.exe" 0
SectionEnd

;--------------------------------
;Uninstaller Section

Section "Uninstall"
  FindWindow $0 "ScreenieMainWnd"
  SendMessage $0 16 0 0

  Delete $INSTDIR\Uninst.exe
  Delete $INSTDIR\screenie.exe
  RMDir $SMPROGRAMS\Screenie
  RMDir $INSTDIR
  DeleteRegKey HKCU SOFTWARE\Screenie2
  DeleteRegValue HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "Screenie"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Screenie"
SectionEnd

LangString Desc_Program_Files ${LANG_ENGLISH} "Screenie program files.  This component is required."
LangString Desc_SMS ${LANG_ENGLISH} "Places shortcut(s) to installed program files in your Windows Start menu."

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${program_files} $(Desc_Program_Files)
!insertmacro MUI_DESCRIPTION_TEXT ${sms} $(Desc_SMS)
!insertmacro MUI_FUNCTION_DESCRIPTION_END