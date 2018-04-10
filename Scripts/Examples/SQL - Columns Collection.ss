<%

var sqlConn as SQL.Connection
var recordSet as SQL.RecordSet

sqlConn.Connect(".")

sqlConn.Execute("SELECT TOP 10 id, name, xtype FROM sysobjects", @recordSet)

print("Index of Name A: " recordSet.Columns.Index("name"))
print("Name of Index 0: " recordSet.Columns.Name(2))

Print("")

;All Columns:
For(i as Numeric to recordSet.Columns.Count())
	Prints(recordSet.Columns.Name(i) & "\t")
Next

Print("")

;All Rows:
While(recordSet.Fetch())
	var sRow as String

	For(i as Numeric to recordSet.Columns.Count())
		sRow.Append(recordSet.Value(i) & "\t")
	Next
	Print(sRow)
WEnd

recordSet.Close();
sqlConn.Close();

%>
