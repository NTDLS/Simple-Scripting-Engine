[Setup]
;-- Main Setup Information
 AppName                = Simple Scripting Engine
 AppVerName             = Simple Scripting Engine 1.0.7.1
 AppCopyright           = Copyright © 1995-2016 NetworkDLS.
 DefaultDirName         = {pf}\NetworkDLS\Simple Scripting Engine
 DefaultGroupName       = NetworkDLS\Simple Scripting Engine
 UninstallDisplayIcon   = {app}\SSE.Exe
 PrivilegesRequired     = PowerUser
 Uninstallable          = Yes
 MinVersion             = 0.0,5.0
 Compression            = bZIP/9
 ChangesAssociations    = Yes
 OutputBaseFilename     = SSE

;-- Windows 2000 & XP (Support Dialog)
 AppPublisher    = NetworkDLS
 AppPublisherURL = http://www.NetworkDLS.com/
 AppUpdatesURL   = http://www.NetworkDLS.com/
 AppVersion      = 1.0.7.1
 
[Components]
 Name: "SSE";        Description: "Scripting Engine";             Types: Full Compact Custom; Flags: Fixed;
 Name: "SSIDE";      Description: "Development Environment";      Types: Full Custom; MinVersion: 0.0,5.1;
 Name: "Examples";   Description: "Example Scripts";              Types: Full Custom;
 Name: "Library";    Description: "Standard Abstraction Library"; Types: Full Compact Custom;
 Name: "Snippets";   Description: "Snippets";                     Types: Full Custom;

[Tasks]
 Name: FileAssociation;         Description: "Create File Association"; GroupDescription: "File Association and Default Action:"; Components: SSE;
 Name: FileAssociation\Edit;    Description: "Edit script";             GroupDescription: "File Association and Default Action:"; Components: SSE; Flags: exclusive
 Name: FileAssociation\Execute; Description: "Execute script";          GroupDescription: "File Association and Default Action:"; Components: SSE; Flags: exclusive unchecked
 
[Files]
 Source: "..\Scripts\Examples\*.*";                        DestDir: "{app}\IDE\Examples\";        Flags: IgnoreVersion recursesubdirs; Components: Examples;
 Source: "..\Scripts\Snippets\*.*";                        DestDir: "{app}\IDE\Snippets";         Flags: IgnoreVersion recursesubdirs; Components: Snippets;
 Source: "..\Scripts\Library\*.*";                         DestDir: "{app}\Library";              Flags: IgnoreVersion recursesubdirs; Components: Library;
 Source: "Libraries\EventMsg.dll";                         DestDir: "{app}";                      Flags: IgnoreVersion;                Components: SSE;
 Source: "..\Source\Engine\Release\Win32\SSE.exe";         DestDir: "{app}";                      Flags: IgnoreVersion;                Components: SSE;
 Source: "..\Source\IDE\bin\Release\SSIDE.exe";            DestDir: "{app}\IDE";                  Flags: IgnoreVersion;                Components: SSIDE;
 Source: "..\Source\IDE\bin\Release\*.dll";                DestDir: "{app}\IDE";                  Flags: IgnoreVersion;                Components: SSIDE;
 Source: "HTML Code Images\*.*";                           DestDir: "{app}\IDE\HTML Code Images"; Flags: IgnoreVersion;                Components: SSIDE;
 Source: "..\Source\IDE\Highlighters\*.*";                 DestDir: "{app}\IDE\Highlighters";     Flags: IgnoreVersion;                Components: SSIDE;
 Source: "..\Documentation\Help.chm";                      DestDir: "{app}\Help";                 Flags: IgnoreVersion;                Components: SSIDE;
 Source: "..\Documentation\AutoComplete.json";             DestDir: "{app}\Help";                 Flags: IgnoreVersion;                Components: SSIDE;

[Icons]
 Name: "{group}\Simple Script IDE";          Filename: "{app}\IDE\SSIDE.exe";            Components: SSIDE;
 Name: "{group}\Simple Script Help";         Filename: "{app}\Help\Help.chm";            Components: SSIDE;
 Name: "{commondesktop}\Simple Script IDE";  Filename: "{app}\IDE\SSIDE.exe";            Components: SSIDE; Tasks: DesktopIcon\Common;
 Name: "{userdesktop}\Simple Script IDE";    Filename: "{app}\IDE\SSIDE.exe";            Components: SSIDE; Tasks: DesktopIcon\User;

[Registry]
;-- Create SS "Simple Script" Association
 Root: HKCR; Subkey: ".ss"; Flags: uninsdeletekey; Tasks: FileAssociation;
 Root: HKCR; Subkey: ".ss"; ValueType: string; ValueData: "SimpleScript"; Tasks: FileAssociation;
 Root: HKCR; Subkey: "SimpleScript"; Flags: uninsdeletekey; Tasks: FileAssociation;
 Root: HKCR; Subkey: "SimpleScript"; ValueType: string; ValueData:"Simple Script"; Tasks: FileAssociation;
 Root: HKCR; Subkey: "SimpleScript\Shell"; ValueType: string; ValueData:"Edit"; Tasks: FileAssociation\Edit;
 Root: HKCR; Subkey: "SimpleScript\Shell"; ValueType: string; ValueData:"Execute"; Tasks: FileAssociation\Execute;
 
 Root: HKCR; Subkey: "SimpleScript\Shell\Execute\Command"; Flags: uninsdeletekey; Tasks: FileAssociation;
 Root: HKCR; Subkey: "SimpleScript\Shell\Execute\Command"; ValueType: string; ValueData:"""{app}\SSE.Exe"" ""%1"""; Tasks: FileAssociation;
 
 Root: HKCR; Subkey: "SimpleScript\Shell\Edit\Command"; Flags: uninsdeletekey; Components: SSIDE; Tasks: FileAssociation;
 Root: HKCR; Subkey: "SimpleScript\Shell\Edit\Command"; ValueType: string;  ValueData:"""{app}\IDE\SSIDE.exe"" ""%1"""; Components: SSIDE; Tasks: FileAssociation;

 Root: HKCR; Subkey: "SimpleScript\Shell\Execute then Pause\Command"; Flags: uninsdeletekey; Tasks: FileAssociation;
 Root: HKCR; Subkey: "SimpleScript\Shell\Execute then Pause\Command"; ValueType: string; ValueData:"""{app}\SSE.Exe"" ""%1"" /Pause"; Tasks: FileAssociation;

 Root: HKCR; Subkey: "SimpleScript\Shell\Execute with Debugging\Command"; Flags: uninsdeletekey; Tasks: FileAssociation;
 Root: HKCR; Subkey: "SimpleScript\Shell\Execute with Debugging\Command"; ValueType: string; ValueData:"""{app}\SSE.Exe"" ""%1"" /Debug /Pause"; Tasks: FileAssociation;

 Root: HKCR; Subkey: "SimpleScript\DefaultIcon"; Flags: uninsdeletekey; Tasks: FileAssociation;
 Root: HKCR; Subkey: "SimpleScript\DefaultIcon"; ValueType: string; ValueData:"{app}\SSE.Exe,0"; Tasks: FileAssociation;

;-- Create SS "Simple Script Project" Association
 Root: HKCR; Subkey: ".ssepx"; Flags: uninsdeletekey; Tasks: FileAssociation;
 Root: HKCR; Subkey: ".ssepx"; ValueType: string; ValueData: "SimpleScriptEngineProject"; Tasks: FileAssociation;
 Root: HKCR; Subkey: "SimpleScriptEngineProject"; Flags: uninsdeletekey; Tasks: FileAssociation;
 Root: HKCR; Subkey: "SimpleScriptEngineProject"; ValueType: string; ValueData:"Simple Scripting Engine Project"; Tasks: FileAssociation;
 Root: HKCR; Subkey: "SimpleScriptEngineProject\Shell"; ValueType: string; ValueData:"Open"; Tasks: FileAssociation\Edit;
 
 Root: HKCR; Subkey: "SimpleScriptEngineProject\Shell\Open\Command"; Flags: uninsdeletekey; Components: SSIDE; Tasks: FileAssociation;
 Root: HKCR; Subkey: "SimpleScriptEngineProject\Shell\Open\Command"; ValueType: string;  ValueData:"""{app}\IDE\SSIDE.exe"" ""%1"""; Components: SSIDE; Tasks: FileAssociation;
 
 Root: HKCR; Subkey: "SimpleScriptEngineProject\DefaultIcon"; Flags: uninsdeletekey; Tasks: FileAssociation;
 Root: HKCR; Subkey: "SimpleScriptEngineProject\DefaultIcon"; ValueType: string; ValueData:"{app}\SSE.Exe,1"; Tasks: FileAssociation;

;-- 
 Root: HKLM; Subkey: "Software\NetworkDLS\Simple Scripting Engine\IDE";         ValueType: string; ValueName:LastExportImagesURI;              ValueData:"/images/";            Components: SSIDE; Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\Simple Scripting Engine\IDE\Options"; ValueType: dword;  ValueName:BracketMatching;                  ValueData:"1";                   Components: SSIDE; Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\Simple Scripting Engine\IDE\Options"; ValueType: dword;  ValueName:EnableAutoComplete;               ValueData:"1";                   Components: SSIDE; Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\Simple Scripting Engine\IDE\Options"; ValueType: dword;  ValueName:ShowGutterMargin;                 ValueData:"1";                   Components: SSIDE; Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\Simple Scripting Engine\IDE\Options"; ValueType: dword;  ValueName:ShowEOLMarker;                    ValueData:"0";                   Components: SSIDE; Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\Simple Scripting Engine\IDE\Options"; ValueType: dword;  ValueName:ShowWhitespace;                   ValueData:"0";                   Components: SSIDE; Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\Simple Scripting Engine\IDE\Options"; ValueType: dword;  ValueName:ShowLineNumbers;                  ValueData:"1";                   Components: SSIDE; Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\Simple Scripting Engine\IDE\Options"; ValueType: dword;  ValueName:ShowScopeIndicator;               ValueData:"1";                   Components: SSIDE; Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\Simple Scripting Engine\IDE\Options"; ValueType: dword;  ValueName:AutoCompleteComplexExpressions;   ValueData:"1";                   Components: SSIDE; Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\Simple Scripting Engine\IDE\Options"; ValueType: dword;  ValueName:AutoCompleteMatchOnBeginningOnly; ValueData:"1";                   Components: SSIDE; Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\Simple Scripting Engine\IDE\Options"; ValueType: dword;  ValueName:AutoCompleteSimpleExpressions;    ValueData:"1";                   Components: SSIDE; Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\Simple Scripting Engine\IDE\Options"; ValueType: dword;  ValueName:DebugShowConsole;                 ValueData:"0";                   Components: SSIDE; Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\Simple Scripting Engine\IDE\Options"; ValueType: dword;  ValueName:EnableCodeFolding;                ValueData:"1";                   Components: SSIDE; Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\Simple Scripting Engine\IDE\Options"; ValueType: string; ValueName:DefaultText;                      ValueData:"{code:DefaultText}";  Components: SSIDE; Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\Simple Scripting Engine\IDE\Options"; ValueType: dword;  ValueName:DefaultCaretX;                    ValueData:"0";                   Components: SSIDE; Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\Simple Scripting Engine\IDE\Options"; ValueType: dword;  ValueName:DefaultCaretY;                    ValueData:"2";                   Components: SSIDE; Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\Simple Scripting Engine\IDE\Help";                                                                                                    Components: SSIDE; Flags: CreateValueIfDoesntExist;

 Root: HKLM; Subkey: "Software\NetworkDLS\Simple Scripting Engine";                                                                     Flags: uninsdeletekey noerror;
 Root: HKLM; Subkey: "Software\NetworkDLS\Simple Scripting Engine\"; ValueType: string; ValueName: "Path";   ValueData:"{app}";         Flags: CreateValueIfDoesntExist;
 Root: HKLM; Subkey: "Software\NetworkDLS\Simple Scripting Engine\"; ValueType: string; ValueName: "Engine"; ValueData:"{app}\SSE.exe"; Flags: CreateValueIfDoesntExist;

[Tasks]
 Name: DesktopIcon;        Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"; Components: SSIDE
 Name: DesktopIcon\Common; Description: "For all users"; GroupDescription: "Additional icons:"; Components: SSIDE; Flags: exclusive
 Name: DesktopIcon\User;   Description: "For the current user only"; GroupDescription: "Additional icons:"; Components: SSIDE; Flags: exclusive unchecked

[Run]
 Filename: "{app}\IDE\SSIDE.exe"; Description: "Run IDE now?"; Flags: postinstall nowait skipifsilent shellexec; Components: SSIDE;

[Code]
function DefaultText(Default: String): String;
begin
Result := '<%'#13#10#13#10#13#10#13#10'%>';
end;
