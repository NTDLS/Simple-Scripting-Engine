<%

Try
	Print("Entering Sleeping...")
	Sleep(-1)
	Print("Done Sleeping...")
Catch(ex)
	Print(ex.GetText())
End Try

%>
