<%

;---(Code Scope & Line)---
Print("Scope: " & Code.Scope())
Print("Line: " & Code.Line())

;---(Code Injection)---
Code.Inject("Function InjectedTestProc(sText as String) as String" _
	& CrLf & "Return(sText & \", and this was passed out\")" _
	& CrLf & "End Function")
	
Print(InjectedTestProc("This was passed in"))

%>
