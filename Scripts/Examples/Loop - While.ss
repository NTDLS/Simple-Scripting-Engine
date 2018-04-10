<%

var iOuter as Numeric
While(iOuter < 10)

	var iInner as Numeric
	While(iInner < 10)
		Print("Loop: Outer(" & iOuter & ") / Inner(" & iInner & ")")
		iInner++
	WEnd
	
	iOuter++
WEnd

%>

