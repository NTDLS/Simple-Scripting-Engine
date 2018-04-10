<%

var number as Numeric = 11 * 10*10*4 / 7.0 ;Do some math.
Print("Number: " & ToInteger(FormatNumeric(Math.Pow(number, 2), 5)) / 3)
Print("Val: " & FormatNumeric(ToInteger("66") / 3), 4)
Print("Pow: " & Math.Pow(number, 2) / 3)
Print("Pow: " & FormatNumeric(Math.Pow(number, 2) / 3))
Print("Pow: " & FormatNumeric(Math.Pow(number, 2) / 3, 2))
Print("Abs: " & FormatNumeric(Math.Abs(10.5 - 100.6), 2))

%>
