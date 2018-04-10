<%;Preprocessors

#Namespace "Windows"

%>
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
<%;Definitions

;;; Generic definitions ;;;
#define ERROR_SUCCESS					0

;;; Predefined standard access types ;;;
#define DELETE						0x00010000
#define READ_CONTROL				0x00020000
#define WRITE_DAC					0x00040000
#define WRITE_OWNER					0x00080000
#define SYNCHRONIZE					0x00100000

#define STANDARD_RIGHTS_REQUIRED	0x000F0000
#define STANDARD_RIGHTS_READ		READ_CONTROL
#define STANDARD_RIGHTS_WRITE		READ_CONTROL
#define STANDARD_RIGHTS_EXECUTE		READ_CONTROL
#define STANDARD_RIGHTS_ALL			0x001F0000
#define SPECIFIC_RIGHTS_ALL			0x0000FFFF

;;; Generic rights ;;;
#define GENERIC_READ				0x80000000L
#define GENERIC_WRITE				0x40000000L
#define GENERIC_EXECUTE				0x20000000L
#define GENERIC_ALL					0x10000000L

;;;MessageBox() Flags ;;;
#define MB_OK                       0x00000000L
#define MB_OKCANCEL                 0x00000001L
#define MB_ABORTRETRYIGNORE         0x00000002L
#define MB_YESNOCANCEL              0x00000003L
#define MB_YESNO                    0x00000004L
#define MB_RETRYCANCEL              0x00000005L
#define MB_CANCELTRYCONTINUE        0x00000006L

#define MB_ICONHAND                 0x00000010L
#define MB_ICONQUESTION             0x00000020L
#define MB_ICONEXCLAMATION          0x00000030L
#define MB_ICONASTERISK             0x00000040L
#define MB_USERICON                 0x00000080L
#define MB_ICONWARNING              MB_ICONEXCLAMATION
#define MB_ICONERROR                MB_ICONHAND
#define MB_ICONINFORMATION          MB_ICONASTERISK
#define MB_ICONSTOP                 MB_ICONHAND

#define MB_DEFBUTTON1               0x00000000L
#define MB_DEFBUTTON2               0x00000100L
#define MB_DEFBUTTON3               0x00000200L
#define MB_DEFBUTTON4               0x00000300L

#define MB_APPLMODAL                0x00000000L
#define MB_SYSTEMMODAL              0x00001000L
#define MB_TASKMODAL                0x00002000L
#define MB_HELP                     0x00004000L
#define MB_NOFOCUS                  0x00008000L
#define MB_SETFOREGROUND            0x00010000L
#define MB_DEFAULT_DESKTOP_ONLY     0x00020000L

#define MB_TOPMOST                  0x00040000L
#define MB_RIGHT                    0x00080000L
#define MB_RTLREADING               0x00100000L

#define MB_TYPEMASK                 0x0000000FL
#define MB_ICONMASK                 0x000000F0L
#define MB_DEFMASK                  0x00000F00L
#define MB_MODEMASK                 0x00003000L
#define MB_MISCMASK                 0x0000C000L

;;; MessageBox() Return Values ;;;
#define MB_IDOK						1
#define MB_IDCANCEL					2
#define MB_IDABORT					3
#define MB_IDRETRY					4
#define MB_IDIGNORE					5
#define MB_IDYES					6
#define MB_IDNO						7
#define MB_IDCLOSE					8
#define MB_IDHELP					9
#define MB_IDTRYAGAIN				10
#define MB_IDCONTINUE				11

;;; GlobalAlloc Flags ;;;
#define GMEM_FIXED					0x0000
#define GMEM_MOVEABLE				0x0002
#define GHND						0x0042
#define GMEM_ZEROINIT				0x0040
#define GPTR						0x0040
%>
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
<%;Types

StrictType FILETIME
	dwLowDateTime As StrictType.Long
	dwHighDateTime As StrictType.Long
End StrictType

StrictType WIN32_FIND_DATA
	dwFileAttributes As StrictType.Long
	ftCreationTime As FILETIME
	ftLastAccessTime As FILETIME
	ftLastWriteTime As FILETIME
	nFileSizeHigh As StrictType.Long
	nFileSizeLow As StrictType.Long
	dwReserved0 As StrictType.Long
	dwReserved1 As StrictType.Long
	cFileName As StrictType.String(260)
	cAlternate As StrictType.String(14)
End StrictType

%>
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
<%;External Prototypes

Declare GetLastError Lib "kernel32" Alias "GetLastError" () As StrictType.uInt32

;Declare FormatMessage Lib "kernel32" _ Alias "FormatMessageA" _
;	(ByVal dwFlags As Long, _
;	lpSource As Any, _
;	ByVal dwMessageId As Long, _
;	ByVal dwLanguageId As Long, _
;	ByVal lpBuffer As String, _
;	ByVal nSize As Long, _
;	Arguments As Long) As Long

Declare Beep Lib "kernel32" _
	(Frequency as StrictType.Int, _
	Duration as StrictType.Int) as StrictType.Int

Declare MessageBox Lib "user32" Alias "MessageBoxA" _
	(ByVal hOwner As StrictType.Int32, _
	ByVal sText As StrictType.String, _
	ByVal sCaption As StrictType.String, _
	ByVal Flags As StrictType.Int32) as StrictType.Int32
	
Declare FindWindow Lib "user32" Alias "FindWindowA" _
	(byref sClassName as StrictType.String, _
	ByVal WindowName as StrictType.String) as StrictType.Handle
	
Declare SetWindowText Lib "user32" Alias "SetWindowTextA" _
	(ByVal hWnd as StrictType.Handle, _
	ByVal sString as StrictType.String) as StrictType.Boolean
	
Declare GetWindowText Lib "user32" Alias "GetWindowTextA" _
	(hWnd as StrictType.Handle, _
	ByRef sOutString as StrictType.String, _
	ByVal sMaxOutSize as StrictType.Int32) as StrictType.Boolean
	
Declare GetWindowTextLength Lib "user32" Alias "GetWindowTextLengthA" _
	(hWnd as StrictType.Handle) as StrictType.Int32

Declare FindFirstFile Lib "kernel32.dll" Alias "FindFirstFileA" _
	(lpFileName As StrictType.String, ByRef lpFindFileData As WIN32_FIND_DATA) As StrictType.Long

Declare FindNextFile Lib "kernel32.dll" Alias "FindNextFileA" _
	(hFindFile As StrictType.Long, ByRef lpFindFileData As WIN32_FIND_DATA) As StrictType.Long 

Declare FindClose Lib "kernel32.dll" _
	(hFindFile As StrictType.Long) As StrictType.Long

Declare CloseHandle Lib "kernel32" _
	(ByVal hObject As StrictType.Handle) As StrictType.Boolean

Declare CopyMemory Lib "kernel32" Alias "RtlMoveMemory" _
	(ByVal Destination As StrictType.Long, _
	ByVal Source As StrictType.String, _
	ByVal Length As StrictType.Long)

Declare GlobalAlloc Lib "kernel32" _
	(ByVal wFlags As StrictType.Long, _
	ByVal dwBytes As StrictType.Long) As StrictType.Long

Declare GlobalFree Lib "kernel32" _
	(ByVal hMem As StrictType.Long) As StrictType.Long

%>
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
<%;Abstractions

Function MAKELONGLONG(loPart as Numeric, hiPart as Numeric) as Numeric
	Return((hiPart * Limits.uInt32.Max) + loPart)
End Function

%>
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
<%;Example:
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;MessageBox:
;var iVar as numeric = 10
;var messageBoxResult as Numeric = MessageBox(NULL, _
;	"This is a variable with a value of: [" & iVar & "].", _
;	File.Name(Code.File()), _
;	MB_ICONQUESTION bOR MB_YESNOCANCEL bOR MB_DEFBUTTON2)
;
;If(messageBoxResult = MB_IDYES)
;	MessageBox(NULL, "You hit the YES button", File.Name(Code.File()), MB_OK)
;Else If(messageBoxResult = MB_IDNO)
;	MessageBox(NULL, "You hit the NO button", File.Name(Code.File()), MB_OK)
;Else If(messageBoxResult = MB_IDCANCEL)
;	MessageBox(NULL, "You hit the CANCEL button", File.Name(Code.File()), MB_OK)
;End If
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;FindWindow, SetWindowText
;var hWindow as Numeric = FindWindow(@NULL, "Untitled - Notepad")
;Print("Handle: " & hWindow)
;Print("Set Result: " & SetWindowText(hWindow, "New Notepad Title"))
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;FindWindow, GetWindowText:
;var hWindow as Numeric = FindWindow(@NULL, "Untitled - Notepad")
;Print("Handle: " & hWindow)
;	
;var sText as string = Space(100)
;GetWindowText(hWindow, @sText, sText.Length())
;Print("Text: " & sText)
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;FindFirstFile, FindNextFile, FindClose:
;var findData as WIN32_FIND_DATA
;var findHandle as Numeric
;var findStatus as Numeric = -1
;findHandle = FindFirstFile("Pages\\*.html", @findData)
;
;While(findStatus != 0)
;	Print(findData.cFileName)
;	findStatus = FindNextFile(findHandle, @findData)
;WEnd
;
;FindClose(findHandle)
;
%>
