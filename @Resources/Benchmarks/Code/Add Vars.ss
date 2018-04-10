<%

Function TestVariableLookupPerformance()

	var timeToRun as Numeric = 10000
	var startTicks as Numeric = Sys.TickCount()
	var setCount as Numeric 
	var totalTime as Numeric 
	
	Print("Set" & Char(9) & "Count" & Char(9) & "Total Time")
	
	Var itemCount as Numeric = 0
	
	While(totalTime < timeToRun)
		If((itemCount % 1000) = 0)
			If(itemCount > 0)
				totalTime = Sys.TickCount() - startTicks
				Print(++setCount & Char(9) & FormatNumeric(itemCount) & Char(9) & FormatNumeric(totalTime)))
			End If
		End If

		Code.Variables.Define("var_" & GUID(), "Numeric", itemCount + 1 & " * 100", Code.Scope() - 1)

		itemCount++
	WEnd
	
	Print("var/ms: " & FormatNumeric((itemCount.ToDouble() / timeToRun.ToDouble()), 2))
	
End Function

%>
