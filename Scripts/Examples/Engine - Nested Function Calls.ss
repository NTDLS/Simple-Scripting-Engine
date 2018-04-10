<%

Function Add(x as Numeric, y as Numeric) as Numeric
    Return(x + y)
End Function

Add(1,1)

Print("[" & Add(5,Add(10,Add(15,Add(20,Add(25,Add(30,Math.Random())))))) & "]")
Print("[" & Add(5,Add(10,Add(15,Add(20,Add(25,Add(30,35)))))) & "]")

%>
