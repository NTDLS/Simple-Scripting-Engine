<%

Settings.ThrowSoftExceptions(False) ;Turn off "Throw Soft Exceptions".

;Throw 5 errors:
Error.Throw("Custom error #1")
Sleep(-100) ;This will produce a "soft" error "Sleep time must be greater than zero"
Sleep(-100) ;This will produce a "soft" error "Sleep time must be greater than zero"
Sleep(-100) ;This will produce a "soft" error "Sleep time must be greater than zero"
Error.Throw("Custom error #2")

If(Error.Count()) ;Did an error occur?

	Print(Error.Count() & " error(s) occured!")

	;Loop through all of the errors that occured
	For(index as Numeric to Error.Count())
		Print(Tab & index + 1 & "): \"" & Error.Text(index) & "\" on line " & Error.Line(index))
	Next
	
	Error.Clear() ;Clear the errors.
Else
	Print("No errors occured.")
End If

Settings.ThrowSoftExceptions(True)

%>
