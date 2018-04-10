<%

var sqlConn as SQL.Connection

sqlConn.Connect(".") ;SQL.Connect(Server, [Database], [Username], [Password])

;Begin a new transation.
sqlConn.Transaction.Begin()

For(i as Numeric = 0 to 100)
	
	Try
		sqlConn.Execute("INSERT INTO Words(Word) VALUES ('" & i & "')")
	Catch(ex)
		Print("Error: " & ex.GetText())
	
		;An error occured, rollback the transaction and exit.
		sqlConn.Transaction.Rollback()
		Break
	End Try
Next

;If we are still in a transaction, commit it.
If(sqlConn.Transaction.Depth() > 0)
	sqlConn.Transaction.Commit()
End If

sqlConn.Close()

%>
