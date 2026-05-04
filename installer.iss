; GK_OBS_Lite_AMD — Inno Setup installer script
; Packages the CMake RelWithDebInfo build output

#define MyAppName "GK_OBS_Lite_AMD"
#define MyAppVersion "0.6.7"
#define MyAppPublisher "George Karagioules"
#define MyAppExeName "obs64.exe"
#define MyAppId "{{E7A3F1B2-5D8C-4A6E-9F0B-3C7D2E1A4B5F}"
#define BuildDir "build_amd_lite\rundir\RelWithDebInfo"

[Setup]
AppId={#MyAppId}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL=https://github.com/georgekgr12/GK_OBS_LITE_AMD
AppSupportURL=https://github.com/georgekgr12/GK_OBS_LITE_AMD/issues
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
LicenseFile=installer_assets\LICENSE.txt
OutputDir=dist-installer
OutputBaseFilename=GK_OBS_Lite_AMD_Setup_{#MyAppVersion}
Compression=lzma2
SolidCompression=yes
SetupIconFile=UI\cmake\windows\obs-studio.ico
UninstallDisplayIcon={app}\bin\64bit\obs-studio.ico
UninstallDisplayName={#MyAppName}
WizardStyle=modern
WizardImageFile=installer_assets\wizard_large.bmp
WizardSmallImageFile=installer_assets\wizard_small.bmp
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
CloseApplications=force
RestartApplications=no
VersionInfoVersion={#MyAppVersion}.0
VersionInfoCompany={#MyAppPublisher}
VersionInfoDescription=GK OBS Lite AMD Edition — Lightweight AMD-optimized OBS
VersionInfoCopyright=Copyright (c) 2026 {#MyAppPublisher}
VersionInfoProductName={#MyAppName}
VersionInfoProductVersion={#MyAppVersion}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"
Name: "startmenu"; Description: "Create a Start Menu shortcut"; GroupDescription: "{cm:AdditionalIcons}"

[Files]
; Main binary directory (obs64.exe + DLLs + Qt plugins)
Source: "{#BuildDir}\bin\64bit\*"; DestDir: "{app}\bin\64bit"; Flags: ignoreversion recursesubdirs createallsubdirs
; Data directory (themes, locale, services, etc.)
Source: "{#BuildDir}\data\*"; DestDir: "{app}\data"; Flags: ignoreversion recursesubdirs createallsubdirs
; Plugin DLLs
Source: "{#BuildDir}\obs-plugins\*"; DestDir: "{app}\obs-plugins"; Flags: ignoreversion recursesubdirs createallsubdirs
; Icon for uninstaller
Source: "UI\cmake\windows\obs-studio.ico"; DestDir: "{app}\bin\64bit"; Flags: ignoreversion
; Portable mode sentinel (settings stored next to the app)
Source: "installer_assets\portable_mode"; DestDir: "{app}"; Flags: onlyifdoesntexist
; License
Source: "installer_assets\LICENSE.txt"; DestDir: "{app}"; Flags: ignoreversion

[Dirs]
; Pre-create config directory with user write permissions
Name: "{app}\config"; Permissions: users-modify

[Icons]
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\bin\64bit\{#MyAppExeName}"; WorkingDir: "{app}\bin\64bit"; IconFilename: "{app}\bin\64bit\obs-studio.ico"; Tasks: desktopicon
Name: "{group}\{#MyAppName}"; Filename: "{app}\bin\64bit\{#MyAppExeName}"; WorkingDir: "{app}\bin\64bit"; IconFilename: "{app}\bin\64bit\obs-studio.ico"; Tasks: startmenu
Name: "{group}\Uninstall {#MyAppName}"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\bin\64bit\{#MyAppExeName}"; WorkingDir: "{app}\bin\64bit"; Description: "Launch {#MyAppName}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
; Clean up logs and crash reports on uninstall; preserve user settings in config\
Type: filesandordirs; Name: "{app}\config\obs-studio\logs"
Type: filesandordirs; Name: "{app}\config\obs-studio\crashes"
Type: filesandordirs; Name: "{app}\config\obs-studio\profiler_data"
Type: filesandordirs; Name: "{app}\config\obs-studio\plugin_config\obs-browser"
