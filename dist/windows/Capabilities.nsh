/*
_____________________________________________________________________________

                 Application Capabilities (Default Programs)
_____________________________________________________________________________

 By Joel Spadin
 Loosely based on code taken from http://nsis.sourceforge.net/File_Association

 These functions register an application with Windows Vista's and Windows 7's 
 Default Programs window.
 
 Usage:
 
 !include "Capabilities.nsh"
 !define CAPABILITIES_NAME "[program name]"
 !define CAPABILITIES_DESCRIPTION "[description]"
 !define CAPABILITIES_PROGID "[progid]"
 !define CAPABILITIES_PATH "[path]"
 ...
 
 During install, call ${RegisterCapabilities}, then use the other register
 macros to create file type, MIME, and protocol associations.
 
 During uninstall, call ${UnRegisterCapabilities} 
 
_______________________________________________________________________________
 
 More information about Default Programs and Client Types can be found here:
 http://msdn.microsoft.com/en-us/library/cc144154(VS.85).aspx
 http://msdn.microsoft.com/en-us/library/cc144109(v=VS.85).aspx
 
 Defines:  All defines not marked [optional] are required.
 
 CAPABILITIES_NAME 
	The canonical name of the program.
 
 CAPABILITIES_LOCAL_NAME 	[optional]
	The localized name of the program as it appears in Default Programs.  
	This should be in the format "@FilePath,-StringID" where FilePath is the 
	path to a .dll or .exe file and StringID is the ID of a resource contained 
	in the file.
 
 CAPABILITIES_DESCRIPTION
	The localized description shown in Default Programs.  This can be either a
	string or in the same format as CAPABILITIES_LOCAL_NAME
 
 CAPABILITIES_PROGID 
	An identifier used in file associations.  Usually, this is the name of the 
	program.  This should not have any spaces in it.
	
 CAPABILITIES_PATH
	The location where capabilities info is stored in the registry.  
	The "Capabilities" key will automatically be added.  If the application is
	a client of some sort, (browser, email, media player, etc.) use
	"Software\Clients\[ClientType]\[ProgramName]".  Otherwise, use
	"Software\[CompanyName]\[ProgramName]" or just "Software\[ProgramName]".
 
 CAPABILITIES_ICON		[optional]
	The icon shown in Default Programs.  This should be in the format 
	"FilePath,IconIndex" where FilePath is the path to a file containing the 
	icon.  If IconIndex is positive, the number is used as the index of the 
	zero-based array of icons stored in the file.  If IconIndex is negative, 
	the absolute value is used as a resource ID.
	
 CAPABILITIES_REINSTALL		[optional]
	The command executed when a user uses Set Program Access and Computer 
	Defaults to select this application as the default for its client type.  
	This command should launch a program that associates the application with 
	all the file and protocol types it can handle.
	
 CAPABILITIES_HIDE_ICONS	[optional]
	The command executed when a user uses Set Program Access and Computer
	Defaults to disable access to this application.  This command should launch 
	a program that hides all shortcuts and access points to this application.  
	It should also prevent the application from being run automatically and 
	update the IconsVisible registry value to 0.
	
 CAPABILITIES_SHOW_ICONS	[optional]
	The command executed when a user uses Set Program Access and Computer
	Defaults to enable access to this application.  This command should launch
	a program that restores shortcuts and access points to the application,
	allows the program to run, and updates the IconsVisible registry value to 1.
 
 
Macros:
	
 ${RegisterCapabilities}
	Registers the program with Default Programs.  Call this once on install
	before using any other functions.
	
${UnRegisterCapabilities}
	Un-registers the program and all its associations.  Call this once on 
	uninstall to clean up all installed registry values.
	
${RegisterFileType} "[extension]" "[executable]" "[icon]" "[description]"
	Registers a file type with the program
	extension:		The file extension to register (ex: .txt)
	executable:		The executable that opens the file type
	icon:			The icon shown for the file type
	description:	Description for the file type shown in Explorer
	
${RegisterMediaType} "[extension]" "[executable]" "[icon]" "[description]"
	Registers a media file type with the program (has a "Play" command)
	(arguments are same as RegisterFileType)
	
${RegisterMimeType} "[mime type]" "[shortname]" "[clsid]"
	Registers a mime type with the program
	mime type:		The MIME type to register (ex: audio/mp3)
	shortname:		A short identifier for the type (ex: mp3)
	clsid:			The CLSID of the program to handle files of this type
	
${RegisterProtocol}	"[protocol]" "[executable]" "[icon]" "[description]"
	Registers a URL protocol with the program
	protocol:		The protocol to register (ex: http)
	(other arguments are same as RegisterFileType)

${UnRegisterFileType} "[extension]"
	Un-registers a previously registered file type
	extension:		The file extension to un-register
	
${UnRegisterMimeType} "[mime type]"
	Un-registers a previously registered MEME type
	mime type:		The MIME type to un-register
	
${UnRegisterProtocol} "[protocol]"
	Un-registers a previously registered URL protocol
	protocol:		The URL protocol to un-register
	
*/




!ifndef Capabilities_INCLUDED
!define Capabilities_INCLUDED

!include "Util.nsh"
!include "StrFunc.nsh"

${StrCase}

!verbose push
!verbose 3
!ifndef _Capabilities_VERBOSE
	!define _Capabilities_VERBOSE 3
!endif
!verbose ${_Capabilities_VERBOSE}
!define Capabilities_VERBOSE `!insertmacro Capabilities_VERBOSE`
!verbose pop

!macro Capabilities_VERBOSE _VERBOSE
	!verbose push
	!verbose 3
	!undef _Capabilities_VERBOSE
	!define _Capabilities_VERBOSE ${_VERBOSE}
	!verbose pop
!macroend

!macro RegisterCapabilitiesCall
	!verbose push
	!verbose ${_Capabilities_VERBOSE}
	${CallArtificialFunction} RegisterCapabilities_
	!verbose pop
!macroend

!macro UnRegisterCapabilitiesCall
	!verbose push
	!verbose ${_Capabilities_VERBOSE}
	${CallArtificialFunction} UnRegisterCapabilities_
	!verbose pop
!macroend

!macro RegisterFileTypeCall _EXTENSION _EXECUTABLE _ICON _DESCRIPTION
	!verbose push
	!verbose ${_Capabilities_VERBOSE}
	Push `${_DESCRIPTION}`
	Push `${_ICON}`
	Push `${_EXECUTABLE}`
	Push `${_EXTENSION}`
	${CallArtificialFunction} RegisterFileType_
	!verbose pop
!macroend

!macro RegisterMediaTypeCall _EXTENSION _EXECUTABLE _ICON _DESCRIPTION
	!verbose push
	!verbose ${_Capabilities_VERBOSE}
	Push `${_DESCRIPTION}`
	Push `${_ICON}`
	Push `${_EXECUTABLE}`
	Push `${_EXTENSION}`
	${CallArtificialFunction} RegisterMediaType_
	!verbose pop
!macroend

!macro RegisterMimeTypeCall _MIMETYPE _SHORTNAME _CLSID
	!verbose push
	!verbose ${_Capabilities_VERBOSE}
	Push `${_CLSID}`
	Push `${_SHORTNAME}`
	Push `${_MIMETYPE}`
	${CallArtificialFunction} RegisterMimeType_
	!verbose pop
!macroend

!macro RegisterProtocolCall _PROTOCOL _EXECUTABLE _ICON _DESCRIPTION
	!verbose push
	!verbose ${_Capabilities_VERBOSE}
	Push `${_DESCRIPTION}`
	Push `${_ICON}`
	Push `${_EXECUTABLE}`
	Push `${_PROTOCOL}`
	${CallArtificialFunction} RegisterProtocol_
	!verbose pop
!macroend

!macro UnRegisterFileTypeCall _EXTENSION
	!verbose push
	!verbose ${_Capabilities_VERBOSE}
	Push `${_EXTENSION}`
	${CallArtificialFunction} UnRegisterFileType_
	!verbose pop
!macroend

!macro UnRegisterMimeTypeCall _MIMETYPE
	!verbose push
	!verbose ${_Capabilities_VERBOSE}
	Push `${_MIMETYPE}`
	${CallArtificialFunction} UnRegisterMimeType_
	!verbose pop
!macroend

!macro UnRegisterProtocolCall _PROTOCOL
	!verbose push
	!verbose ${_Capabilities_VERBOSE}
	Push `${_MIMETYPE}`
	${CallArtificialFunction} UnRegisterProtocol_
	!verbose pop
!macroend




!define RegisterCapabilities `!insertmacro RegisterCapabilitiesCall`
!define un.RegisterCapabilities `!insertmacro RegisterCapabilitiesCall`

!macro RegisterCapabilities
!macroend

!macro un.RegisterCapabilities
!macroend

!macro RegisterCapabilities_
	!verbose push
	!verbose ${_Capabilities_VERBOSE}
	Push $0
	
	!ifndef CAPABILITIES_PATH
		!error "CAPABILITIES_PATH not defined"
	!endif
	!ifndef CAPABILITIES_NAME
		!error "CAPABILITIES_NAME not defined"
	!endif
	!ifndef CAPABILITIES_PROGID
		!error "CAPABILITIES_PROGID not defined"
	!endif
	!ifndef CAPABILITIES_DESCRIPTION
		!error "CAPABILITIES_DESCRIPTION not defined"
	!endif
	
	StrCpy $0 "${CAPABILITIES_PATH}\Capabilities"
	; add the application to RegisteredApplications
	WriteRegStr HKLM "Software\RegisteredApplications" "${CAPABILITIES_NAME}" "$0"
	
	; write application info
	WriteRegStr HKLM "${CAPABILITIES_PATH}" "" "${CAPABILITIES_NAME}"
	
	!ifdef CAPABILITIES_LOCAL_NAME
		WriteRegStr HKLM "${CAPABILITIES_PATH}" "LocalizedString" "${CAPABILITIES_LOCAL_NAME}"
	!endif
	!ifdef CAPABILITIES_ICON
		WriteRegStr HKLM "${CAPABILITIES_PATH}\DefaultIcon" "" "${CAPABILITIES_ICON}"
	!endif
	
	; write installinfo if defined
	!ifdef CAPABILITIES_REINSTALL
		WriteRegStr HKLM "${CAPABILITIES_PATH}\InstallInfo" "ReinstallCommand" "${CAPABILITIES_REINSTALL}"
	!endif
	!ifdef CAPABILITIES_HIDE_ICONS
		WriteRegStr HKLM "${CAPABILITIES_PATH}\InstallInfo" "HideIconsCommand" "${CAPABILITIES_HIDE_ICONS}"
		WriteRegDWORD HKLM "${CAPABILITIES_PATH}\InstallInfo" "IconsVisible" 0x1
	!endif
	!ifdef CAPABILITIES_SHOW_ICONS
		WriteRegStr HKLM "${CAPABILITIES_PATH}\InstallInfo" "ShowIconsCommand" "${CAPABILITIES_SHOW_ICONS}"
		WriteRegDWORD HKLM "${CAPABILITIES_PATH}\InstallInfo" "IconsVisible" 0x1
	!endif
	
	; write application capabilities info
	!ifdef CAPABILITIES_LOCAL_NAME
		WriteRegStr HKLM "$0" "ApplicationName" "${CAPABILITIES_LOCAL_NAME}"
	!else
		WriteRegStr HKLM "$0" "ApplicationName" "${CAPABILITIES_NAME}"
	!endif
	WriteRegStr HKLM "$0" "ApplicationDescription" "${CAPABILITIES_DESCRIPTION}"
	
	Pop $0
	!verbose pop
!macroend



!define UnRegisterCapabilities `!insertmacro UnRegisterCapabilitiesCall`
!define un.UnRegisterCapabilities `!insertmacro UnRegisterCapabilitiesCall`

!macro UnRegisterCapabilities
!macroend

!macro un.UnRegisterCapabilities
!macroend

!macro UnRegisterCapabilities_
	!define MacroID ${__LINE__}
	!verbose push
	!verbose ${_Capabilities_VERBOSE}
	
	Push $0
	Push $1

	; remove all file associations
	FileTypeLoop_${MacroID}:
		EnumRegValue $0 HKLM "${CAPABILITIES_PATH}\Capabilities\FileAssociations" 0
		StrCmp $0 "" FileTypeDone_${MacroID}
		
		ReadRegStr $1 HKLM "${CAPABILITIES_PATH}\Capabilities\FileAssociations" "$0"
		DeleteRegKey HKCR $1
		DeleteRegValue HKLM "${CAPABILITIES_PATH}\Capabilities\FileAssociations" "$0"

		Goto FileTypeLoop_${MacroID}
	FileTypeDone_${MacroID}:
	
	; remove all MIME associations
	MimeTypeLoop_${MacroID}:
		EnumRegValue $0 HKLM "${CAPABILITIES_PATH}\Capabilities\MimeAssociations" 0
		StrCmp $0 "" MimeTypeDone_${MacroID}
		
		ReadRegStr $1 HKLM "${CAPABILITIES_PATH}\Capabilities\MimeAssociations" "$0"
		DeleteRegKey HKCR "$1"
		DeleteRegValue HKLM "${CAPABILITIES_PATH}\Capabilities\MimeAssociations" "$0"

		Goto MimeTypeLoop_${MacroID}
	MimeTypeDone_${MacroID}:
	
	; remove all protocol associations
	ProtocolLoop_${MacroID}:
		EnumRegValue $0 HKLM "${CAPABILITIES_PATH}\Capabilities\UrlAssociations" 0
		StrCmp $0 "" ProtocolDone_${MacroID}
		
		ReadRegStr $1 HKLM "${CAPABILITIES_PATH}\Capabilities\UrlAssociations" "$0"
		DeleteRegKey HKCR "$1"
		DeleteRegValue HKLM "${CAPABILITIES_PATH}\Capabilities\UrlAssociations" "$0"

		Goto ProtocolLoop_${MacroID}
	ProtocolDone_${MacroID}:
	
	
	; remove capabilities keys
	DeleteRegValue HKLM "Software\RegisteredApplications" "${CAPABILITIES_NAME}"
	DeleteRegKey HKLM ${CAPABILITIES_PATH}
	
	Pop $1
	Pop $0
	!verbose pop
	!undef MacroID
!macroend





!define RegisterFileType `!insertmacro RegisterFileTypeCall`
!define un.RegisterFileType `!insertmacro RegisterFileTypeCall`

!macro RegisterFileType
!macroend

!macro un.RegisterFileType
!macroend


!macro RegisterFileType_
	!verbose push
	!verbose ${_Capabilities_VERBOSE}
	
	Exch $R3 ;ext
	Exch 
	Exch $R2 ;exe
	Exch 2
	Exch $R1 ;icon
	Exch
	Exch 3
	Exch $R0 ;desc
	Push $0

	; create an association name
	; ex: .mp3 becomes ProgID.AssocFile.MP3
	${StrCase} $0 "$R3" "U"
	StrCpy $0 "${CAPABILITIES_PROGID}.AssocFile$0"
	
	; link capabilities to association in classes root
	WriteRegStr HKLM "${CAPABILITIES_PATH}\Capabilities\FileAssociations" "$R3" "$0"
	
	; write file association in classes root
	WriteRegStr HKCR "$0" "" "$R0"
	WriteRegStr HKCR "$0\DefaultIcon" "" "$R1"
	WriteRegStr HKCR "$0\shell" "" "Open"
	WriteRegStr HKCR "$0\shell\open" "" "&Open"
	WriteRegStr HKCR "$0\shell\open\command" "" '"$R2" "%1"'

	Pop $0
	Pop $R0
	Pop $R1
	Pop $R2
	Pop $R3

	!verbose pop
!macroend




!define RegisterMediaType `!insertmacro RegisterMediaTypeCall`
!define un.RegisterMediaType `!insertmacro RegisterMediaTypeCall`

!macro RegisterMediaType
!macroend

!macro un.RegisterMediaType
!macroend


!macro RegisterMediaType_
	!verbose push
	!verbose ${_Capabilities_VERBOSE}
	
	Exch $R3 ;ext
	Exch 
	Exch $R2 ;exe
	Exch 2
	Exch $R1 ;icon
	Exch
	Exch 3
	Exch $R0 ;desc
	Push $0


	; create an association name
	; ex: .mp3 becomes ProgID.AssocFile.MP3
	${StrCase} $0 "$R3" "U"
	StrCpy $0 "${CAPABILITIES_PROGID}.AssocFile$0"
	
	; link capabilities to association in classes root
	WriteRegStr HKLM "${CAPABILITIES_PATH}\Capabilities\FileAssociations" "$R3" "$0"
	
	; write file association in classes root
	WriteRegStr HKCR "$0" "" "$R0"
	WriteRegStr HKCR "$0\DefaultIcon" "" "$R1"
	WriteRegStr HKCR "$0\shell" "" "Play"
	WriteRegStr HKCR "$0\shell\open" "" "&Open"
	WriteRegStr HKCR "$0\shell\open\command" "" '"$R2" "%1"'
	WriteRegStr HKCR "$0\shell\play" "" "&Play"
	WriteRegStr HKCR "$0\shell\play\command" "" '"$R2" "%L"'

	Pop $0
	Pop $R0
	Pop $R1
	Pop $R2
	Pop $R3

	!verbose pop
!macroend




!define RegisterMimeType `!insertmacro RegisterMimeTypeCall`
!define un.RegisterMimeType `!insertmacro RegisterMimeTypeCall`

!macro RegisterMimeType
!macroend

!macro un.RegisterMimeType
!macroend


!macro RegisterMimeType_
	!verbose push
	!verbose ${_Capabilities_VERBOSE}
	
	Exch $R2 ;mime
	Exch 
	Exch $R1 ;shortname
	Exch 
	Exch 2
	Exch $R0 ;clsid
	
	Push $0

	; create an association name
	; ex: audio/mp3 becomes ProgID.AssocMIME.MP3
	${StrCase} $0 "$R1" "U"
	StrCpy $0 "${CAPABILITIES_PROGID}.AssocMIME.$0"
	
	; link capabilities to association in classes root
	WriteRegStr HKLM "${CAPABILITIES_PATH}\Capabilities\MimeAssociations" "$R2" "$0"
	
	; write file association in classes root
	WriteRegStr HKCR "$0\CLSID" "" "$R0"

	Pop $0
	Pop $R0
	Pop $R1

	!verbose pop
!macroend



!define RegisterProtocol `!insertmacro RegisterProtocolCall`
!define un.RegisterProtocol `!insertmacro RegisterProtocolCall`

!macro RegisterProtocol
!macroend

!macro un.RegisterProtocol
!macroend


!macro RegisterProtocol_
	!verbose push
	!verbose ${_Capabilities_VERBOSE}
	
	Exch $R3 ;protocol
	Exch 
	Exch $R2 ;exe
	Exch 2
	Exch $R1 ;icon
	Exch
	Exch 3
	Exch $R0 ;desc
	Push $0


	; create an association name
	; ex: http becomes ProgID.AssocProtocol.HTTP
	${StrCase} $0 "$R3" "U"
	StrCpy $0 "${CAPABILITIES_PROGID}.AssocProtocol.$0"
	
	; link capabilities to association in classes root
	WriteRegStr HKLM "${CAPABILITIES_PATH}\Capabilities\UrlAssociations" "$R3" "$0"
	
	; write file association in classes root
	WriteRegStr HKCR "$0" "" "$R0"
	WriteRegStr HKCR "$0\DefaultIcon" "" "$R1"
	WriteRegStr HKCR "$0\shell\open" "" "&Open"
	WriteRegStr HKCR "$0\shell\open\command" "" '"$R2" "%1"'

	Pop $0
	Pop $R0
	Pop $R1
	Pop $R2
	Pop $R3

	!verbose pop
!macroend




!define UnRegisterFileType `!insertmacro UnRegisterFileTypeCall`
!define un.UnRegisterFileType `!insertmacro UnRegisterFileTypeCall`

!macro UnRegisterFileType
!macroend

!macro un.UnRegisterFileType
!macroend

!macro UnRegisterFileType_
	!define MacroID ${__LINE__}
	!verbose push
	!verbose ${_Capabilities_VERBOSE}

	Exch $R0 ;ext
	Push $0
	
	ReadRegStr $0 HKLM "${CAPABILITIES_PATH}\Capabilities\FileAssociations" "$R0"
	StrCmp $0 "" skip_${MacroID}
	
	DeleteRegKey HKCR "$0"
	DeleteRegValue HKLM "${CAPABILITIES_PATH}\Capabilities\FileAssociations" "$R0"
	skip_${MacroID}:
	
	Pop $0
	Pop $R0
	!verbose pop
	!undef MacroID
!macroend



!define UnRegisterMimeType `!insertmacro UnRegisterMimeTypeCall`
!define un.UnRegisterMimeType `!insertmacro UnRegisterMimeTypeCall`

!macro UnRegisterMimeType
!macroend

!macro un.UnRegisterMimeType
!macroend

!macro UnRegisterMimeType_
	!define MacroID ${__LINE__}
	!verbose push
	!verbose ${_Capabilities_VERBOSE}

	Exch $R0 ;mime
	Push $0
	
	ReadRegStr $0 HKLM "${CAPABILITIES_PATH}\Capabilities\MimeAssociations" "$R0"
	StrCmp $0 "" skip_${MacroID}
	
	DeleteRegKey HKCR "$0"
	DeleteRegValue HKLM "${CAPABILITIES_PATH}\Capabilities\MimeAssociations" "$R0"
	skip_${MacroID}:
	
	Pop $0
	Pop $R0
	!verbose pop
	!undef MacroID
!macroend



!define UnRegisterProtocol `!insertmacro UnRegisterProtocolCall`
!define un.UnRegisterProtocol `!insertmacro UnRegisterProtocolCall`

!macro UnRegisterProtocol
!macroend

!macro un.UnRegisterProtocol
!macroend

!macro UnRegisterProtocol_
	!define MacroID ${__LINE__}
	!verbose push
	!verbose ${_Capabilities_VERBOSE}

	Exch $R0 ;protocol
	Push $0
	
	ReadRegStr $0 HKLM "${CAPABILITIES_PATH}\Capabilities\UrlAssociations" "$R0"
	StrCmp $0 "" skip_${MacroID}
	
	DeleteRegKey HKCR "$0"
	DeleteRegValue HKLM "${CAPABILITIES_PATH}\Capabilities\UrlAssociations" "$R0"
	skip_${MacroID}:
	
	Pop $0
	Pop $R0
	!verbose pop
	!undef MacroID
!macroend




!endif