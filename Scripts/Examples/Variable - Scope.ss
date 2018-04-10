<%

var sScopeTest as String = "Outer Level is at Scope " & Code.Scope()

if(1 = 1)
	var sScopeTest as String = "Inner Level is at Scope " & Code.Scope()
	Print(sScopeTest)
End if

Print(sScopeTest)

%>
