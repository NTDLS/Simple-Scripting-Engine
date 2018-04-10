<%

var Encoded as String = Web.URLEncode("\"This (is) at Test @ You!\"")
var Decoded as String = Web.URLDecode(Encoded)

Print("Encoded: " & Encoded)
Print("Decoded: " & Decoded)

%>