Try

Catch(ex)
	Print("Error: [" & ex.GetText() & "]" & CrLf & Tab _
		& "in [" & ex.GetFile() & "] on line " & ex.GetLine())		
End Try
