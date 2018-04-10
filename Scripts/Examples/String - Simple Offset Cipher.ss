<%

Function SimpleOffset(input as String) as String

	var result as String

	For(index as Numeric to input.Length())
		result.Append(Char(ASCII(input.SubString(index, 1)) ^ 64)) ;3.8968
	Next

	Return(result)
	
End Function

var originalText as String = "Hello Cruel World"
var cipheredText as String = SimpleOffset(originalText)

Print("Original: " & originalText)
Print("Ciphered: " & cipheredText)
Print("Original: " & SimpleOffset(cipheredText))

%>
