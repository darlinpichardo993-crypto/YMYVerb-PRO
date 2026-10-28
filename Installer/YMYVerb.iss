; Inno Setup script for YMYVerb VST3 effect and standalone desktop app.
#define MyAppName "YMYVerb"
#define MyAppVersion "1.0.1"
#ifndef VST3Dir
  #define VST3Dir "..\build\YMYVerb_artefacts\Release\VST3\YMYVerb.vst3"
#endif
#ifndef StandaloneExe
  #define StandaloneExe "..\build\YMYVerb_artefacts\Release\Standalone\YMYVerb.exe"
#endif

[Setup]
AppId={{9F3C6B2E-5A41-4C8E-8B1D-7E2A4D6C1F10}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher=YMY Records
DefaultDirName={autopf}\YMY Records\YMYVerb
DisableProgramGroupPage=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
OutputDir=..\dist
OutputBaseFilename=YMYVerb_Setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
UninstallDisplayName={#MyAppName} {#MyAppVersion}

[Languages]
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "docs"; Description: "Instalar la documentación (README)"; Flags: checkedonce
Name: "desktopicon"; Description: "Crear un acceso directo en el escritorio"; GroupDescription: "Accesos directos:"; Flags: unchecked

[Files]
; Install plugin into the standard system-wide VST3 folder.
Source: "{#VST3Dir}\*"; DestDir: "{commoncf64}\VST3\YMYVerb.vst3"; Flags: recursesubdirs createallsubdirs ignoreversion
; Standalone app is installed alongside the documentation.
Source: "{#StandaloneExe}"; DestDir: "{app}"; DestName: "YMYVerb.exe"; Flags: ignoreversion
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion; Tasks: docs

[Icons]
Name: "{autoprograms}\YMY Records\YMYVerb"; Filename: "{app}\YMYVerb.exe"; WorkingDir: "{app}"
Name: "{autodesktop}\YMYVerb"; Filename: "{app}\YMYVerb.exe"; WorkingDir: "{app}"; Tasks: desktopicon

[Code]
function InitializeSetup(): Boolean;
begin
  Result := True;
end;

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf64}\VST3\YMYVerb.vst3"
; User presets live in Documents\YMY Records\YMYVerb and are NEVER touched by install/uninstall.
