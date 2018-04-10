<%

var Global as string = "Global String in \"" & File.Name(Code.File()) & "\""

Print("File: \"" & File.Name(Code.File()) & "\"")

Function Add(X as Numeric, Y as Numeric) as Numeric
	Return(X + Y)
End Function


%>