<html>
	<head>
		<title>SSE Forms</title>
	</head>
	
	<body>

		<%
		If(Form.Value.IsDefined("Submitted"))
			Print("Form.Value.Count: " & Form.Value.Count() & "<br />")
			Print("Form.Value.ByIndex: " & Form.Value.ByIndex(1) & "<br />")
			Print("Form.Value.Index: " & Form.Value.Index("LastName") & "<br />")
			Print("Form.Value.ByName: " & Form.Value.ByName("LastName") & "<br />")
			Print("Form.Value.Name: " & Form.Value.Name(1) & "<br />")
			Print("Form.Value.IsDefined: " & Form.Value.IsDefined("LastName") & "<br />")
			Print("Form.Value.IsDefined: " & Form.Value.IsDefined("Last_Name") & "<br />")
			
			;Display all of the posted form variables.
			For(i as Numeric = 0 to Form.Value.Count())
				print("[" & Form.Value.Name(i) & "]=[" & Form.Value.ByIndex(i) & "]<br />")
			Next
		End If
		%>

		<form name="MainForm" method="post">
			<input type="text" name="Firstname" value="<%=Form.Value.ByName("Firstname")%>"><br />
			<input type="text" name="MiddleInitial" value="<%=Form.Value.ByName("MiddleInitial")%>"><br />
			<input type="text" name="Lastname" value="<%=Form.Value.ByName("Lastname")%>"><br />
			<input type="hidden" name="Submitted" value="Yes"><br />
			<input type="submit" value="Go!"><br />
		</form>
	
	</body>
</html>
