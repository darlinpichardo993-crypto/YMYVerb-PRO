; Inno Setup script. Build:  iscc /DVST3Dir="C:\path\to\YMYVerb.vst3" Installer\YMYVerb.iss
#define MyAppName "YMYVerb"
#define MyAppVersion "1.0.0"
#ifndef VST3Dir
  #define VST3Dir "..\build\YMYVerb_artefacts\Release\VST3\YMYVerb.vst3"
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
Name: "docs"; Description: "Instalar la documentacion (README)"; Flags: checkedonce

[Files]
; The VST3 goes to the standard 64-bit VST3 folder (Program Files\Common Files\VST3).
Source: "{#VST3Dir}\*"; DestDir: "{commoncf64}\VST3\YMYVerb.vst3"; Flags: recursesubdirs createallsubdirs ignoreversion
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion; Tasks: docs

[Code]
function InitializeSetup(): Boolean;
begin
  Result := True;
end;

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf64}\VST3\YMYVerb.vst3"
; User presets live in Documents\YMY Records\YMYVerb and are NEVER touched by install/uninstall.
