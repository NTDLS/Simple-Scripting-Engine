<%
	var timeBefore as Numeric = Sys.TickCount()
	GenerateDocumentation("C:\\Temp")
	Print("Completed in " & FormatNumeric((Sys.TickCount() - timeBefore) / 1000.0, 2) & " seconds.")
%>
