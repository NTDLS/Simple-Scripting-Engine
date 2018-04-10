<%

var people as XML.Writer
people.SetName("People")

for(i as Numeric = 0 to 10)
	var names as XML.Writer
	
	names.SetName("Person")
	names.Add("Id", i)
	names.Add("FirstName", "Steve_" & i)
	names.Add("LastName", "Jobs_" & i)

	people.AddXml(@names)	
Next
	
people.Save("C:\\People.xml")

Print("Length: " & people.Length() & ", Text: [" & people.Text() & "]")

%>
