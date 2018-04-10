var sqlConn as SQL.Connection

sqlConn.Connect("(local)")

Print(sqlConn.Value("SELECT GetDate()"))

sqlConn.Close()
