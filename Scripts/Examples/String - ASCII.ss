<%

var sString as String = "Hello"

For(i as Numeric = 0 to sString.Length())
	print("Character[" & i & "]: " & CustomToUpper(sString[i]))
Next

Function CustomToUpper(sValue as String) as String
	If(ASCII(sValue) >= ASCII("a") and ASCII(sValue) <= ASCII("z"))
		Return(Char(ASCII(sValue) - 32))
	Else
		Return(sValue)
	End If
End Function

Function CustomToLower(sValue as String) as String
	If(ASCII(sValue) >= ASCII("A") and ASCII(sValue) <= ASCII("Z"))
		Return(Char(ASCII(sValue) + 32))
	Else
		Return(sValue)
	End If
End Function

%>