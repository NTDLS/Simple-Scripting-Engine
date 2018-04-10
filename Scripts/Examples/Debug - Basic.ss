<%

For(outer as Numeric to 10)
	For(inner as Numeric to 10)
		Print("Loop: Outer(" & outer & ") / Inner(" & inner & ")")
		
		Debug.Evaluate("\"Outer:\" & outer & \",Inner :\" & inner")
		Sleep(50)
		
		Debug.Break(outer = 5 and inner = 5) ;Break on a condition.
		
		If(outer = 9 and inner = 9)
			Debug.Break() ;Break without a condition.
		End If
	Next
Next

%>

