<%

Function PrintFile(fileName as string)
	var requestedFile as File
	If(requestedFile.Open(fileName, FileAccess.Read))
		Prints(requestedFile.Read())
		requestedFile.Close()
	End If
End Function

Function WriteMenuLink(text as String, query as String)
	var fullLink as String = gFRONT_PAGE & "?" & query
	Prints("<a class=\"Menu\" href=\"" & fullLink.Replace("//", "/") & "\">" & text & "</a><br />")
End Function

%>
