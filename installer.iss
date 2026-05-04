; RDNA Cast — Inno Setup installer script
; Packages the CMake RelWithDebInfo build output

#define MyAppName "RDNA Cast"
#define MyAppNameShort "RDNACast"
#define MyAppVersion "0.7.4"
#define MyAppPublisher "George Karagioules"
#define MyAppExeName "obs64.exe"
; New AppId for RDNA Cast (separate from legacy GK_OBS_Lite_AMD).
; The legacy AppId is detected at install time and uninstalled silently;
; user config is migrated from the old install path.
#define MyAppId "{{B7C9E2D4-1F5A-4E8B-9C3D-7A2F8E1B5C09}"
#define LegacyAppId "{E7A3F1B2-5D8C-4A6E-9F0B-3C7D2E1A4B5F}_is1"
#define BuildDir "build_amd_lite\rundir\RelWithDebInfo"

[Setup]
AppId={#MyAppId}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL=https://github.com/georgekgr12/RDNACast
AppSupportURL=https://github.com/georgekgr12/RDNACast/issues
DefaultDirName={autopf}\{#MyAppNameShort}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
UsePreviousAppDir=no
LicenseFile=installer_assets\LICENSE.txt
OutputDir=dist-installer
OutputBaseFilename=RDNACast_Setup_{#MyAppVersion}
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
VersionInfoDescription=RDNA Cast — High-performance streaming for AMD Radeon
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
; Plugin DLLs (RDNA Cast plugin folder name)
Source: "{#BuildDir}\rdna-plugins\*"; DestDir: "{app}\rdna-plugins"; Flags: ignoreversion recursesubdirs createallsubdirs
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
; shellexec uses ShellExecuteEx so UAC handling/elevation transitions work cleanly
; (CreateProcess fails with ERROR_ELEVATION_REQUIRED 740 when transitioning out
; of the elevated installer context for the user-token postinstall launch).
Filename: "{app}\bin\64bit\{#MyAppExeName}"; WorkingDir: "{app}\bin\64bit"; Description: "Launch {#MyAppName}"; Flags: nowait postinstall skipifsilent shellexec

[UninstallDelete]
; Clean up logs and crash reports on uninstall; preserve user settings in config\
Type: filesandordirs; Name: "{app}\config\rdnacast\logs"
Type: filesandordirs; Name: "{app}\config\rdnacast\crashes"
Type: filesandordirs; Name: "{app}\config\rdnacast\profiler_data"

[Code]
var
  LegacyInstallPath: string;

function GetLegacyInfo(var installDir: string; var uninstallStr: string): Boolean;
begin
  Result := RegQueryStringValue(HKLM,
              'Software\Microsoft\Windows\CurrentVersion\Uninstall\{#LegacyAppId}',
              'InstallLocation', installDir)
            and RegQueryStringValue(HKLM,
              'Software\Microsoft\Windows\CurrentVersion\Uninstall\{#LegacyAppId}',
              'UninstallString', uninstallStr);
  if not Result then begin
    Result := RegQueryStringValue(HKLM32,
                'Software\Microsoft\Windows\CurrentVersion\Uninstall\{#LegacyAppId}',
                'InstallLocation', installDir)
              and RegQueryStringValue(HKLM32,
                'Software\Microsoft\Windows\CurrentVersion\Uninstall\{#LegacyAppId}',
                'UninstallString', uninstallStr);
  end;
end;

function InitializeSetup(): Boolean;
var
  installDir, uninstallStr: string;
  resultCode: Integer;
begin
  Result := True;
  LegacyInstallPath := '';

  if GetLegacyInfo(installDir, uninstallStr) then begin
    LegacyInstallPath := installDir;
    uninstallStr := RemoveQuotes(uninstallStr);

    if MsgBox(
      'A previous installation of GK_OBS_Lite_AMD was found at:' + #13#10 +
      installDir + #13#10 + #13#10 +
      'It will be removed before installing RDNA Cast at a clean location.' + #13#10 +
      'Your settings, scenes, and profiles will be migrated automatically.' + #13#10 + #13#10 +
      'Continue?', mbConfirmation, MB_YESNO) = IDNO then
    begin
      Result := False;
      Exit;
    end;

    if not Exec(uninstallStr, '/SILENT /NORESTART /SUPPRESSMSGBOXES', '',
                SW_HIDE, ewWaitUntilTerminated, resultCode) then
    begin
      MsgBox(
        'Failed to launch the previous version uninstaller.' + #13#10 +
        'Please uninstall GK_OBS_Lite_AMD manually before installing RDNA Cast.',
        mbError, MB_OK);
      Result := False;
      Exit;
    end;
  end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
var
  oldConfig, newConfig: string;
  resultCode: Integer;
begin
  if (CurStep = ssPostInstall) and (LegacyInstallPath <> '') then begin
    oldConfig := AddBackslash(LegacyInstallPath) + 'config';
    newConfig := AddBackslash(ExpandConstant('{app}')) + 'config';

    if DirExists(oldConfig) then begin
      // Best-effort copy; xcopy is built into Windows
      Exec(ExpandConstant('{cmd}'),
           '/c xcopy /E /I /Y /Q "' + oldConfig + '" "' + newConfig + '"',
           '', SW_HIDE, ewWaitUntilTerminated, resultCode);

      // Remove the now-orphaned legacy directory tree (best-effort)
      Exec(ExpandConstant('{cmd}'),
           '/c rmdir /S /Q "' + LegacyInstallPath + '"',
           '', SW_HIDE, ewWaitUntilTerminated, resultCode);
    end;
  end;
end;
