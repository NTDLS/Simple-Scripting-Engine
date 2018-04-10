var sqlConn as SQL.Connection
var rsTemp as SQL.RecordSet

sqlConn.Connect("(local)")
sqlConn.Execute("SELECT * FROM sys.objects WHERE type = 'u'", @rsTemp)

While(rsTemp.Fetch())
	Print(rsTemp.Value("name"))
WEnd

rsTemp.Close()
sqlConn.Close()
