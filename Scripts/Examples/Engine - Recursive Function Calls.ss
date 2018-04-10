<%

Function TestRecursionProc(level as Numeric)

	Print("In: " & level & " -> (Scope: " & Code.Scope() & ")")

	If(level < 10)
   		TestRecursionProc(level + 1)
   	Else
   		Print("Done with Recursion!")
	End If
	
	Print("Out: " & level & " -> (Scope: " & Code.Scope() & ")")

End Function

Print("Recursion Begin: (Scope: " & Code.Scope() & ")")
TestRecursionProc(0)
Print("Recursion End: (Scope: " & Code.Scope() & ")")

%>
