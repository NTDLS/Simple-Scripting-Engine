<%
#Include "Windows.EventLog.ss"
%>

<%
Print("RecordInfo: " & _
	IIF(RecordInfo("This is info!"), _
	"Success.", "Failure."))

Print("RecordError: " & _
	IIF(RecordError("This is an error!"), _
	"Success.", "Failure."))

Print("RecordWarning: " & _
	IIF(RecordWarning("Hello event a warning!"), _
	"Success.", "Failure."))
%>
