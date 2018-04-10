<%
Function DefaultValue(text as String, defaultValue as String) as String
	If(text.Length() > 0)
		Return(text)
	Else
		Return(defaultValue)
	End If
End Function
%>

<%Function WriteTableAttribute(name as String, value as String)%>
	<tr>
		<td class="body"><%=name%></td>
		<td class="body"><%=value%></td>
	</tr>
<%End Function%>
