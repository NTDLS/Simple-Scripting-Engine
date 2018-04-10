<%

Prints(Uptime())

Function Uptime() as String

	var iTicks as Numeric = (Sys.TickCount() / 1000)
	var iSeconds as Numeric = (iTicks % 60)
	var iMinutes as Numeric = (iTicks / 60 % 60)
	var iHours as Numeric = (iTicks / 3600 % 24)
	var iDays as Numeric = (iTicks / 3600 / 24)
	var sString as String

	If(iSeconds = 60)
		iSeconds = 0
		iMinutes++
	End If
	
	If(iMinutes = 60)
		iMinutes = 0
		iHours++
	End If
	
	If(iHours = 24)
		iHours = 0
		iDays++
	End If
	
	sString = PadInt(iDays, 3) & " day"
	If(iDays != 1)
		sString.Append("s")
	End If
	
	sString = sString & ", " & PadInt(iHours, 2) & " hour"
	If(iHours != 1)
		sString.Append("s")
	End If
	
	sString = sString & ", " & PadInt(iMinutes, 2) & " minute"
	If(iMinutes != 1)
		sString.Append("s")
	End If
	
	sString = sString & ", " & PadInt(iSeconds, 2) & " second"
	If(iSeconds != 1)
		sString.Append("s")
	End If
	
	Return(sString)
End Function
	
Function PadInt(iInt as Numeric, iPadTo as Numeric) as String
	
	var sPad as String = iInt
		
	While(sPad.Length() < iPadTo)
		sPad = "0" & sPad
	WEnd

	Return(sPad)

End Function
%>
