<%
Function PopulateBody()
	var pathInfo as String = Env.Get("PATH_INFO").Replace(Env.Get("SCRIPT_NAME"), "")

	If(pathInfo.Length() = 0)
		PutHome()
	Else
		If(Code.Functions.IsDefined("Put" & pathInfo.SubString(1)))
			Code.Functions.Call("Put" & pathInfo.SubString(1) & "()")
		Else
			PutHome()
		End If
	End If

End Function
%>

<%Function PutHome()%>
	<i>Choose a menu item.</i>
<%End Function%>

<%Function PutEngine()%>
	<table border="0" width="90%" align="center" cellspacing="0" cellpadding="1">
	<tr>
		<td colspan="3" class="head"><font size="4">Engine Information</font></td>
	</tr>
	<tr>
		<td class="head"><b>Name</b></td>
		<td class="head"><b>Value</b></td>
	</tr>

	<%
	WriteTableAttribute("Name", Engine.Name)
	WriteTableAttribute("Version", Engine.Version)
	WriteTableAttribute("Build", Engine.Build)
	%>
	
	</table>
<%End Function%>

<%Function PutProcess()%>
	<table border="0" width="90%" align="center" cellspacing="0" cellpadding="1">
	<tr>
		<td colspan="3" class="head"><font size="4">Process Information</font></td>
	</tr>
	<tr>
		<td class="head"><b>Name</b></td>
		<td class="head"><b>Value</b></td>
	</tr>

	<%
	WriteTableAttribute("ProcessId", FormatNumeric(Process.Id()))
	WriteTableAttribute("Working Set", FormatSize(Process.WorkingSetSize(), 2))
	WriteTableAttribute("Page File", FormatSize(Process.PagefileUsage(), 2))
	WriteTableAttribute("User", Process.Username())
	%>
	
	</table>
<%End Function%>

<%Function PutSystem()%>
	<table border="0" width="90%" align="center" cellspacing="0" cellpadding="1">
	<tr>
		<td colspan="3" class="head"><font size="4">Operating System</font></td>
	</tr>
	<tr>
		<td class="head"><b>Name</b></td>
		<td class="head"><b>Value</b></td>
	</tr>

	<%
	WriteTableAttribute("OS Version", Sys.OSVersion())
	WriteTableAttribute("Machine Name", Sys.MachineName())

	WriteTableAttribute("Total Physical", FormatSize(Sys.Memory.TotalPhysical(), 2))
	WriteTableAttribute("Available Physical", FormatSize(Sys.Memory.AvailablePhysical(), 2))

	WriteTableAttribute("Total Virtual", FormatSize(Sys.Memory.TotalVirtual(), 2))
	WriteTableAttribute("Available Virtual", FormatSize(Sys.Memory.AvailableVirtual(), 2))

	WriteTableAttribute("Total Page File", FormatSize(Sys.Memory.TotalPageFile(), 2))
	WriteTableAttribute("Available Page File", FormatSize(Sys.Memory.AvailablePageFile(), 2))

	WriteTableAttribute("Memory Load", FormatNumeric(Sys.Memory.Load(), 2) & "%")
	%>
	
	</table>
<%End Function%>

<%Function PutEnvironment()%>
	<table border="0" width="90%" align="center" cellspacing="0" cellpadding="1">
	<tr>
		<td colspan="3" class="head"><font size="4">Environment Variables</font></td>
	</tr>
	<tr>
		<td class="head"><b>Index</b></td>
		<td class="head"><b>Name</b></td>
		<td class="head"><b>Value</b></td>
	</tr>
	<%For(i as Numeric = 0 to Env.Count())%>
	<tr>
		<td class="body"><%=i%></td>
		<td class="body"><%=Env.Name(i)%></td>
		<td class="body"><%=DefaultValue(Env.Value(i), "&nbsp;")%></td>
	</tr>
	<%Next%>
	</table>
<%End Function%>
