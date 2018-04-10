<%

var plainText as String = "Hello World"
var encoded as String = Base64Encode(plainText)
var decoded as String = Base64Decode(encoded)
Print("Plain Text: \"" & plainText & "\", Length: " & plainText.Length())
Print("Encoded: \"" & encoded & "\", Length: " & encoded.Length())
Print("Decoded: \"" & decoded & "\", Length: " & decoded.Length())

%>