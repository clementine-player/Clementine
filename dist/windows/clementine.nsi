!define PRODUCT_NAME "Clementine"
!define PRODUCT_PUBLISHER "Clementine"
!define PRODUCT_VERSION_MAJOR 0
!define PRODUCT_VERSION_MINOR 2
!define PRODUCT_DISPLAY_VERSION "0.2"
!define PRODUCT_WEB_SITE "http://code.google.com/p/clementine-player/"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define PRODUCT_INSTALL_DIR "$PROGRAMFILES\Clementine"

SetCompressor /SOLID lzma
!include "MUI2.nsh"

!define MUI_ICON "clementine.ico"

!define MUI_COMPONENTSPAGE_SMALLDESC
!define MUI_FINISHPAGE_RUN clementine.exe

; Installer pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES  
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

Name "${PRODUCT_NAME}"
OutFile "${PRODUCT_NAME}Setup-${PRODUCT_DISPLAY_VERSION}.exe"
InstallDir "${PRODUCT_INSTALL_DIR}"
ShowInstDetails show
ShowUnInstDetails show
; BrandingText "${PRODUCT_PUBLISHER}"
RequestExecutionLevel admin

Section "Clementine" Clementine
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer

  File "/oname=clementine.exe" "clementine.exe"
  File "/oname=clementine.ico" "clementine.ico"
  File "/oname=lastfm.lib" "lastfm.lib"
  File "/oname=libgcc_s_dw2-1.dll" "libgcc_s_dw2-1.dll"
  File "/oname=libgcc_s_sjlj-1.dll" "libgcc_s_sjlj-1.dll"
  File "/oname=libtag.dll" "libtag.dll"
  File "/oname=mingwm10.dll" "mingwm10.dll"
  File "/oname=phonon4.dll" "phonon4.dll"
  File "/oname=QtCore4.dll" "QtCore4.dll"
  File "/oname=QtGui4.dll" "QtGui4.dll"
  File "/oname=QtNetwork4.dll" "QtNetwork4.dll"
  File "/oname=QtSql4.dll" "QtSql4.dll"
  File "/oname=QtXml4.dll" "QtXml4.dll"
  File "/oname=zlib1.dll" "zlib1.dll"
SectionEnd

Section "Phonon Backends" phonon_backend
  SetOutPath "$INSTDIR\phonon_backend"
  SetOverwrite ifnewer
  
  File "/oname=phonon_ds94.dll" "phonon_backend\phonon_ds94.dll"
SectionEnd

Section "Start menu items" startmenu
  ; Create Start Menu folders and shortcuts.
  CreateDirectory "$STARTMENU\Programs\${PRODUCT_NAME}"
  CreateShortCut "$STARTMENU\Programs\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk" "$INSTDIR\clementine.exe"
  CreateShortCut "$STARTMENU\Programs\${PRODUCT_NAME}\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
SectionEnd

Section "Uninstaller"
  ; Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_NAME}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\Uninstall.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\clementine.ico"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_DISPLAY_VERSION}"
  WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "VersionMajor" "${PRODUCT_VERSION_MAJOR}"
  WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "VersionMinor" "${PRODUCT_VERSION_MINOR}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd

Section "Uninstall"
  ; Delete all the files
  Delete "$INSTDIR\clementine.ico"
  Delete "$INSTDIR\clementine.exe"
  Delete "$INSTDIR\lastfm.lib"
  Delete "$INSTDIR\libgcc_s_dw2-1.dll"
  Delete "$INSTDIR\libgcc_s_sjlj-1.dll"
  Delete "$INSTDIR\libtag.dll"
  Delete "$INSTDIR\mingwm10.dll"
  Delete "$INSTDIR\phonon4.dll"
  Delete "$INSTDIR\QtCore4.dll"
  Delete "$INSTDIR\QtGui4.dll"
  Delete "$INSTDIR\QtNetwork4.dll"
  Delete "$INSTDIR\QtSql4.dll"
  Delete "$INSTDIR\QtXml4.dll"
  Delete "$INSTDIR\zlib1.dll"
  Delete "$INSTDIR\phonon_backend\phonon_ds94.dll"
  
  ; Remove the installation folder.
  RMDir "$INSTDIR\phonon_backend"
  RMDir "$INSTDIR"

  ; Remove the Shortcuts
  Delete "$STARTMENU\Programs\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk"
  Delete "$STARTMENU\Programs\${PRODUCT_NAME}\Uninstall.lnk"
  RMDir /r "$STARTMENU\Programs\${PRODUCT_NAME}"
  
  ; Remove the entry from 'installed programs list'
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
SectionEnd