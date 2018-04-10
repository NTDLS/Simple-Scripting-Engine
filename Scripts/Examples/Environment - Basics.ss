<%

Print("Path: " & Env.Get("Path"))

For(i as Numeric = 0 to Env.Count())
	Print(Env.Name(i) & "=" Env.Value(i))
Next

%>
