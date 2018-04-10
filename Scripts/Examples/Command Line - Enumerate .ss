<%

For(i as Numeric = 0 to Env.CommandLine.Count())
	Print("Parameter " & i & ": \"" & Env.CommandLine.Get(i) & "\"")
Next

%>
