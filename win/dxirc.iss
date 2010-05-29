; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{11B40D37-A7C9-4720-8741-EEC36F2D07DB}
AppName=dxirc
AppVerName=dxirc 0.50.0
AppPublisher=David Vachulka
AppPublisherURL=http://www.dxirc.org
AppSupportURL=http://www.dxirc.org
AppUpdatesURL=http://www.dxirc.org
DefaultDirName={pf}\dxirc
DefaultGroupName=dxirc
LicenseFile=COPYING
OutputBaseFilename=dxirc-0.50.0-setup
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "czech"; MessagesFile: "compiler:Languages\Czech.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0.0,6.0.1

[Files]
Source: "dxirc.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "*.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "icons\*.png"; DestDir: "{app}\icons"; Flags: recursesubdirs
Source: "icons\smileys\dxirc.smiley"; DestDir: "{app}\icons\smileys"; Flags: ignoreversion
Source: "locale\*.mo"; DestDir: "{app}\locale"; Flags: recursesubdirs
Source: "sounds\*.wav"; DestDir: "{app}\sounds"; Flags: recursesubdirs
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\dxirc"; Filename: "{app}\dxirc.exe"
Name: "{group}\{cm:ProgramOnTheWeb,dxirc}"; Filename: "http://www.dxirc.org/"
Name: "{group}\{cm:UninstallProgram,dxirc}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\dxirc"; Filename: "{app}\dxirc.exe"; Tasks: desktopicon; WorkingDir: {app}
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\dxirc"; Filename: "{app}\dxirc.exe"; Tasks: quicklaunchicon; WorkingDir: {app}

[Run]
Filename: "{app}\dxirc.exe"; Description: "{cm:LaunchProgram,dxirc}"; Flags: nowait postinstall skipifsilent


