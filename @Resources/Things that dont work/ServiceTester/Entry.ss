<%
#Include "Helpers/WindowsServices.ss"

var services as List = EnumerateServices("Avid")

For(i as Numeric to services.Count())
    Print("Name: " & services[i])
Next

%>
