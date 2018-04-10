<% ;--(Code Scope Mismatch Checking)-----------------------------------------
Print("Scope:" Code.Scope())
	Try
		;Sleep(-10)
		Print("Yes->" & "All is well")
	Catch(ex)
		Print("No->" & ex.GetText())
IF(True)
	End Try
	Print("Scope:" Code.Scope())
End If
%>
