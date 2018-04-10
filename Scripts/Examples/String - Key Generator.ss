<%
Function GenerateKey(iLength as Numeric) as String
	
	var sResult as String
	
	For(i as numeric = 0 to iLength)
		sResult.Append(Char((Math.Random(65, 90))))
	Next
	
	Return(sResult)
		
End Function
%>


<%
	Print(GenerateKey(100))
%>
