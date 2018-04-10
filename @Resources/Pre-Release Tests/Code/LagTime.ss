<%

var totalLag as Numeric

Print(TestLag(100, 100)) ;Release: 100,100=920

Function TestLag(targetTime as Numeric, iterations as Numeric) as Numeric

	var before as Numeric
	var totalLag as Numeric
	
	For(i as Numeric = 0 to iterations)
		var before as Numeric = sys.TickCount()
	
		While(Sys.TickCount() - before < targetTime)
			;Workload.
		WEnd
	
		totalLag += (Sys.TickCount() - before) - targetTime
	Next
	
	Return(totalLag)
End Function

%>
