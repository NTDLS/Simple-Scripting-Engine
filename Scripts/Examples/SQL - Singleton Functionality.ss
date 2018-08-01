<%

var sqlConnection as SQL.Connection

sqlConnection.Connect("DRIVER={ODBC Driver 11 for SQL Server};DATABASE=master;SERVER=(local);Trusted_Connection=yes;")

For(i as Numeric to 10)
	Print(sqlConnection.Value("SELECT GetDate()"))
	Print(sqlConnection.Value("SELECT NewID()"))
	Print(FormatNumeric(sqlConnection.Value("SELECT CheckSum(NewID())") / 6767.6767, 5))
	Print(sqlConnection.Value("SELECT Convert(VarChar, GetDate(), 101)"))
Next

sqlConnection.Close()

%>

