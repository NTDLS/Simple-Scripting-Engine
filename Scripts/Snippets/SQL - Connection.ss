var sqlConn as SQL.Connection
var rsTemp as SQL.RecordSet

sqlConn.Connect("DRIVER={ODBC Driver 11 for SQL Server};DATABASE=master;SERVER=(local);Trusted_Connection=yes;")
sqlConn.Execute("SELECT * FROM sys.objects WHERE type = 'u'", @rsTemp)

While(rsTemp.Fetch())
	Print(rsTemp.Value("name"))
WEnd

rsTemp.Close()
sqlConn.Close()
