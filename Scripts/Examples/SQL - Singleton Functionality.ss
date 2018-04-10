<%

var sqlConn as SQL.Connection

sqlConn.Connect(".") ;SQL.Connect(Server, [Database], [Username], [Password])

For(iItt as Numeric to 10)
	Print(sqlConn.Value("SELECT GetDate()"))
	Print(sqlConn.Value("SELECT NewID()"))
	Print(FormatNumeric(sqlConn.Value("SELECT CheckSum(NewID())") / 6767.6767, 5))
	Print(sqlConn.Value("SELECT Convert(VarChar, GetDate(), 101)"))
Next

sqlConn.Close()

%>

