<%
#Include "Windows.ss"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;This error was fixed on 2/25/2016.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

var findData as WIN32_FIND_DATA
var findHandle as Numeric =  Windows.FindFirstFile("C:\\*.*", @findData)
Print(findData.cFileName & "This text will not be displayed because the variable is being stored and printed as a byte array.")
Windows.FindClose(findHandle)

%>
