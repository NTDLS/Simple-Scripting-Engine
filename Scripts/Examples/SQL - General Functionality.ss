<%

var sqlConn as SQL.Connection
var recordSet as SQL.RecordSet

sqlConn.Connect(".")

sqlConn.Execute("SELECT * FROM sys.objects WHERE type = 'u'", @recordSet)

While(recordSet.Fetch())
	Print("(" & FormatNumeric(recordSet.Value("object_id")) _
		& ") - [" & recordSet.Value("name") & "]" _
		& " was created on \"" & recordSet.Value("create_date") & "\"")
WEnd

recordSet.Close()
sqlConn.Close()

%>
