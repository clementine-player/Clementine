!define PRODUCT_NAME "Clementine"
!define PRODUCT_PUBLISHER "Clementine"
!define PRODUCT_VERSION_MAJOR 0
!define PRODUCT_VERSION_MINOR 2
!define PRODUCT_DISPLAY_VERSION "0.3-beta2"
!define PRODUCT_WEB_SITE "http://code.google.com/p/clementine-player/"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define PRODUCT_INSTALL_DIR "$PROGRAMFILES\Clementine"

SetCompressor /SOLID lzma
!include "MUI2.nsh"

!define MUI_ICON "clementine.ico"

!define MUI_COMPONENTSPAGE_SMALLDESC
!define MUI_FINISHPAGE_RUN $INSTDIR/clementine.exe

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

  File "clementine.exe"
  File "clementine.ico"
  File "iconv.dll"
  File "lastfm.lib"
  File "libFLAC-8.dll"
  File "libgcrypt-11.dll"
  File "libgcc_s_dw2-1.dll"
  File "libgcc_s_sjlj-1.dll"
  File "libgio-2.0-0.dll"
  File "libglib-2.0-0.dll"
  File "libgmodule-2.0-0.dll"
  File "libgnutls-26.dll"
  File "libgobject-2.0-0.dll"
  File "libgpg-error-0.dll"
  File "libgstaudio-0.10.dll"
  File "libgstbase-0.10.dll"
  File "libgstcontroller-0.10.dll"
  File "libgstdshow-0.10.dll"
  File "libgstinterfaces-0.10.dll"
  File "libgstnet-0.10.dll"
  File "libgstpbutils-0.10.dll"
  File "libgstreamer-0.10.dll"
  File "libgstriff-0.10.dll"
  File "libgstrtp-0.10.dll"
  File "libgstrtsp-0.10.dll"
  File "libgstsdp-0.10.dll"
  File "libgsttag-0.10.dll"
  File "libgthread-2.0-0.dll"
  File "libmp3lame-0.dll"
  File "libogg-0.dll"
  File "liboil-0.3-0.dll"
  File "libopenjpeg-2.dll"
  File "libschroedinger-1.0-0.dll"
  File "libsoup-2.4-1.dll"
  File "libspeex-1.dll"
  File "libtag.dll"
  File "libtheoradec-1.dll"
  File "libtheoraenc-1.dll"
  File "libvorbis-0.dll"
  File "libvorbisenc-2.dll"
  File "libxml2-2.dll"
  File "libxml2.dll"
  File "mingwm10.dll"
  File "QtCore4.dll"
  File "QtGui4.dll"
  File "QtNetwork4.dll"
  File "QtSql4.dll"
  File "QtXml4.dll"
  File "xvidcore.dll"
  File "zlib1.dll"
SectionEnd

Section "Gstreamer plugins" gstreamer-plugins
  SetOutPath "$INSTDIR\gstreamer-plugins"
  SetOverwrite ifnewer
  
  File "/oname=libgstasfdemux.dll" "gstreamer-plugins\libgstasfdemux.dll"
  File "/oname=libgstaudioconvert.dll" "gstreamer-plugins\libgstaudioconvert.dll"
  File "/oname=libgstaudioresample.dll" "gstreamer-plugins\libgstaudioresample.dll"
  File "/oname=libgstautodetect.dll" "gstreamer-plugins\libgstautodetect.dll"
  File "/oname=libgstcoreelements.dll" "gstreamer-plugins\libgstcoreelements.dll"
  File "/oname=libgstdecodebin.dll" "gstreamer-plugins\libgstdecodebin.dll"
  File "/oname=libgstdirectsound.dll" "gstreamer-plugins\libgstdirectsound.dll"
  File "/oname=libgstffmpeggpl.dll" "gstreamer-plugins\libgstffmpeggpl.dll"
  File "/oname=libgstflac.dll" "gstreamer-plugins\libgstflac.dll"
  File "/oname=libgstgio.dll" "gstreamer-plugins\libgstgio.dll"
  File "/oname=libgsticydemux.dll" "gstreamer-plugins\libgsticydemux.dll"
  File "/oname=libgstid3demux.dll" "gstreamer-plugins\libgstid3demux.dll"
  File "/oname=libgstogg.dll" "gstreamer-plugins\libgstogg.dll"
  File "/oname=libgstsoup.dll" "gstreamer-plugins\libgstsoup.dll"
  File "/oname=libgstspeex.dll" "gstreamer-plugins\libgstspeex.dll"
  File "/oname=libgsttypefindfunctions.dll" "gstreamer-plugins\libgsttypefindfunctions.dll"
  File "/oname=libgstvolume.dll" "gstreamer-plugins\libgstvolume.dll"
  File "/oname=libgstvorbis.dll" "gstreamer-plugins\libgstvorbis.dll"
  File "/oname=libgstwavparse.dll" "gstreamer-plugins\libgstwavparse.dll"
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
  Delete "$INSTDIR\iconv.dll"
  Delete "$INSTDIR\lastfm.lib"
  Delete "$INSTDIR\libFLAC-8.dll"
  Delete "$INSTDIR\libgcrypt-11.dll"
  Delete "$INSTDIR\libgcc_s_dw2-1.dll"
  Delete "$INSTDIR\libgcc_s_sjlj-1.dll"
  Delete "$INSTDIR\libgio-2.0-0.dll"
  Delete "$INSTDIR\libglib-2.0-0.dll"
  Delete "$INSTDIR\libgmodule-2.0-0.dll"
  Delete "$INSTDIR\libgnutls-26.dll"
  Delete "$INSTDIR\libgobject-2.0-0.dll"
  Delete "$INSTDIR\libgpg-error-0.dll"
  Delete "$INSTDIR\libgstaudio-0.10.dll"
  Delete "$INSTDIR\libgstbase-0.10.dll"
  Delete "$INSTDIR\libgstcontroller-0.10.dll"
  Delete "$INSTDIR\libgstdshow-0.10.dll"
  Delete "$INSTDIR\libgstinterfaces-0.10.dll"
  Delete "$INSTDIR\libgstnet-0.10.dll"
  Delete "$INSTDIR\libgstpbutils-0.10.dll"
  Delete "$INSTDIR\libgstreamer-0.10.dll"
  Delete "$INSTDIR\libgstriff-0.10.dll"
  Delete "$INSTDIR\libgstrtp-0.10.dll"
  Delete "$INSTDIR\libgstrtsp-0.10.dll"
  Delete "$INSTDIR\libgstsdp-0.10.dll"
  Delete "$INSTDIR\libgsttag-0.10.dll"
  Delete "$INSTDIR\libgthread-2.0-0.dll"
  Delete "$INSTDIR\libmp3lame-0.dll"
  Delete "$INSTDIR\libogg-0.dll"
  Delete "$INSTDIR\liboil-0.3-0.dll"
  Delete "$INSTDIR\libopenjpeg-2.dll"
  Delete "$INSTDIR\libschroedinger-1.0-0.dll"
  Delete "$INSTDIR\libsoup-2.4-1.dll"
  Delete "$INSTDIR\libspeex-1.dll"
  Delete "$INSTDIR\libtag.dll"
  Delete "$INSTDIR\libtheoradec-1.dll"
  Delete "$INSTDIR\libtheoraenc-1.dll"
  Delete "$INSTDIR\libvorbis-0.dll"
  Delete "$INSTDIR\libvorbisenc-2.dll"
  Delete "$INSTDIR\libxml2-2.dll"
  Delete "$INSTDIR\libxml2.dll"
  Delete "$INSTDIR\mingwm10.dll"
  Delete "$INSTDIR\QtCore4.dll"
  Delete "$INSTDIR\QtGui4.dll"
  Delete "$INSTDIR\QtNetwork4.dll"
  Delete "$INSTDIR\QtSql4.dll"
  Delete "$INSTDIR\QtXml4.dll"
  Delete "$INSTDIR\xvidcore.dll"
  Delete "$INSTDIR\zlib1.dll"

  Delete "$INSTDIR\gstreamer-plugins\libgstasfdemux.dll"
  Delete "$INSTDIR\gstreamer-plugins\libgstaudioconvert.dll"
  Delete "$INSTDIR\gstreamer-plugins\libgstaudioresample.dll"
  Delete "$INSTDIR\gstreamer-plugins\libgstautodetect.dll"
  Delete "$INSTDIR\gstreamer-plugins\libgstcoreelements.dll"
  Delete "$INSTDIR\gstreamer-plugins\libgstdecodebin.dll"
  Delete "$INSTDIR\gstreamer-plugins\libgstdirectsound.dll"
  Delete "$INSTDIR\gstreamer-plugins\libgstffmpeggpl.dll"
  Delete "$INSTDIR\gstreamer-plugins\libgstflac.dll"
  Delete "$INSTDIR\gstreamer-plugins\libgstgio.dll"
  Delete "$INSTDIR\gstreamer-plugins\libgsticydemux.dll"
  Delete "$INSTDIR\gstreamer-plugins\libgstid3demux.dll"
  Delete "$INSTDIR\gstreamer-plugins\libgstogg.dll"
  Delete "$INSTDIR\gstreamer-plugins\libgstspeex.dll"
  Delete "$INSTDIR\gstreamer-plugins\libgstsoup.dll"
  Delete "$INSTDIR\gstreamer-plugins\libgsttypefindfunctions.dll"
  Delete "$INSTDIR\gstreamer-plugins\libgstvolume.dll"
  Delete "$INSTDIR\gstreamer-plugins\libgstvorbis.dll"
  Delete "$INSTDIR\gstreamer-plugins\libgstwavparse.dll"
  
  ; Remove the installation folder.
  RMDir "$INSTDIR\gstreamer-plugins"
  RMDir "$INSTDIR"

  ; Remove the Shortcuts
  Delete "$STARTMENU\Programs\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk"
  Delete "$STARTMENU\Programs\${PRODUCT_NAME}\Uninstall.lnk"
  RMDir /r "$STARTMENU\Programs\${PRODUCT_NAME}"
  
  ; Remove the entry from 'installed programs list'
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
SectionEnd
