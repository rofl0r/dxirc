; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{11B40D37-A7C9-4720-8741-EEC36F2D07DB}
AppName=dxirc
AppVerName=dxirc 0.70.svn
AppPublisher=David Vachulka
AppPublisherURL=http://www.dxirc.org
AppSupportURL=http://www.dxirc.org
AppUpdatesURL=http://www.dxirc.org
DefaultDirName={pf}\dxirc
DefaultGroupName=dxirc
LicenseFile=COPYING
OutputBaseFilename=dxirc-0.70.svn-setup
Compression=lzma
SolidCompression=yes

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"
Name: "cs"; MessagesFile: "compiler:Languages\Czech.isl"

[CustomMessages]
en.Program=Program
cs.Program=Program
en.Spell=Spellchecking
cs.Spell=Kontrola pravopisu
en.SpellEn=English
cs.SpellEn=Angli�tina
en.SpellCs=Czech
cs.SpellCs=�e�tina
en.FullInstallation=Full installation
cs.FullInstallation=�pln� instalace
en.CustomInstallation=Custom installation
cs.CustomInstallation=Voliteln� instalace

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0.0,6.0.1

[Types]
Name: "full"; Description: "{cm:FullInstallation}"
Name: "custom"; Description: "{cm:CustomInstallation}"; Flags: iscustom

[Components]
Name: "program"; Description: "{cm:Program}"; Types: full custom; Flags: fixed
Name: "spell"; Description: "{cm:Spell}"; Types: full
Name: "spell\en"; Description: "{cm:SpellEn}"; Types: full
Name: "spell\cs"; Description: "{cm:SpellCs}"; Types: full

[Files]
Source: "dxirc.exe"; DestDir: "{app}"; Components: program; Flags: ignoreversion
Source: "*.dll"; DestDir: "{app}"; Components: program; Flags: ignoreversion
Source: "icons\*.png"; DestDir: "{app}\icons"; Components: program; Flags: recursesubdirs
Source: "icons\smileys\dxirc.smiley"; DestDir: "{app}\icons\smileys"; Components: program; Flags: ignoreversion
Source: "locale\*.mo"; DestDir: "{app}\locale"; Components: program; Flags: recursesubdirs
Source: "sounds\*.wav"; DestDir: "{app}\sounds"; Components: program; Flags: recursesubdirs
; Spellcheking (enchant)
Source: "lib\*.*"; DestDir: "{app}\lib"; Components: program; Flags: recursesubdirs
Source: "share\enchant\myspell\*en_US*"; DestDir: "{app}\share\enchant\myspell"; Components: spell\en
Source: "share\enchant\myspell\*cs_CZ*"; DestDir: "{app}\share\enchant\myspell"; Components: spell\cs
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\dxirc"; Filename: "{app}\dxirc.exe"
Name: "{group}\{cm:ProgramOnTheWeb,dxirc}"; Filename: "http://www.dxirc.org/"
Name: "{group}\{cm:UninstallProgram,dxirc}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\dxirc"; Filename: "{app}\dxirc.exe"; Tasks: desktopicon; WorkingDir: {app}
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\dxirc"; Filename: "{app}\dxirc.exe"; Tasks: quicklaunchicon; WorkingDir: {app}

[Run]
Filename: "{app}\dxirc.exe"; Description: "{cm:LaunchProgram,dxirc}"; Flags: nowait postinstall skipifsilent





