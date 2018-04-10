var txtFile as File

If(txtFile.Open("C:\\TestFileName.txt", FileAccess.Read))
	Print("Read: [" & txtFile.Read() & "]")
	txtFile.Close()
End If
