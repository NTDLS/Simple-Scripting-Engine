<%;Preprocessors

#Namespace "Windows.Time"
#Include "Windows.ss"

%>
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
<%;Definitions

StrictType SystemTime
	Year         as StrictType.Int16
	Month        as StrictType.Int16
	DayOfWeek    as StrictType.Int16
	Day          as StrictType.Int16
	Hour         as StrictType.Int16
	Minute       as StrictType.Int16
	Second       as StrictType.Int16
	Milliseconds as StrictType.Int16
End StrictType

%>
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
<%;External Prototypes

Declare GetSystemTime Lib "kernel32" (ByRef lpSystemTime As SystemTime)
Declare GetLocalTime Lib "kernel32" (ByRef lpSystemTime As SystemTime)

%>
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
<%;Abstractions


%>
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
<%;Example
;
;var stTime as SYSTEMTIME
;
;Windows.Time.GetSystemTime(@stTime)
;Print(stTime.Month & "/" & stTime.Day & "/" & stTime.Year _
;	& " " & stTime.Hour & ":" & stTime.Minute & ":" & stTime.Second)
;	
%>
