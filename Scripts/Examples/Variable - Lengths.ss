<%

Print("Integer: " & DataLen("123456789"))        ;  4 byte integer.
Print("Double : " & DataLen("123456.789101"))    ;  8 byte double.
Print("String : " & DataLen("This is a String")) ; 16 byte string.

Print("String : " & Length("123456789"))        ;  9 byte string.
Print("String : " & Length("123456.789101"))    ; 13 byte string.
Print("String : " & Length("This is a String")) ; 16 byte string.

%>

