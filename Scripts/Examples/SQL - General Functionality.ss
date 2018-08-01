<%

var sqlConnection as SQL.Connection
var sqlRecord as SQL.RecordSet

sqlConnection.Connect("DRIVER={ODBC Driver 11 for SQL Server};DATABASE=master;SERVER=(local);Trusted_Connection=yes;")

sqlConnection.Execute("SELECT * FROM sys.objects WHERE type = 'u'", @sqlRecord)

While(sqlRecord.Fetch())
	Print("(" & FormatNumeric(sqlRecord.Value("object_id")) _
		& ") - [" & sqlRecord.Value("name") & "]" _
		& " was created on \"" & sqlRecord.Value("create_date") & "\"")
WEnd

sqlRecord.Close()
sqlConnection.Close()

%>
