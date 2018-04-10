<%

Function Cipher(sInput as String, sKey as String) as String

	var iKeyIndex as Numeric
	var sResult as String
	
	For(iDataIndex as Numeric to sInput.Length())
	
		sResult.Append(Char(ASCII(sInput.SubString(iDataIndex, 1)) _
			^ (ASCII(sKey.SubString(iKeyIndex, 1)))))
		
		If(++iKeyIndex = sKey.Length())
			iKeyIndex = 0
		End If
	Next

	Return(sResult)
End Function

var sKey as String = "m0n3y$"
var sOriginal as String = "This is the string that will be encrypted, then decrypted"
var sCiphered as String = Cipher(sOriginal, sKey)

Print("Original(" & sOriginal.Length() & "): " & sOriginal)
Print("Ciphered(" & sCiphered.Length() & "): " & sCiphered)
Print("Original: " &  Cipher(sCiphered, sKey))

%>
