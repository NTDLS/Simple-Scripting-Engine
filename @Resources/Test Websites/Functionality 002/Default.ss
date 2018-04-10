<%
#debug

#ContentType "text/html"

#Include "Utility.ss"
#Include "Body.ss"
%>

<html>
	<head>
		<title><%=Engine.Name%></title>
		<%#Include "Style/Default.ss"%>
	</head>
	<body>
		<br />
		<table border="1" width="100%" align="center" cellspacing="0" cellpadding="0">
		<tr>
			<td class="default">
				&nbsp;
			</td>
			<td class="default">
				&nbsp;
				<font size="3" color="#AA0000"><%=Engine.Name%> Showcase</font>
				<font size="1" color="#AA0000"> v<%=Engine.Version%></font>
			</td>
		</tr>
		<tr height="200">
			<td class="default" width="130" align="left" valign="top">
				<%#Include "Menu.ss"%>
			</td>
			<td class="default" width="100%" align="left" valign="top">
				<%;----------( Body Begin )----------%>
				<table width="100%" border="0">
				<tr>
					<td>
						<%PopulateBody()%>
					</td>
				</tr>				
				</table>
				<%;----------(  Body End  )----------%>
			</td>
		</tr>
		<tr>
			<td colspan="2" class="default">
				<font size="1">
				<%=Engine.Name & " v" & Engine.Version%> running on <%=Env.Get("SERVER_SOFTWARE")%>
				</font>
			</td>
		</tr>
		</table>
	</body>
</html>
