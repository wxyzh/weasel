; weasel installation script
!include FileFunc.nsh
!include LogicLib.nsh
!include MUI2.nsh
!include x64.nsh
!define LIBRARY_X64

Unicode true

!ifndef WEASEL_VERSION
!define WEASEL_VERSION 0.15.1
!endif

!ifndef WEASEL_BUILD
!define WEASEL_BUILD 0
!endif

!define WEASEL_ROOT $INSTDIR\weasel-${WEASEL_VERSION}
!define REG_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\Weasel"

; The name of the installer
Name "小狼毫 ${WEASEL_VERSION}"

; The file to write
OutFile "archives\weasel-${WEASEL_VERSION}.${WEASEL_BUILD}-installer_x64.exe"

VIProductVersion "${WEASEL_VERSION}.${WEASEL_BUILD}"
VIAddVersionKey /LANG=2052 "ProductName" "小狼毫"
VIAddVersionKey /LANG=2052 "Comments" "Powered by RIME | 中州韵输入法引擎"
VIAddVersionKey /LANG=2052 "CompanyName" "式恕堂"
VIAddVersionKey /LANG=2052 "LegalCopyright" "Copyleft RIME Developers"
VIAddVersionKey /LANG=2052 "FileDescription" "小狼毫输入法"
VIAddVersionKey /LANG=2052 "FileVersion" "${WEASEL_VERSION}"

!define MUI_ICON ..\resource\weasel.ico
SetCompressor /SOLID lzma

; The default installation directory
InstallDir $PROGRAMFILES64\Rime

; Registry key to check for directory (so if you install again, it will
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Rime\Weasel" "InstallDir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------

; Pages

!insertmacro MUI_PAGE_LICENSE "LICENSE.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

;--------------------------------

; Languages

!insertmacro MUI_LANGUAGE "SimpChinese"
LangString DISPLAYNAME ${LANG_SIMPCHINESE} "小狼毫输入法"
LangString LNKFORMANUAL ${LANG_SIMPCHINESE} "【小狼毫】说明书"
LangString LNKFORSETTING ${LANG_SIMPCHINESE} "【小狼毫】输入法设定"
LangString LNKFORDICT ${LANG_SIMPCHINESE} "【小狼毫】用户词典管理"
LangString LNKFORSYNC ${LANG_SIMPCHINESE} "【小狼毫】用户资料同步"
LangString LNKFORDEPLOY ${LANG_SIMPCHINESE} "【小狼毫】重新部署"
LangString LNKFORSERVER ${LANG_SIMPCHINESE} "小狼毫算法服务"
LangString LNKFORUSERFOLDER ${LANG_SIMPCHINESE} "【小狼毫】用户文件夹"
LangString LNKFORAPPFOLDER ${LANG_SIMPCHINESE} "【小狼毫】程序文件夹"
LangString LNKFORUPDATER ${LANG_SIMPCHINESE} "【小狼毫】检查新版本"
LangString LNKFORSETUP ${LANG_SIMPCHINESE} "【小狼毫】安装选项"
LangString LNKFORUNINSTALL ${LANG_SIMPCHINESE} "卸载小狼毫"

!insertmacro MUI_LANGUAGE "TradChinese"
LangString DISPLAYNAME ${LANG_TRADCHINESE} "小狼毫輸入法"
LangString LNKFORMANUAL ${LANG_TRADCHINESE} "【小狼毫】說明書"
LangString LNKFORSETTING ${LANG_TRADCHINESE} "【小狼毫】輸入法設定"
LangString LNKFORDICT ${LANG_TRADCHINESE} "【小狼毫】用戶詞典管理"
LangString LNKFORSYNC ${LANG_TRADCHINESE} "【小狼毫】用戶資料同步"
LangString LNKFORDEPLOY ${LANG_TRADCHINESE} "【小狼毫】重新部署"
LangString LNKFORSERVER ${LANG_TRADCHINESE} "小狼毫算法服務"
LangString LNKFORUSERFOLDER ${LANG_TRADCHINESE} "【小狼毫】用戶文件夾"
LangString LNKFORAPPFOLDER ${LANG_TRADCHINESE} "【小狼毫】程序文件夾"
LangString LNKFORUPDATER ${LANG_TRADCHINESE} "【小狼毫】檢查新版本"
LangString LNKFORSETUP ${LANG_TRADCHINESE} "【小狼毫】安裝選項"
LangString LNKFORUNINSTALL ${LANG_TRADCHINESE} "卸載小狼毫"

!insertmacro MUI_LANGUAGE "English"
LangString DISPLAYNAME ${LANG_ENGLISH} "Weasel"
LangString LNKFORMANUAL ${LANG_ENGLISH} "[Weasel] Manual"
LangString LNKFORSETTING ${LANG_ENGLISH} "[Weasel] Settings"
LangString LNKFORDICT ${LANG_ENGLISH} "[Weasel] Dictionary Manager"
LangString LNKFORSYNC ${LANG_ENGLISH} "[Weasel] Sync User Profile"
LangString LNKFORDEPLOY ${LANG_ENGLISH} "[Weasel] Deploy"
LangString LNKFORSERVER ${LANG_ENGLISH} "Weasel Server"
LangString LNKFORUSERFOLDER ${LANG_ENGLISH} "[Weasel] User Folder"
LangString LNKFORAPPFOLDER ${LANG_ENGLISH} "[Weasel] App Folder"
LangString LNKFORUPDATER ${LANG_ENGLISH} "[Weasel] Check for Updates"
LangString LNKFORSETUP ${LANG_ENGLISH} "[Weasel] Installation Preference"
LangString LNKFORUNINSTALL ${LANG_ENGLISH} "Uninstall Weasel"
;--------------------------------

!macro TIP_WHEN_AMD64_INSTALLER_RUNAT_X86
!ifdef LIBRARY_X64
  ${If} ${RunningX64}
  ${else}
     MessageBox MB_OK|MB_ICONINFORMATION "请在64位操作系统下运行。"
     Abort
  ${EndIf}
!endif
!macroend

Function .onInit
  !insertmacro TIP_WHEN_AMD64_INSTALLER_RUNAT_X86
  SetShellVarContext all

  System::Call 'SHCore::SetProcessDpiAwareness(i 2)i.R0'

  SetRegView 64

  ReadRegStr $R0 HKLM \
  "Software\Microsoft\Windows\CurrentVersion\Uninstall\Weasel" \
  "UninstallString"
  StrCmp $R0 "" done

  StrCpy $0 "Upgrade"
  IfSilent uninst 0
  MessageBox MB_OKCANCEL|MB_ICONINFORMATION \
  "安装前，我打算先卸载旧版本的小狼毫。$\n$\n按下「确定」移除旧版本，按下「取消」放弃本次安装。" \
  IDOK uninst
  Abort

uninst:
  System::Call 'SHCore::SetProcessDpiAwareness(i 2)i.R0'
  ; Backup data directory from previous installation, user files may exist
  ReadRegStr $R1 HKLM SOFTWARE\Rime\Weasel "WeaselRoot"
  StrCmp $R1 "" call_uninstaller
  IfFileExists $R1\data\*.* 0 call_uninstaller
  CreateDirectory $TEMP\weasel-backup
  CopyFiles $R1\data\*.* $TEMP\weasel-backup

call_uninstaller:
  ExecWait '$R0 /S'
  Sleep 800

done:
FunctionEnd

; The stuff to install
Section "Weasel"

  SectionIn RO

  SetRegView 64

  ; Write the new installation path into the registry
  WriteRegStr HKLM SOFTWARE\Rime\Weasel "InstallDir" "$INSTDIR"

  ; Reset INSTDIR for the new version
  StrCpy $INSTDIR "${WEASEL_ROOT}"

  IfFileExists "$INSTDIR\WeaselServer.exe" 0 +2
  ExecWait '"$INSTDIR\WeaselServer.exe" /quit'

  SetOverwrite try
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR

  IfFileExists $TEMP\weasel-backup\*.* 0 program_files
  CreateDirectory $INSTDIR\data
  CopyFiles $TEMP\weasel-backup\*.* $INSTDIR\data
  RMDir /r $TEMP\weasel-backup

program_files:
  File "LICENSE.txt"
  File "README.txt"
  File "7-zip-license.txt"
  File "7z.dll"
  File "7z.exe"
  File "COPYING-curl.txt"
  File "curl.exe"
  File "curl-ca-bundle.crt"
  File "rime-install.bat"
  File "rime-install-config.bat"
  File "weasel.dll"
  ${If} ${RunningX64}
    File "weaselx64.dll"
  ${EndIf}
  File "weaselt.dll"
  ${If} ${RunningX64}
    File "weaseltx64.dll"
  ${EndIf}
  ; File "weasel.ime"
  ; ${If} ${RunningX64}
  ;   File "weaselx64.ime"
  ; ${EndIf}
  ; File "weaselt.ime"
  ; ${If} ${RunningX64}
  ;   File "weaseltx64.ime"
  ; ${EndIf}
  File "WeaselDeployer.exe"
  File "WeaselServer.exe"
  File "WeaselSetup.exe"
  File "rime.dll"
  File "WinSparkle.dll"
  ; shared data files
  SetOutPath $INSTDIR\data
  File "data\*.yaml"
  File "data\rime.lua"
  File /nonfatal "data\*.txt"
  ; File /nonfatal "data\*.gram"
  ; lua files
  SetOutPath $INSTDIR\data\lua
  File "data\lua\*.lua"
  ; opencc data files
  SetOutPath $INSTDIR\data\opencc
  File "data\opencc\*.json"
  File "data\opencc\*.ocd2"
  ; images
  SetOutPath $INSTDIR\data\preview
  File "data\preview\*.png"

  SetOutPath $INSTDIR

  ; test /T flag for zh_TW locale
  StrCpy $R2  "/i"
  ${GetParameters} $R0
  ClearErrors
  ${GetOptions} $R0 "/S" $R1
  IfErrors +2 0
  StrCpy $R2 "/s"
  ${GetOptions} $R0 "/T" $R1
  IfErrors +2 0
  StrCpy $R2 "/t"

  ExecWait '"$INSTDIR\WeaselSetup.exe" $R2'

  ; run as user...
  ExecWait "$INSTDIR\WeaselDeployer.exe /install"

  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "${REG_UNINST_KEY}" "DisplayName" "$(DISPLAYNAME)"
  WriteRegStr HKLM "${REG_UNINST_KEY}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegStr HKLM "${REG_UNINST_KEY}" "DisplayVersion" "${WEASEL_VERSION}.${WEASEL_BUILD}"
  WriteRegStr HKLM "${REG_UNINST_KEY}" "Publisher" "式恕堂"
  WriteRegStr HKLM "${REG_UNINST_KEY}" "URLInfoAbout" "https://rime.im/"
  WriteRegStr HKLM "${REG_UNINST_KEY}" "HelpLink" "https://rime.im/docs/"
  WriteRegDWORD HKLM "${REG_UNINST_KEY}" "NoModify" 1
  WriteRegDWORD HKLM "${REG_UNINST_KEY}" "NoRepair" 1
  WriteUninstaller "$INSTDIR\uninstall.exe"

  ; run as user...
  ExecWait "$INSTDIR\WeaselDeployer.exe /install"

  ; Write autorun key
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "WeaselServer" "$INSTDIR\WeaselServer.exe"
  ; Start WeaselServer
  Exec "$INSTDIR\WeaselServer.exe"

  ; Prompt reboot
  StrCmp $0 "Upgrade" 0 +2
  ; SetRebootFlag true

  ; Prompt logoff
  MessageBox MB_YESNO|MB_ICONINFORMATION \
  "安装完毕，注销后生效。$\n$\n「是」立即注销，「否」手动注销。" \
  /SD IDYES IDYES label_yes IDNO label_no

  label_yes:
  System::Call "User32::ExitWindowsEx(i 0x0, i 0x0) i .r0"

  label_no:

SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"
  SetShellVarContext all
  CreateDirectory "$SMPROGRAMS\$(DISPLAYNAME)"
  CreateShortCut "$SMPROGRAMS\$(DISPLAYNAME)\$(LNKFORMANUAL).lnk" "$INSTDIR\README.txt"
  CreateShortCut "$SMPROGRAMS\$(DISPLAYNAME)\$(LNKFORSETTING).lnk" "$INSTDIR\WeaselDeployer.exe" "" "$SYSDIR\shell32.dll" 21
  CreateShortCut "$SMPROGRAMS\$(DISPLAYNAME)\$(LNKFORDICT).lnk" "$INSTDIR\WeaselDeployer.exe" "/dict" "$SYSDIR\shell32.dll" 6
  CreateShortCut "$SMPROGRAMS\$(DISPLAYNAME)\$(LNKFORSYNC).lnk" "$INSTDIR\WeaselDeployer.exe" "/sync" "$SYSDIR\shell32.dll" 26
  CreateShortCut "$SMPROGRAMS\$(DISPLAYNAME)\$(LNKFORDEPLOY).lnk" "$INSTDIR\WeaselDeployer.exe" "/deploy" "$SYSDIR\shell32.dll" 144
  CreateShortCut "$SMPROGRAMS\$(DISPLAYNAME)\$(LNKFORSERVER).lnk" "$INSTDIR\WeaselServer.exe" "" "$INSTDIR\WeaselServer.exe" 0
  CreateShortCut "$SMPROGRAMS\$(DISPLAYNAME)\$(LNKFORUSERFOLDER).lnk" "$INSTDIR\WeaselServer.exe" "/userdir" "$SYSDIR\shell32.dll" 126
  CreateShortCut "$SMPROGRAMS\$(DISPLAYNAME)\$(LNKFORAPPFOLDER).lnk" "$INSTDIR\WeaselServer.exe" "/weaseldir" "$SYSDIR\shell32.dll" 19
  CreateShortCut "$SMPROGRAMS\$(DISPLAYNAME)\$(LNKFORUPDATER).lnk" "$INSTDIR\WeaselServer.exe" "/update" "$SYSDIR\shell32.dll" 13
  CreateShortCut "$SMPROGRAMS\$(DISPLAYNAME)\$(LNKFORSETUP).lnk" "$INSTDIR\WeaselSetup.exe" "" "$SYSDIR\shell32.dll" 162
  CreateShortCut "$SMPROGRAMS\$(DISPLAYNAME)\$(LNKFORUNINSTALL).lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0

SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"    

  ExecWait '"$INSTDIR\WeaselServer.exe" /quit'

  ExecWait '"$INSTDIR\WeaselSetup.exe" /u'

  ; Remove registry keys
  SetRegView 64
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Weasel"
  DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "WeaselServer"
  DeleteRegKey HKLM SOFTWARE\Rime

  ; Remove files and uninstaller
  SetOutPath $TEMP
  Delete /REBOOTOK "$INSTDIR\data\opencc\*.*"
  Delete /REBOOTOK "$INSTDIR\data\preview\*.*"
  Delete /REBOOTOK "$INSTDIR\data\lua\*.*"
  Delete /REBOOTOK "$INSTDIR\data\*.*"
  Delete /REBOOTOK "$INSTDIR\*.*"
  RMDir /REBOOTOK "$INSTDIR\data\opencc"
  RMDir /REBOOTOK "$INSTDIR\data\preview"
  RMDir /REBOOTOK "$INSTDIR\data\lua"
  RMDir /REBOOTOK "$INSTDIR\data"
  RMDir /REBOOTOK "$INSTDIR"
  SetShellVarContext all
  Delete /REBOOTOK "$SMPROGRAMS\$(DISPLAYNAME)\*.*"
  RMDir /REBOOTOK "$SMPROGRAMS\$(DISPLAYNAME)"

  ; Prompt reboot
  ; SetRebootFlag true

  ; Prompt logoff
  MessageBox MB_YESNO|MB_ICONINFORMATION \
  "卸载完毕，注销后生效。$\n$\n「是」立即注销，「否」手动注销。" \
  /SD IDYES IDYES label_yes IDNO label_no

  label_yes:
  System::Call "User32::ExitWindowsEx(i 0x0, i 0x0) i .r0"

  label_no:

SectionEnd
