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
	var sqlConnection as SQL.Connection
	var sqlRecord as SQL.RecordSet

	sqlConnection.Connect("DRIVER={ODBC Driver 11 for SQL Server};DATABASE=" & databaseName & ";SERVER=(local);Trusted_Connection=yes;")
	
	sqlConnection.Execute(text, @sqlRecord)
	
	While(sqlRecord.Fetch())		
		values.Add(sqlRecord.Value(0))
	WEnd

	sqlRecord.Close()
	sqlConnection.Close()

	Return(values)
	
End Function
%>
