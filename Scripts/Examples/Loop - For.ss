<%

;Loop using internal variable.
For(iCount as Numeric = 0 to 10)
	Print("Count[1]:" & iCount)
Next

var iCount as Numeric

;Loop using external variable.
For(iCount = 0 to 10)
	Print("Count[2]:" & iCount)
Next

;Loop using external variable without setting value.
For(iCount to 20)
	Print("Count[3]:" & iCount)
Next

%>

