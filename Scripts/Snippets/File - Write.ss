var txtFile as File

If(txtFile.Open("C:\\TestFileName.txt", FileAccess.Write))
	txtFile.Write("...")
	txtFile.Close()
End If

DeleteFile("C:\\TestFileName.txt")
