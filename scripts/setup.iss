; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!
#define SETUP_COMPILER 1
#include "../src/common/version.h"
#define MyAppName PRODUCT_NAME
#define MyAppVerName PRODUCT_NAME + " " + PRODUCT_VERSION
#define MyAppPublisher "J�zef Starosczyk"
#define MyAppURL "http://www.copyhandler.com"

#define InstallerFilename "chsetup-" + PRODUCT_VERSION

#define ExeFilename32 "ch.exe"
#define ExeFilename64 "ch64.exe"
#define ShellExtFilename32 "chext.dll"
#define ShellExtFilename64 "chext64.dll"
#define LibCHCoreFilename32 "libchcore32u.dll"
#define LibCHCoreFilename64 "libchcore64u.dll"

#define LibCHEngineFilename32 "libchengine32u.dll"
#define LibCHEngineFilename64 "libchengine64u.dll"

#define LibSerializerFilename32 "libserializer32u.dll"
#define LibSerializerFilename64 "libserializer64u.dll"

#define LibStringFilename32 "libstring32u.dll"
#define LibStringFilename64 "libstring64u.dll"

#define LibLoggerFilename32 "liblogger32u.dll"
#define LibLoggerFilename64 "liblogger64u.dll"

#define LibictranslateFilename32 "libictranslate32u.dll"
#define LibictranslateFilename64 "libictranslate64u.dll"
#define ICTranslateFilename32 "ictranslate.exe"
#define ICTranslateFilename64 "ictranslate64.exe"
#define SQLite32 "sqlite3_32.dll"
#define SQLite64 "sqlite3_64.dll"

#define RegCHExtFilename32 "regchext.exe"
#define RegCHExtFilename64 "regchext64.exe"

#define MSRedistDir32 "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Redist\MSVC\14.16.27012\x86"
#define MSRedistDir64 "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Redist\MSVC\14.16.27012\x64"
#define DbgHelp32 "C:\Program Files (x86)\Windows Kits\8.1\Debuggers\x86"
#define DbgHelp64 "C:\Program Files (x86)\Windows Kits\8.1\Debuggers\x64"
#define UCrtDir32 "C:\Program Files (x86)\Windows Kits\10\Redist\10.0.17763.0\ucrt\DLLs\x86"
#define UCrtDir64 "C:\Program Files (x86)\Windows Kits\10\Redist\10.0.17763.0\ucrt\DLLs\x64"

[Setup]
AppName={#MyAppName}
AppVerName={#MyAppVerName}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=true
LicenseFile=..\License.txt
OutputDir=.\
OutputBaseFilename={#InstallerFilename}
Compression=lzma/ultra
SolidCompression=true
AppMutex=_Copy handler_ instance
ShowLanguageDialog=auto
AppID={{9CF6A157-F0E8-4216-B229-C0CA8204BE2C}
InternalCompressLevel=ultra
AppCopyright={#COPYRIGHT_INFO}
AppVersion={#PRODUCT_VERSION}
UninstallDisplayIcon={app}\{#ExeFilename32}
AppContact=ixen(at)copyhandler(dot)com
VersionInfoVersion=
VersionInfoTextVersion={#PRODUCT_VERSION}
VersionInfoCopyright={#COPYRIGHT_INFO}
ChangesEnvironment=true
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "brazilianportuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
Name: "catalan"; MessagesFile: "compiler:Languages\Catalan.isl"
Name: "czech"; MessagesFile: "compiler:Languages\Czech.isl"
Name: "danish"; MessagesFile: "compiler:Languages\Danish.isl"
Name: "dutch"; MessagesFile: "compiler:Languages\Dutch.isl"
Name: "finnish"; MessagesFile: "compiler:Languages\Finnish.isl"
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"
Name: "greek"; MessagesFile: "compiler:Languages\Greek.isl"
Name: "hebrew"; MessagesFile: "compiler:Languages\Hebrew.isl"
Name: "hungarian"; MessagesFile: "compiler:Languages\Hungarian.isl"
Name: "italian"; MessagesFile: "compiler:Languages\Italian.isl"
Name: "japanese"; MessagesFile: "compiler:Languages\Japanese.isl"
Name: "norwegian"; MessagesFile: "compiler:Languages\Norwegian.isl"
Name: "polish"; MessagesFile: "compiler:Languages\Polish.isl"
Name: "portuguese"; MessagesFile: "compiler:Languages\Portuguese.isl"
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "serbiancyrillic"; MessagesFile: "compiler:Languages\SerbianCyrillic.isl"
Name: "serbianlatin"; MessagesFile: "compiler:Languages\SerbianLatin.isl"
Name: "slovenian"; MessagesFile: "compiler:Languages\Slovenian.isl"
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"
Name: "ukrainian"; MessagesFile: "compiler:Languages\Ukrainian.isl"

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked
Name: quicklaunchicon; Description: {cm:CreateQuickLaunchIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked
Name: startatboot; Description: {cm:StartAtBoot}; Flags: unchecked

[Files]
Source: "..\bin\release\help\*.chm"; DestDir: "{app}\help"; Flags: ignoreversion recursesubdirs createallsubdirs restartreplace uninsrestartdelete
Source: "..\bin\release\langs\*.lng"; DestDir: "{app}\langs"; Flags: ignoreversion recursesubdirs createallsubdirs restartreplace uninsrestartdelete
Source: "..\License.txt"; DestDir: "{app}"; Flags: ignoreversion restartreplace uninsrestartdelete

; binaries - 32bit
Source: "..\bin\release\{#ExeFilename32}"; DestDir: "{app}"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: not Is64BitInstallMode
Source: "..\bin\release\{#ShellExtFilename32}"; DestDir: "{app}"; Flags: restartreplace uninsrestartdelete regserver replacesameversion; Check: not Is64BitInstallMode
Source: "..\bin\release\{#RegCHExtFilename32}"; DestDir: "{app}"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: not Is64BitInstallMode
Source: "..\bin\release\{#LibCHCoreFilename32}"; DestDir: "{app}"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: not Is64BitInstallMode
Source: "..\bin\release\{#LibCHEngineFilename32}"; DestDir: "{app}"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: not Is64BitInstallMode
Source: "..\bin\release\{#LibSerializerFilename32}"; DestDir: "{app}"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: not Is64BitInstallMode
Source: "..\bin\release\{#LibStringFilename32}"; DestDir: "{app}"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: not Is64BitInstallMode
Source: "..\bin\release\{#LibLoggerFilename32}"; DestDir: "{app}"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: not Is64BitInstallMode
Source: "..\bin\release\{#LibictranslateFilename32}"; DestDir: "{app}"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: not Is64BitInstallMode
Source: "..\bin\release\{#ICTranslateFilename32}"; DestDir: "{app}"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: not Is64BitInstallMode
Source: "..\bin\release\{#SQLite32}"; DestDir: "{app}"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: not Is64BitInstallMode

Source: "{#MSRedistDir32}\Microsoft.VC141.CRT\*"; DestDir: "{app}"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: not Is64BitInstallMode
Source: "{#MSRedistDir32}\Microsoft.VC141.MFC\*"; DestDir: "{app}"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: not Is64BitInstallMode
Source: "{#UCrtDir32}\*"; DestDir: "{app}"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: not Is64BitInstallMode
Source: "{#DbgHelp32}\dbghelp.dll"; DestDir: "{app}"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: not Is64BitInstallMode

; binaries - 64bit
Source: "..\bin\release\{#ExeFilename64}"; DestDir: "{app}"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: Is64BitInstallMode
Source: "..\bin\release\{#ShellExtFilename64}"; DestDir: "{app}"; Flags: restartreplace uninsrestartdelete regserver replacesameversion; Check: Is64BitInstallMode
Source: "..\bin\release\{#RegCHExtFilename64}"; DestDir: "{app}"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: Is64BitInstallMode
Source: "..\bin\release\{#LibCHCoreFilename64}"; DestDir: "{app}"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: Is64BitInstallMode
Source: "..\bin\release\{#LibCHEngineFilename64}"; DestDir: "{app}"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: Is64BitInstallMode
Source: "..\bin\release\{#LibSerializerFilename64}"; DestDir: "{app}"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: Is64BitInstallMode
Source: "..\bin\release\{#LibStringFilename64}"; DestDir: "{app}"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: Is64BitInstallMode
Source: "..\bin\release\{#LibLoggerFilename64}"; DestDir: "{app}"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: Is64BitInstallMode
Source: "..\bin\release\{#LibictranslateFilename64}"; DestDir: "{app}"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: Is64BitInstallMode
Source: "..\bin\release\{#ICTranslateFilename64}"; DestDir: "{app}"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: Is64BitInstallMode
Source: "..\bin\release\{#SQLite64}"; DestDir: "{app}"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: Is64BitInstallMode

Source: "{#MSRedistDir64}\Microsoft.VC141.CRT\*"; DestDir: "{app}"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: Is64BitInstallMode
Source: "{#MSRedistDir64}\Microsoft.VC141.MFC\*"; DestDir: "{app}"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: Is64BitInstallMode
Source: "{#UCrtDir64}\*"; DestDir: "{app}"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: Is64BitInstallMode
Source: "{#DbgHelp64}\dbghelp.dll"; DestDir: "{app}"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: Is64BitInstallMode

; binaries - 32bit shellext on 64bit system
Source: "..\bin\release\{#ShellExtFilename32}"; DestDir: "{app}\ShellExt32"; Flags: restartreplace uninsrestartdelete regserver replacesameversion; Check: Is64BitInstallMode
Source: "..\bin\release\{#LibCHCoreFilename32}"; DestDir: "{app}\ShellExt32"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: Is64BitInstallMode
Source: "..\bin\release\{#LibCHEngineFilename32}"; DestDir: "{app}\ShellExt32"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: Is64BitInstallMode
Source: "..\bin\release\{#LibSerializerFilename32}"; DestDir: "{app}\ShellExt32"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: Is64BitInstallMode
Source: "..\bin\release\{#LibStringFilename32}"; DestDir: "{app}\ShellExt32"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: Is64BitInstallMode
Source: "..\bin\release\{#LibLoggerFilename32}"; DestDir: "{app}\ShellExt32"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: Is64BitInstallMode
Source: "..\bin\release\{#SQLite32}"; DestDir: "{app}\ShellExt32"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: Is64BitInstallMode
Source: "{#MSRedistDir32}\Microsoft.VC141.CRT\*"; DestDir: "{app}\ShellExt32"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: Is64BitInstallMode
Source: "{#MSRedistDir32}\Microsoft.VC141.MFC\*"; DestDir: "{app}\ShellExt32"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: Is64BitInstallMode
Source: "{#UCrtDir32}\*"; DestDir: "{app}\ShellExt32"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: Is64BitInstallMode
Source: "{#DbgHelp32}\dbghelp.dll"; DestDir: "{app}\ShellExt32"; Flags: ignoreversion restartreplace uninsrestartdelete; Check: Is64BitInstallMode

; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: {group}\{#MyAppName}; Filename: {app}\{code:ExpandArch|ExeFilename}; WorkingDir: {app}
Name: {group}\{cm:TranslateCopyHandler}; Filename: {app}\{code:ExpandArch|ICTranslateFilename}; Parameters: """{app}\langs\english.lng"""; WorkingDir: {app}\lang
Name: {group}\{cm:UninstallCopyHandler}; Filename: {uninstallexe}; WorkingDir: {app}
Name: {commondesktop}\{#MyAppName}; Filename: {app}\{code:ExpandArch|ExeFilename}; Tasks: desktopicon; WorkingDir: {app}
Name: {commonappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}; Filename: {app}\{code:ExpandArch|ExeFilename}; Tasks: quicklaunchicon; WorkingDir: {app}

[Run]
Filename: "{app}\{code:ExpandArch|ExeFilename}"; Flags: nowait postinstall skipifsilent; Description: "{cm:LaunchProgram,{#MyAppName}}"
Filename: "Reg.exe"; Parameters: "delete ""HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\Run"" /v ""Copy Handler"" /f"; Flags: runasoriginaluser;
Filename: "Reg.exe"; Parameters: "add ""HKCU\SOFTWARE\Microsoft\Windows\CurrentVersion\Run"" /v ""Copy Handler"" /t REG_SZ /d ""{app}\{code:ExpandArch|ExeFilename}"" /f"; Flags: runasoriginaluser postinstall; Tasks: startatboot

[Registry]
Root: "HKLM"; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\Run"; ValueType: none; ValueName: "Copy Handler"; Flags: deletevalue uninsdeletevalue

[Dirs]
Name: {app}\help; Flags: uninsalwaysuninstall
Name: {app}\langs; Flags: uninsalwaysuninstall

[CustomMessages]
TranslateCopyHandler=Translate Copy Handler
polish.TranslateCopyHandler=Przet�umacz program Copy Handler
UninstallCopyHandler=Uninstall Copy Handler
polish.UninstallCopyHandler=Odinstaluj program Copy Handler
StartAtBoot=Run program at system startup
polish.StartAtBoot=Uruchom program przy starcie systemu

[ThirdParty]
CompileLogMethod=append

[Code]
function ExpandArch(ConstantStr: String): String;
begin
	if Is64BitInstallMode then
	begin
		case ConstantStr of
			'ExeFilename': Result := '{#ExeFilename64}';
			'ICTranslateFilename': Result := '{#ICTranslateFilename64}';
		end;
	end
	else
	begin
		case ConstantStr of
			'ExeFilename': Result := '{#ExeFilename32}';
			'ICTranslateFilename': Result := '{#ICTranslateFilename32}';
		end;
	end;
end;
