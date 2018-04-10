<%

var sVal as string = "Test"
Print(sVal.Replace("es", "HJ").Replace("H", "_").Replace("_", "$$"))
Print(Replace(sVal, "es", "HJ").Replace("H", "_").Replace("_", "$$"))
Print("Test".Replace("es", "HJ").Replace("H", "_").Replace("_", "$$"))
Print("A" & "B" & "C" & "D" & "E".Length())

%>
