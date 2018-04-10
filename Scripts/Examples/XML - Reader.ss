<%
var people as XML.Reader

people.Open("C:\\People.xml")

people.ProgressiveScan(true)

Var record as XML.Reader
While(people.ToReader("Person", @record))
	Print("ID: " & record.ToInteger("Id") & ", First:" & record.ToString("FirstName") & ", Last: " & record.ToString("LastName"))
WEnd
%>
