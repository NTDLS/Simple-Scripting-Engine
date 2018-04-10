<%

Function CleanRequest(input as String) as String

	var disallowedSingle as string = "~`!@#$%^&*()+="
	var disallowedDouble as string = "."

	While(!input.Equals(Web.URLDecode(input)))
		input = Web.URLDecode(input)
	WEnd
	
	For(i as Numeric = 0 to disallowedSingle.Length())
		If(input.IndexOf(disallowedSingle.SubString(i, 1)) >= 0)
			Return("")
		End If
	Next

	For(i as Numeric = 0 to disallowedDouble.Length())
		var double as String = disallowedDouble.SubString(i, 1)
		If(input.IndexOf(double & double) >= 0)
			Return("")
		End If
	Next
	
	Return(input)
End Function

Print("http://test.com" & CleanRequest("/Folder/%252e%252e/../../../%2e%2e/cgi.*"))
Print("http://test.com" & CleanRequest("/Folder/cgi.exe"))

%>
