var sqlConn as SQL.Connection

sqlConn.Connect("DRIVER={ODBC Driver 11 for SQL Server};DATABASE=master;SERVER=(local);Trusted_Connection=yes;")

Print(sqlConn.Value("SELECT GetDate()"))

sqlConn.Close()
