<%

;The : symbol in front of a literal string causes the parser to ignore the escape character.
;	This allows for easy string file paths since you will not need to escape all '\' characters.
var sFilePath as String = :"C:\Temp\File.txt"

Print(sFilePath)

%>
