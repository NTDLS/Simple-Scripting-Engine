<%

;FileAccess:
;	.Read		: Opens a file for read access.
;	.Write		: Opens a file for write access.
;	.ReadWrite	: Opens a file for both read and write access.

var txtFile as File ;Declare the file object.

;Create a file:
If(txtFile.Open("C:\\Test.txt", FileAccess.Write))
	var bytesWritten as Numeric = txtFile.Write("This is the contents of the file!")
	Print("Write: " & bytesWritten & " bytes.")
	Print("File size is " & FormatSize(txtFile.Size(), 2))
	txtFile.Close()
	
	Print("File size is " & FormatSize(File.Size("C:\\Test.txt"), 2))
	
	;Read from an existing file:
	If(txtFile.Open("C:\\Test.txt", FileAccess.Read))
		;txtFile.Seek(5) ;Seek from the current position.
		;txtFile.Seek.FromEnd(-10) ;Seek from the end of the file
		;txtFile.Seek.FromBeginning(12) ;Seek from the beginning of the file
		Print("Read: [" & txtFile.Read() & "]")
		txtFile.Close()
	End If
End If

File.Delete("C:\\Test.txt")

If(Error.Count() > 0)
	Print("File testing failed with " & Error.Count() & " error(s)!")
Else
	Print("File testing was successfull!")
End If

%>