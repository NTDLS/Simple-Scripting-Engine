<%

var sString as String = "Hello World"
Print("The Length of \"" & sString & "\" is " & sString.Length() & " characters.")
Print("Numeric Property: " & (5 * 50.0.Length())) ;20
Print("Test".Length()) ;4
Print("Test".Length() * 15.Length());8
Print("Test".Length() * 15.Length() * 2);16
Print("Test".Length() * 15.Length() * 2 + "ghgh".Length())
Print(1000000000.Length().Length()) ;2 (because the length of the length of 1000000000 is 2 digits (10).

If("This STRING".Equals("THIS String", False))
	Print("Is Equal")
Else
	Print("Is not Equal")
End If

If("This STRING".Equals("THIS String"))
	Print("Is Equal")
Else
	Print("Is not Equal")
End If

var iLength as Numeric = sString.Length() * 2
Print("(iLength * 2) = " & iLength)

%>