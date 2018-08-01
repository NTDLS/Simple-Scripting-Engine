<%

var sqlConnection as SQL.Connection
var sqlRcord as SQL.RecordSet

sqlConnection.Connect("DRIVER={ODBC Driver 11 for SQL Server};DATABASE=master;SERVER=(local);Trusted_Connection=yes;")

sqlConnection.Execute("SELECT TOP 10 id, name, xtype FROM sysobjects", @sqlRcord)

print("Index of Name A: " sqlRcord.Columns.Index("name"))
print("Name of Index 0: " sqlRcord.Columns.Name(2))

Print("")

;All Columns:
For(i as Numeric to sqlRcord.Columns.Count())
	Prints(sqlRcord.Columns.Name(i) & "\t")
Next

Print("")

;All Rows:
While(sqlRcord.Fetch())
	var sRow as String

	For(i as Numeric to sqlRcord.Columns.Count())
		sRow.Append(sqlRcord.Value(i) & "\t")
	Next
	Print(sRow)
WEnd

sqlRcord.Close();
sqlConnection.Close();

%>
