<%#Include "Windows.Registry.ss"%>
<%#Include "Windows.Security.ss"%>
<%#Include "Windows.Time.ss"%>
<%#Include "Windows.IO.ss"%>

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
<%
Print("CPU: " & Windows.Registry.GetString(Windows.Registry.HKLM, _
	"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", _
	"ProcessorNameString"))
%>
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
<%

var stTime as SYSTEMTIME

Windows.Time.GetSystemTime(@stTime)

Print(stTime.Month & "/" & stTime.Day & "/" & stTime.Year _
	& " " & stTime.Hour & ":" & stTime.Minute & ":" & stTime.Second)

%>
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
<%

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;MessageBox:
var iVar as numeric = 10

var messageBoxResult as Numeric = Windows.MessageBox(NULL, _
	"This is a variable with a value of: [" & iVar & "].", _
	File.Name(Code.File()), _
	Windows.MB_ICONQUESTION bOR Windows.MB_YESNOCANCEL bOR Windows.MB_DEFBUTTON2)

If(messageBoxResult = Windows.MB_IDYES)
	Windows.MessageBox(NULL, "You hit the YES button", File.Name(Code.File()), Windows.MB_OK)
Else If(messageBoxResult = Windows.MB_IDNO)
	Windows.MessageBox(NULL, "You hit the NO button", File.Name(Code.File()), Windows.MB_OK)
Else If(messageBoxResult = Windows.MB_IDCANCEL)
	Windows.MessageBox(NULL, "You hit the CANCEL button", File.Name(Code.File()), Windows.MB_OK)
End If

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;FindWindow, SetWindowText
Code.Scope.Enter()
var hWindow as Numeric = Windows.FindWindow(@NULL, "Untitled - Notepad")
Print("Handle: " & hWindow)
Print("Set Result: " & Windows.SetWindowText(hWindow, "New Notepad Title"))
Code.Scope.Exit()
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;FindWindow, GetWindowText:
Code.Scope.Enter()
var hWindow as Numeric = Windows.FindWindow(@NULL, "New Notepad Title")
Print("Handle: " & hWindow)
	
var sText as string = Space(100)
Windows.GetWindowText(hWindow, @sText, sText.Length())
Print("Text: " & sText)
Code.Scope.Exit()

%>
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
<%

var bytesWritten as Numeric = Windows.IO.WriteToFile("C:\\Test.txt", "Hello World")
Print("Wrote " & bytesWritten & " bytes.")

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

var someFileName as String = Windows.Registry.GetString(Windows.Registry.HKLM, _
	"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IEXPLORE.EXE", _
	"")

;CreateFile and GetFileSize
var hFile as Numeric = Windows.IO.CreateFile(someFileName, _
	Windows.IO.GENERIC_READ, Windows.IO.FILE_SHARE_READ, NULL, _
	Windows.IO.OPEN_EXISTING, Windows.IO.FILE_ATTRIBUTE_NORMAL, NULL)

If(hFile != Windows.IO.INVALID_HANDLE_VALUE)
	var fileSizeHigh as Numeric
	var fileSizeLow as Numeric = Windows.IO.GetFileSize(hFile, @fileSizeHigh)
	Print("Total: " & FormatSize(Windows.MAKELONGLONG(fileSizeLow, fileSizeHigh), 2))
Else
	print("Failed to open file.")
End If

%>
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
