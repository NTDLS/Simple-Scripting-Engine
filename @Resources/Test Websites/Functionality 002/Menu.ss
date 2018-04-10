<table border="0" cellpadding="2" cellspacing="0">
	<%MenuItem("Home")%>
	<%MenuItem("Engine")%>
	<%MenuItem("Process")%>
	<%MenuItem("System")%>
	<%MenuItem("Environment")%>
</table>

<%Function MenuItem(name as String)%>
	<tr>
		<td>
			<img src="/Images/ArrowBullet.gif" />
		</td>
		<td>
			<a href="/<%=File.Name(Env.Get("SCRIPT_FILENAME"))%>/<%=name%>"><%=name%></a><br />
		</td>
	</tr>
<%End Function%>
