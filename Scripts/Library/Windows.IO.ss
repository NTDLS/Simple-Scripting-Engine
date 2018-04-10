<%;Preprocessors

#Namespace "Windows.IO"
#Include "Windows.ss"

%>
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
<%;Definitions

;;; File operation error codes ;;;
#define INVALID_HANDLE_VALUE				0xFFFFFFFF
#define INVALID_FILE_SIZE					0xFFFFFFFF
#define INVALID_SET_FILE_POINTER			0xFFFFFFFF
#define INVALID_FILE_ATTRIBUTES				0xFFFFFFFF

;;; File access ;;;
#define GENERIC_READ						Windows.GENERIC_READ
#define GENERIC_WRITE						Windows.GENERIC_WRITE
#define GENERIC_EXECUTE						Windows.GENERIC_EXECUTE
#define GENERIC_ALL							Windows.GENERIC_ALL

;;; File share modes ;;;
#define FILE_SHARE_READ						0x00000001  
#define FILE_SHARE_WRITE					0x00000002  
#define FILE_SHARE_DELETE					0x00000004

;;; File creation dispositions ;;;
#define CREATE_NEW							1
#define CREATE_ALWAYS						2
#define OPEN_EXISTING						3
#define OPEN_ALWAYS							4
#define TRUNCATE_EXISTING					5

;;; File attributes ;;;
#define FILE_ATTRIBUTE_READONLY				0x00000001  
#define FILE_ATTRIBUTE_HIDDEN				0x00000002  
#define FILE_ATTRIBUTE_SYSTEM				0x00000004  
#define FILE_ATTRIBUTE_DIRECTORY			0x00000010  
#define FILE_ATTRIBUTE_ARCHIVE				0x00000020  
#define FILE_ATTRIBUTE_DEVICE				0x00000040  
#define FILE_ATTRIBUTE_NORMAL				0x00000080  
#define FILE_ATTRIBUTE_TEMPORARY			0x00000100  
#define FILE_ATTRIBUTE_SPARSE_FILE			0x00000200  
#define FILE_ATTRIBUTE_REPARSE_POINT		0x00000400  
#define FILE_ATTRIBUTE_COMPRESSED			0x00000800  
#define FILE_ATTRIBUTE_OFFLINE				0x00001000  
#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED	0x00002000  
#define FILE_ATTRIBUTE_ENCRYPTED			0x00004000  
#define FILE_ATTRIBUTE_VIRTUAL				0x00010000  

%>
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
<%;External Prototypes

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Declare [Function Name]
;	Lib		: The DLL or library containing the executable code
;	Alias	: (Not required) The name of the function within the "Lib". If not specified
;				the engine will default to the function name.
;	Type 	: (Not required) The calling convention: StdCall, CDECL or FastCall. <Default: StdCall>
;(Parameter lists) as [Return Type].
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

Declare ReadFile Lib "kernel32" _
	(ByVal hFile As StrictType.Handle, _
	ByVal lpBuffer As StrictType.String, _
	ByVal nNumberOfBytesToRead As StrictType.uInt32, _
	ByRef lpNumberOfBytesRead As StrictType.uInt32, _
	ByVal lpOverlapped As StrictType.Int32) As StrictType.Boolean

Declare WriteFile Lib "kernel32" _
	(ByVal hFile As StrictType.Handle, _
	ByVal lpBuffer As StrictType.String, _
	ByVal nNumberOfBytesToWrite As StrictType.uInt32, _
	ByRef lpNumberOfBytesWritten As StrictType.uInt32, _
	ByVal lpOverlapped As StrictType.Int32) As StrictType.Boolean

Declare CreateFile Lib "kernel32.dll" Alias "CreateFileA"_
	(ByVal lpFileName As StrictType.String, _
	ByVal dwDesiredAccess As StrictType.uInt32, _
	ByVal dwShareMode As StrictType.uInt32, _
	ByVal lpSecurityAttributes As StrictType.Int32, _
	ByVal dwCreationDisposition As StrictType.uInt32, _
	ByVal dwFlagsAndAttributes As StrictType.uInt32, _
	ByVal hTemplateFile As StrictType.Handle) As StrictType.Handle

Declare CopyFile Lib "kernel32" Alias "CopyFileA" _
	(ExistingFileName as StrictType.String, _
	NewFileName as StrictType.String, _
	bFailIfExists as StrictType.Boolean) as StrictType.Boolean

Declare MoveFile Lib "kernel32" Alias "MoveFileA" _
	(ExistingFileName as StrictType.String, _
	NewFileName as StrictType.String) as StrictType.Boolean

Declare GetFileSize Lib "kernel32" (hFile As StrictType.Handle, _
	ByRef lpFileSizeHigh As StrictType.uInt32) As StrictType.uInt32

%>
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
<%;Abstractions

Function WriteToFile(fileName as String, text as String) as Numeric

	var hFile as Numeric = Windows.IO.CreateFile(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)
	
	var bytesWritten as Numeric = 0
	Windows.IO.WriteFile(hFile, text, text.Length(), @bytesWritten, null)
	
	Windows.CloseHandle(hFile)

	return(bytesWritten)

End Function

%>
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
<%;Example
;
;var bytesWritten as Numeric = Windows.IO.WriteToFile("C:\\Test.txt", "Hello World")
;Print("Wrote " & bytesWritten & " bytes.")
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;;CreateFile and GetFileSize
;var hFile as Numeric = Windows.IO.CreateFile("C:\\LargeFile.7z", _
;	GENERIC_READ, FILE_SHARE_READ, NULL, _
;	OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)
;
;If(hFile != INVALID_HANDLE_VALUE)
;	var fileSizeHigh as Numeric
;	var fileSizeLow as Numeric = Windows.IO.GetFileSize(hFile, @fileSizeHigh)
;	Print("Total: " & FormatSize(MAKELONGLONG(fileSizeLow, fileSizeHigh), 5))
;
;	Windows.CloseHandle(hFile)
;Else
;	print("Failed to open file.")
;End If
;
%>
