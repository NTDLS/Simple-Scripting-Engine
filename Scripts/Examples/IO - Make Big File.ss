<%
Function MakeBigFile(fileName as String, megaBytes as Numeric)

	var bigFile as File
	
	bigFile.Open(fileName, FileAccess.Write, CreationDisposition.AlwaysCreate)
	
	var buffer as String = Space(1024 * 1024)
	
	For(i as Numeric = 0 to megaBytes)
		bigFile.Write(buffer)
	Next
	
	bigFile.Close()
	
End Function
%>

<%

Var fileName as String = "C:\\Bigfile.txt"

MakeBigFile(fileName, 10)

Print("Final file size: " & FormatSize(File.Size(fileName), 2))

File.Delete(fileName)

%>