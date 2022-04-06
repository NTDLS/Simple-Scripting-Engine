<%
	var timeBefore as Numeric = Sys.TickCount()
	
	
	;Generates HTML help files, Keyword List for auto-completion, types list and HTML Help index file.
	GenerateDocumentation("C:\\Temp")	
	
	Print("Completed in " & FormatNumeric((Sys.TickCount() - timeBefore) / 1000.0, 2) & " seconds.")
%>
