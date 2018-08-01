<%

var sqlConnection as SQL.Connection

sqlConnection.Connect("DRIVER={ODBC Driver 11 for SQL Server};DATABASE=master;SERVER=(local);Trusted_Connection=yes;")

;Begin a new transation.
sqlConnection.Transaction.Begin()

For(i as Numeric = 0 to 100)
	
	Try
		sqlConnection.Execute("INSERT INTO Words(Word) VALUES ('" & i & "')")
	Catch(ex)
		Print("Error: " & ex.GetText())
	
		;An error occured, rollback the transaction and exit.
		sqlConnection.Transaction.Rollback()
		Break
	End Try
Next

;If we are still in a transaction, commit it.
If(sqlConnection.Transaction.Depth() > 0)
	sqlConnection.Transaction.Commit()
End If

sqlConnection.Close()

%>
