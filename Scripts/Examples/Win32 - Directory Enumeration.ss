<%
#Include "Windows.ss"
#Include "Windows.IO.ss"

EnumerateDirectory("C:", True)

Function EnumerateDirectory(path as String, recursive as Numeric)

	var findData as WIN32_FIND_DATA
	var findHandle as Numeric
	var findStatus as Numeric = -1
	findHandle = Windows.FindFirstFile(path & "\\*.*", @findData)
	
	While(findStatus != 0)
		If(!findData.cFileName.Equals(".") and !findData.cFileName.Equals(".."))
		    var fullPath as string = path & "\\" & findData.cFileName
		    
			Print(fullPath)
		    
			If(recursive and (findData.dwFileAttributes BAnd Windows.IO.FILE_ATTRIBUTE_DIRECTORY))
				EnumerateDirectory(path & "\\" & findData.cFileName, recursive)
			End If
		End If
		
		findStatus = Windows.FindNextFile(findHandle, @findData)
	WEnd
	Windows.FindClose(findHandle)

End Function
%>
