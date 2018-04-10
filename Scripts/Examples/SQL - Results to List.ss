<%
#debug
%>

<%
var sqlText as string = "SELECT  FirstName + ' ' + LastName as FullName FROM [Person].[Person] ORDER BY FirstName + ' ' + LastName"

var values as List = GetSQLValues(".", "AdventureWorks", sqlText)

For(i as Numeric = 0 to values.Count())
	Print(i & " of " & values.Count() - 1 & ": " & values[i])
Next
%>

<%
Function GetSQLValues(serverName as String, databaseName as String, text as String) as List

	var values as List
	var sqlConn as SQL.Connection
	var recordSet as SQL.RecordSet

	sqlConn.Connect(serverName, databaseName)
	
	sqlConn.Execute(text, @recordSet)
	
	While(recordSet.Fetch())		
		values.Add(recordSet.Value(0))
	WEnd

	recordSet.Close()
	sqlConn.Close()

	Return(values)
	
End Function
%>
