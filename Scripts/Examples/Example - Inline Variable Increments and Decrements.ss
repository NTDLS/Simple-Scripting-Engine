<%

var i as Numeric

Print(i++ & i++ & i++ & i++ & i++ & i++ & _
	  i-- & i-- & i-- & i-- & i-- & i-- & i)

Print(++i & ++i & ++i & ++i & ++i & ++i & ++i _
	  --i & --i & --i & --i & --i & --i)

i = 0

While(i < 10)
	Print("i = " & i++)
WEnd


%>