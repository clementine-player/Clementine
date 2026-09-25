; Sets the AppUserModelID of a shortcut.
;
; Windows finds the name and icon to show for Clementine's media controls in
; the volume flyout by looking for a Start menu shortcut with the same
; AppUserModelID as the app.  This must match kAppUserModelId in
; src/core/windowsmediacontrols.cpp.
;
; Usage:
;   !insertmacro SetShortcutAppUserModelId "path\to\shortcut.lnk"

!define APP_USER_MODEL_ID "Clementine.Clementine"

!macro SetShortcutAppUserModelId shortcut
  Push "${shortcut}"
  Call SetShortcutAppUserModelId
!macroend

Function SetShortcutAppUserModelId
  Exch $0  ; Shortcut path
  Push $1  ; IShellLinkW
  Push $2  ; IPersistFile
  Push $3  ; IPropertyStore
  Push $4  ; PROPERTYKEY
  Push $5  ; PROPVARIANT
  Push $6  ; HRESULT

  ; CoCreateInstance(CLSID_ShellLink, IID_IShellLinkW)
  System::Call 'ole32::CoCreateInstance(g "{00021401-0000-0000-C000-000000000046}", p 0, i 1, g "{000214F9-0000-0000-C000-000000000046}", *p .r1) i .r6'
  IntCmp $6 0 0 done done

  ; IPersistFile::Load(shortcut, STGM_READWRITE)
  System::Call '$1->0(g "{0000010b-0000-0000-C000-000000000046}", *p .r2) i .r6'
  IntCmp $6 0 0 release_link release_link
  System::Call '$2->5(w r0, i 2) i .r6'
  IntCmp $6 0 0 release_file release_file

  ; IPropertyStore::SetValue(PKEY_AppUserModel_ID, VT_LPWSTR) then Commit
  System::Call '$1->0(g "{886D8EEB-8CF2-4446-8D02-CDBA1DBDCF99}", *p .r3) i .r6'
  IntCmp $6 0 0 release_file release_file
  System::Call '*(g "{9F4C2855-9F79-4B39-A8D0-E1D42DE1D5F3}", i 5) p .r4'
  System::Call '*(&i2 31, &i2 0, &i2 0, &i2 0, w "${APP_USER_MODEL_ID}", p 0) p .r5'
  System::Call '$3->6(p r4, p r5) i .r6'
  IntCmp $6 0 0 release_store release_store
  System::Call '$3->7() i .r6'
  IntCmp $6 0 0 release_store release_store

  ; IPersistFile::Save(NULL, TRUE)
  System::Call '$2->6(p 0, i 1) i .r6'

release_store:
  System::Free $4
  System::Free $5
  System::Call '$3->2()'
release_file:
  System::Call '$2->2()'
release_link:
  System::Call '$1->2()'
done:
  IntCmp $6 0 +2
    DetailPrint "Failed to set AppUserModelID on $0: $6"

  Pop $6
  Pop $5
  Pop $4
  Pop $3
  Pop $2
  Pop $1
  Pop $0
FunctionEnd
