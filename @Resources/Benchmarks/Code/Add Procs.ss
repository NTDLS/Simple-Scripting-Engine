<%

Function TestFunctionLookupPerformance()
	var startTicks as Numeric = Sys.TickCount()
	var beforeTicks as Numeric
	var beforeCount as Numeric
	var setCount as Numeric 
	
	Print("Set" & Char(9) & "Count"  & Char(9) & "Total Count" & Char(9) & "Time" & Char(9) & "Total Time")
	
	For(i as Numeric = 0 to 30001)
	
		If((i % 1000) = 0)
			If(beforeTicks > 0)
				Print(++setCount & Char(9) _
					& FormatNumeric(i - beforeCount) & Char(9) _
					& FormatNumeric(i) & Char(9) _
					& FormatNumeric(Sys.TickCount() - beforeTicks) & Char(9) _
					& FormatNumeric(Sys.TickCount() - startTicks)) _
				)
			End If
		
			beforeCount = i
			beforeTicks = Sys.TickCount()
		End If	
		
		var codeLine as String
		codeLine.Append("Function Proc_" & GUID() & "() as Numeric" & CrLf)
		codeLine.Append("Return(" & i & ")" & CrLf)
		codeLine.Append("End Function" & CrLf)
			
		Code.Inject(codeLine)
	Next
End Function
	
%>
