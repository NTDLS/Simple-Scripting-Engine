<%

    Function TestRecursionProc(iLevel as Numeric)
    
    	Print("In: " & iLevel & " -> (Scope: " & Code.Scope() & ")")
    
    	If(iLevel < 10)
       		TestRecursionProc(iLevel + 1)
       	Else
       		Print("Done with Recursion!")
    	End If
    	
    	Print("Out: " & iLevel & " -> (Scope: " & Code.Scope() & ")")
    
    End Function
    
    Print("Recursion Begin: (Scope: " & Code.Scope() & ")")
	TestRecursionProc(0)
    Print("Recursion End: (Scope: " & Code.Scope() & ")")
    
    Print("Hello")


%>