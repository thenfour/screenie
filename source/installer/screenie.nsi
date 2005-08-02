

; The name of the installer
Name "Screenie"


Page instfiles


VIProductVersion "0.1.0.4"
VIAddVersionKey "ProductName" "Screenie Installer"
VIAddVersionKey "CompanyName" "Carl Corcoran & Roger Clark"
VIAddVersionKey "LegalCopyright" "©2005 Carl Corcoran & Roger Clark.  All Rights Reserved."
VIAddVersionKey "FileDescription" "Screenie Installer.  http://screenie.net"
VIAddVersionKey "FileVersion" "0.1.0.4"
VIAddVersionKey "Registrant" "[development]"


; The file to write
OutFile "..\..\bin-installer\ScreenieSetup.exe"


; The stuff to install
Section "Screenie Program Files (required)"

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR

SectionEnd

