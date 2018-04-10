<%;Preprocessors
#Include "Windows.Registry.ss"
%>
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
<%;Definitions
    
#define EVENTLOG_SUCCESS                0x0000
#define EVENTLOG_ERROR_TYPE             0x0001
#define EVENTLOG_WARNING_TYPE           0x0002
#define EVENTLOG_INFORMATION_TYPE       0x0004
#define EVENTLOG_AUDIT_SUCCESS          0x0008
#define EVENTLOG_AUDIT_FAILURE          0x0010

#define MSG_BASIC_FAILURE                0xC0020001UL
#define MSG_BASIC_INFORMATION            0x40020002UL
#define MSG_BASIC_WARNING                0x80020003UL

%>
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
<%;External Prototypes

Declare RegisterEventSource Lib "advapi32.dll" Alias "RegisterEventSourceA" _
	(ByVal lpUNCServerName As StrictType.String, _
    ByVal lpSourceName As StrictType.String) As StrictType.Long

Declare DeregisterEventSource Lib "advapi32.dll" _
	(ByVal hEventLog As StrictType.Long) As StrictType.Long

Declare ReportEvent Lib "advapi32.dll" Alias "ReportEventA" _
	(ByVal hEventLog As StrictType.Handle, _
    ByVal wType As StrictType.uInt16, _
    ByVal wCategory As StrictType.uInt16, _
    ByVal dwEventID As StrictType.uInt32, _
    ByVal lpUserSid As StrictType.uInt32, _
    ByVal wNumStrings As StrictType.uInt16, _
    ByVal dwDataSize As StrictType.uInt32, _        
    ByRef lpStrings As StrictType.Long, _
    ByVal lpRawData As StrictType.Long) As StrictType.Boolean
%>
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
<%;Abstractions

Function RecordInfo(sMessage As String) as Numeric
	var eventName as String = Replace(File.Name(Process.ModuleName()), File.Extension(Process.ModuleName()), "") & "EventMsg"
	Return (RecordEvent(EVENTLOG_INFORMATION_TYPE, MSG_BASIC_INFORMATION, eventName, sMessage))
End Function

Function RecordError(sMessage As String) as Numeric
	var eventName as String = Replace(File.Name(Process.ModuleName()), File.Extension(Process.ModuleName()), "") & "EventMsg"
	Return (RecordEvent(EVENTLOG_ERROR_TYPE, MSG_BASIC_FAILURE, eventName, sMessage))
End Function

Function RecordWarning(sMessage As String) as Numeric
	var eventName as String = Replace(File.Name(Process.ModuleName()), File.Extension(Process.ModuleName()), "") & "EventMsg"
	Return (RecordEvent(EVENTLOG_WARNING_TYPE, MSG_BASIC_WARNING, eventName, sMessage))
End Function

Function RecordEvent(eventType as Numeric, eventId as Numeric, eventName as String, sMessage As String) as Numeric

    var lRetv As Numeric
    var hEventLog As Numeric
    var hMsg As Numeric
    var cbStringSize As Numeric
    
    RegisterCustomEventSource(eventName)
    
    hEventLog = RegisterEventSource(NULL, eventName)
    
    cbStringSize = sMessage.Length()
    hMsg = Windows.GlobalAlloc(Windows.GMEM_ZEROINIT, cbStringSize + 2)
    
	Windows.CopyMemory(hMsg, sMessage, cbStringSize)

    lRetv = ReportEvent(hEventLog, eventType, 0, eventId, NULL, 1, 0, @hMsg, 0)
    If(lRetv = 0)
    	Print("An eroor occured reporting the event. Error " & Windows.GetLastError() & ".")
    End If
    
	Windows.GlobalFree(hMsg)

	DeregisterEventSource(hEventLog)

	Return (lRetv != 0)

End Function

Function RegisterCustomEventSource(eventName as String)

	var registryKeyName as String = "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\" & eventName

	Try
		Windows.Registry.GetString(Windows.Registry.HKLM, registryKeyName, "EventMessageFile")
	Catch(ex)
		Print("The event source could not be found, creating it.")
	
		Windows.Registry.CreateKey(Windows.Registry.HKLM, registryKeyName)
	
		var typesSupported as Numeric = EVENTLOG_ERROR_TYPE bOR EVENTLOG_WARNING_TYPE bOR EVENTLOG_INFORMATION_TYPE
	
		Windows.Registry.SetString(Windows.Registry.HKLM, registryKeyName, "EventMessageFile", _
			Replace(File.Path(Process.ModuleName()) & "\\EventMsg.dll", "\\\\", "\\"))
	
		Windows.Registry.SetDWORD(Windows.Registry.HKLM, registryKeyName, "TypesSupported", typesSupported)
	End Try

End Function

%>

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
<%;Example
;Print("RecordInfo: " & _
;	IIF(RecordInfo("This is info!"), _
;	"Success.", "Failure."))
;
;Print("RecordError: " & _
;	IIF(RecordError("This is an error!"), _
;	"Success.", "Failure."))
;
;Print("RecordWarning: " & _
;	IIF(RecordWarning("Hello event a warning!"), _
;	"Success.", "Failure."))
%>
