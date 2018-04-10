;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                       -- Healthy Dose of Reality --                        ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; This script was designed to test the various parts of the scripting engine ;
;  throughout the development process *and* prior to any release to ensure   ;
;  that core functionality has not been altered by new features or "fixes".  ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
<%="--Beginning of File--" & CrLf%>
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

<% ;Configuration Values:
var PerformSQLTests as Numeric = True
%>

<% ;--(Test recursive properties and literal properties)----------------------
var sVal as string = "Test"
Print(sVal.Replace("es", "HJ").Replace("H", "_").Replace("_", "$$"))
Print(Replace(sVal, "es", "HJ").Replace("H", "_").Replace("_", "$$"))
Print("Test".Replace("es", "HJ").Replace("H", "_").Replace("_", "$$"))
Print("A" & "B" & "C" & "D" & "E".Length())
%><%HorizontalRule()%>

<% ;--(Inline Variable increments and decrements)-----------------------------
Code.Scope.Enter()
var i as Numeric

Print(i++ & i++ & i++ & i++ & i++ & i++ & _
	  i-- & i-- & i-- & i-- & i-- & i-- & i)

Print(++i & ++i & ++i & ++i & ++i & ++i & ++i _
	  --i & --i & --i & --i & --i & --i)

i = 0

While(i < 10)
	Print("i = " & i++)
WEnd
Code.Scope.Exit()
%><%HorizontalRule()%>

<% ;--(Advanced Nested Exception/Error Handling)------------------------------
var sVariable as String = "Scope: " & Code.Scope()

Print("Global[1]->" & sVariable & " <Must See #1>")

Try
	Try
		Try
			var sVariable as String = "Scope: " & Code.Scope()
			Print("Try[2]->" & sVariable & " <Must See #2>")
			Sleep(-1)
			Print("Try[3]->" & sVariable & " <CANNOT GET HERE!>")
		Catch(ex)
			var sVariable as String = "Scope: " & Code.Scope()
			;Sleep(-1)
			Print("Catch[4]->" & sVariable & " <Must See #3>" & "[" & ex.GetText() & "]")
		End Try
		
		Sleep(-1)
	Catch(ex)
		Try
			Try
				Try
					var sVariable as String = "Scope: " & Code.Scope()
					Print("Try[5]->" & sVariable & " <Must See #4>")
					Sleep(-1)
					Print("Try[6]->" & sVariabl & " <CANNOT GET HERE!>")
				Catch(ex)
					var sVariable as String = "Scope: " & Code.Scope()
					Print("Catch[7]->" & sVariable & " <Must See #5>" & " [" & ex.GetText() & "]")
				End Try
			Catch
				var sVariable as String = "Scope: " & Code.Scope()
				Sleep(-1)
				Print("Catch[8]->" & sVariable & " <CANNOT GET HERE!>" & " [" & ex.GetText() & "]")
			End Try
			
			Try
				Try
					var sVariable as String = "Scope: " & Code.Scope()
					Print("Try[9]->" & sVariable & " <Must See #6>")
					Sleep(-1)
					Print("Try[10]->" & sVariable & " <CANNOT GET HERE!>")
				Catch(ex)
					var sVariable as String = "Scope: " & Code.Scope()
					Print("Catch[11]->" & sVariable & " <Must See #7>" & " [" & ex.GetText() & "]")
				End Try
			Catch(ex)
				var sVariable as String = "Scope: " & Code.Scope()
				Print("Catch[12]->" & sVariable & " <CANNOT GET HERE!>" & " [" & ex.GetText() & "]")
			End Try
						
		Catch(ex)
			var sVariable as String = "Scope: " & Code.Scope()
			Print("Catch[13]->" & sVariable & " <CANNOT GET HERE!>" & "[" & ex.GetText() & "]")
		End Try
	End Try
Catch(ex)
	var sVariable as String = "Scope: " & Code.Scope()
	Print("Catch[14]->" & sVariable & " <CANNOT GET HERE!>" & " [" & ex.GetText() & "]")
End Try

Print("Global[15]->" & sVariable & " <Must See #8>")
%><%HorizontalRule()%>

<% ;--(Basic Exception/Error Handling)------------------------------
Settings.ThrowSoftExceptions(False)
Sleep(-100) ;This will produce the error "Sleep time must be greater than zero"

If(Error.Count()) ;Did an error occur?
	Print(Error.Count() & " error(s) occured:")
	
	;Loop through all of the errors that occured
	var iIndex as Numeric
	While(iIndex < Error.Count())
		Print(Tab & iIndex + 1 & "): " & Error.Text(iIndex) & " on line " & Error.Line(iIndex))
		iIndex++
	WEnd
	
	Error.Clear() ;Clear the errors.
Else
	Print("No errors occured.")
End If
Settings.ThrowSoftExceptions(True)
%><% HorizontalRule() %>

<% ;---Test basic file operations)-------------------------------
;FileAccess:
;	.Read		: Opens a file for read access.
;	.Write		: Opens a file for write access.
;	.ReadWrite	: Opens a file for both read and write access.

var txtFile as File ;Declare the file object.

;Create a file:
If(txtFile.Open(:"C:\Test.txt", FileAccess.Write))
	var BytesWritten as Numeric = txtFile.Write("This is the contents of the file!")
	Print("Write: " & BytesWritten & " bytes.")
	txtFile.Close()
	
	;Read from an existing file:
	If(txtFile.Open(:"C:\Test.txt", FileAccess.Read))
		;txtFile.Seek(5) ;Seek from the current position.
		;txtFile.Seek.FromEnd(-10) ;Seek from the end of the file
		;txtFile.Seek.FromBeginning(12) ;Seek from the beginning of the file
		Print("Read: [" & txtFile.Read() & "]")
		txtFile.Close()
	End If
End If

File.Delete(:"C:\Test.txt")

If(Error.Count() > 0)
	Print("File testing failed with " & Error.Count() & " error(s)!")
Else
	Print("File testing was successfull!")
End If
%><%HorizontalRule()%>

<% ;---(Test the speed of an iterator)-------------------------------
Function TestSpeed(Iterations as Numeric)

	var iStartTime as Numeric = Sys.TickCount()
	var iSpeedLoop as Numeric = 0
	
	While(iSpeedLoop++ < Iterations)
		;Do nothing...
	WEnd
	
	var dTime as Numeric = (Sys.TickCount() - iStartTime) / 1000.0
	
	If(dTime != 0)
		Print("Iterator Time: " & FormatNumeric(dTime, 2) _
			& " seconds @ " & FormatNumeric(Iterations / dTime, 0) & " loops/second.")
	End If
End Function

TestSpeed(1000)
%>

<%If(PerformSQLTests = True)%>
	<% ;---(SQL Singleton Functionality)-------------------------------
	Try
		var sqlConnection as SQL.Connection
		sqlConnection.Connect("(local)") ;SQL.Connect(Server, [Database], [Username], [Password])
	
		For(iItt as Numeric = 0 to 10)
			Print(sqlConnection.Value("SELECT GetDate()"))
			Print(sqlConnection.Value("SELECT NewID()"))
			Print(FormatNumeric(sqlConnection.Value("SELECT CheckSum(NewID())") / 6767.6767, 5))
			Print(sqlConnection.Value("SELECT Convert(VarChar, GetDate(), 101)"))
		Next
		
		sqlConnection.Close()
	Catch(ex)
		Print("(SQL Singleton Functionality Error: " & ex.GetText())
		Debug.Break()
	End Try
	%><%HorizontalRule()%>
	
	<% ;---(Test SQL General Functionality)-------------------------------
	Try
		var sqlConnection as SQL.Connection
		sqlConnection.Connect("(Local)")
		var recordSet as SQL.RecordSet
		
		sqlConnection.Execute("SELECT * FROM sys.objects WHERE type <> 'u' order by create_date desc ", @recordSet)
		
		While(recordSet.Fetch())
			Print("(" & FormatNumeric(recordSet.Value("object_id")) _
				& ") - [" & recordSet.Value("name") & "]" _
				& " was created on \"" & recordSet.Value("create_date") & "\"")
		WEnd
	
		recordSet.Close()
		
		sqlConnection.Close()
	Catch(ex)
		Print("(SQL General Functionality Error: " & ex.GetText())
		Debug.Break()
	End Try
	%><%HorizontalRule()%>

<%End If%>

<% ;---(Test Scopes)-------------------------------
var Variable as String = "Test1"

Code.Scope.Enter()
var Variable as String = "Test2"
Print("Variable: " & Variable)
Code.Scope.Exit()

Code.Scope.Enter()
var Variable as String = "Test3"
Print("Variable: " & Variable)
Code.Scope.Exit()

Print("Variable: " & Variable)
%><%HorizontalRule()%>

<% ;---(Test Properties)-------------------------------
var sString as String = "Hello World"
Print("The Length of \"" & sString & "\" is " & sString.Length() & " characters.")
Print("Numeric Property: " & (5 * 50.0.Length())) ;20
Print("Test".Length()) ;4
Print("Test".Length() * 15.Length());8
Print("Test".Length() * 15.Length() * 2);16
Print("Test".Length() * 15.Length() * 2 + "ghgh".Length())
Print(1000000000.Length().Length()) ;2 (because the length of the length of 1000000000 is 2 digits (10).

If("This STRING".Equals("THIS String", False))
	Print("Is Equal")
Else
	Print("Is not Equal")
End If

If("This STRING".Equals("THIS String"))
	Print("Is Equal")
Else
	Print("Is not Equal")
End If

var iLength as Numeric = sString.Length() * 2
Print("(iLength * 2) = " & iLength)
%><%HorizontalRule()%>

---(Test inline code)---
This is a random <%=FormatNumeric(Math.Random(), 0)%> number. <%=CrLf%>
<%HorizontalRule()%>

<% ;---(Test Misc. Core Functionality)-------------------------------
Print("Scope: " & Code.Scope())
Print("Line: " & Code.Line())
%><%HorizontalRule()%>

<% ;---(Test Misc. Date/Time Functionality)-------------------------------
Print("Date: " & Date.Now())
Print("Date: " & Date.Now("(MM.dd.yyyy)"))
Print("Time: " & Time.Now())
Print("Time: " & Time.Now("HH:mm:ss.ms"))
%><%HorizontalRule()%>

<% ;---(Test Misc. String Functionality)-------------------------------
Print("ASCII: " & ASCII("A"))
Print("Char: " & Char(60 + 5))
Print("Reverse: " & Reverse("dlroW olleH"))
Print("SubString: [" & SubString("Hello World", 3, 5) & "]")
Print("SubString: [" & SubString("Hello World", 6) & "]")
%><%HorizontalRule()%>

<% ;---(Test While Loop)-------------------------------
var iOuter as Numeric = 0

While(iOuter < 5)
	var iInner as Numeric = 0 ;Note that this variable falls in and out of scope.

	While(iInner < 5)
		Print("Loop: Outer(" & iOuter & ") / Inner(" & iInner & ")")
		iInner++ ;Various incrementors
	WEnd	
		
	iOuter += 1 ;Various incrementors
WEnd
%><%HorizontalRule()%>

<% ;---(Test Variable Lengths)-------------------------------
Print(DataLen("123456789"))        ;  4 byte integer.
Print(DataLen("123456.789101"))    ;  8 byte double.
Print(DataLen("This is a String")) ; 16 byte string.

Print(Length("123456789"))        ;  9 byte string.
Print(Length("123456.789101"))    ; 13 byte string.
Print(Length("This is a String")) ; 16 byte string.
%><%HorizontalRule()%>

<% ;---(Test String->Numeric Conversion)------------------------------
var iNumeric as Numeric = 10 * 10 & "77.66" ; "100" & "77.66" = "10077.66"
Print("iNumeric: " & FormatNumeric(iNumeric * 2, 2)) ;20,155.32
%><%HorizontalRule()%>

<% ;---(Test Variable Incrementers)-------------------------------
var iIncrement as Numeric = 10

Print("iIncrement: " & iIncrement)
iIncrement++
Print("iIncrement: " & iIncrement)
iIncrement--
Print("iIncrement: " & iIncrement)
iIncrement += (66.77 + 0.23) + 0.5 ;Will equal 77.5
Print("iIncrement: " & iIncrement)
iIncrement -= (66.77 + 0.23) + 0.5 ;Will equal 10
Print("iIncrement: " & iIncrement)
%><%HorizontalRule()%>

<% ;---(test Scoped Variables)-------------------------------
var sScopeTest as String = "Outer Level"

if(1 = 1)
	var sScopeTest as String = "Inner Level"
	Print(sScopeTest)
End if

Print(sScopeTest)
%><%HorizontalRule()%>

<% ;---(Test basic logic)-------------------------------
	Print("Before Scope: " & Code.Scope())

    If(1 = 0 or (10 * 9.99) >= 99.90) ;True
        Print("In If #1(" & Code.Scope() & ")")
 
	    If(1 = 2) ;False
	        Print("In If #2 (" & Code.Scope() & ")")
	    Else If(100 >= 10*10) ;True
	        Print("In Else #2 (" & Code.Scope() & ")")
	        
		    If(1 = 1) ;True
		        Print("In If #3 (" & Code.Scope() & ")")
		    Else
		        Print("In Else #3 (" & Code.Scope() & ")")
		    End If
		Else
			Print("In last else #2 (" & Code.Scope() & ")")
	    End If        
    Else
        Print("In Else #1 (" & Code.Scope() & ")")
    End If

	Print("After Scope: " & Code.Scope())
%><%HorizontalRule()%>

<% ;---(Test recursive function calls)-------------------------------
    Function TestRecursionProc(iLevel as Numeric)
    
    	Print("In: " & iLevel & " -> (Scope: " & Code.Scope() & ")")
    
    	If(iLevel < 10)
       		TestRecursionProc(iLevel + 1)
       	Else
       		Print("Done with Recursion!")
    	End If
    	
    	Print("Out: " & iLevel & " -> (Scope: " & Code.Scope() & ")")
    
    End Function
    
    Print("Recursion Begin: (Scope: " & Code.Scope() & ")")
	TestRecursionProc(0)
    Print("Recursion End: (Scope: " & Code.Scope() & ")")
    
    Print("Hello")
%><%HorizontalRule()%>

<%
	Print("Line One" _
		& CrLf & "Line Two" _
		& CrLf & "Line Three" _
		& CrLf & "Line Four")
%><%HorizontalRule()%>

<% ;---(Test code injection)-------------------------------
Code.Inject("Function InjectedTestProc(sText as String) as String" _
	& CrLf & "Return(sText & \", and this was passed out\")" _
	& CrLf & "End Function")
	
Print(InjectedTestProc("This was passed in"))
%><%HorizontalRule()%>

<% ;---(Test State functionality)-------------------------------
	Print("Scope: " & Code.Scope() & _
		CrLf & "Line: " & Code.Line())
%><%HorizontalRule()%>

<% ;---(Basic Math Test)-------------------------------
var Number as Numeric = 11 * 10*10*4 / 7.0 ;Do some math.
Print("Number: " & ToInteger(FormatNumeric(Math.Pow(Number, 2), 5)) / 3)
Print("Number: " & ToInteger(Math.Pow(Number, 2)) / 3)
Print("Round: " & Math.Round(100.12346000000000, 2))
Print("Smart Round:" & Math.SmartRound(100.123460000000010))
Print("ParseDoubleExpression: " & Math.SmartRound(Math.ParseDoubleExpression("10 * 9.99"))))
Print("ParseIntegerExpression: " & Math.ParseIntegerExpression("10 * 9.99"))
%><%HorizontalRule()%>

<% ;---(Test parenthesis positions in common mathematical calls)-------------------------------
	Function GetNumber() as Numeric
		Return(66.66)
	End Function

	Print(GetNumber())

	Print(GetNumber() / (77.0 * 255))
	Print((GetNumber() / 77.77) * 255)
%><%HorizontalRule()%>

<% ;---(Test nested function calls)-------------------------------
    Function Add(iX as Numeric, iY as Numeric) as Numeric
        Return(iX + iY)
    End Function

    Add(1,1)

    Print("[" & Add(5,Add(10,Add(15,Add(20,Add(25,Add(30,Math.Random())))))) & "]")
    Print("[" & Add(5,Add(10,Add(15,Add(20,Add(25,Add(30,35)))))) & "]")
%><%HorizontalRule()%>

<% ;---(Test String and Numeric concatenations)-------------------------------
    var iMath as Numeric = (81 * 11) + 5
    var sValue2 as String = "I have " & 11 * iMath + 123 / 2 & " dollars in the bank!"
    var sValue3 as String = "I have " & iMath * 11 + 123 / 2 & " dollars in the bank!"
    var sValue4 as String = "I have " & iMath * 11 + 123 / 2.0 & " dollars in the bank!"

    Print("iMath: " & iMath)
    Print("sValue2: " & sValue2)
    Print("sValue3: " & sValue3)
    Print("sValue4: " & sValue4)

    Print("Random: " & Math.Random())
    Print("GUID: " & GUID() & " (Blah->" & Math.Random() & ")")

    iMath = iMath + 1
    iMath++

    ;6 to the power of 4 = 1296
    Math.Pow(6,4)
    Print("Power of 6,(2+2): " & Math.Pow(6,2+2))
%><%HorizontalRule()%>

---(Lots of Complex code Blocks)-------------------------------
<%
Prints("Print(s) #1")%> -- <%Prints("Print(s) #2" & CrLf)
%><%
Prints("This")%> is a rand<%   Prints("om " & Math.Random()) %> - and so is <%Prints("this" & " " & Math.Random())%> Text <%=CrLf%>
<%HorizontalRule()%>

<% ;---(Test Preprocessors)-------------------------------
#define TEST_STR	"This is a Test!"	
#define TEST_DEC	1024*1024 ;1,048,576
#define TEST_HEX	0xff ;255 in Hex
#define TEST_MATH	((((TEST_HEX*2)+2)/2)*4) ;1,024

Print("String: " & TEST_STR)
Print("Dec: " & TEST_DEC)
Print("Hex: " & TEST_HEX)
Print("Math: " & TEST_MATH)

;#undef TEST_MATH ;TEST_MATH above will not exist if this line is uncommented because it is a preprocessor.
%><%HorizontalRule()%>

<% ;--(URLEncoding & URLDecoding)------------------------------
var Encoded as String = Web.URLEncode("\"This (is) at Test @ You!\"")
var Decoded as String = Web.URLDecode(Encoded)

Print("Encoded: " & Encoded)
Print("Decoded: " & Decoded)
%><%HorizontalRule()%>

<% ;--(OS)-----------------------------------------------------------------
Print("OS Version: " & Sys.OSVersion())
Print("Username: " & Process.Username())
Print("MachineName: " & Sys.MachineName())
%><%HorizontalRule()%>

<% ;--([For] Loop Functionality)------------------------------
var iCount1 as Numeric = 5*2
For(iCount1 to 10+10)
	Print("Count[1]: " & iCount1)
Next

var iCount2 as Numeric
For(iCount2 = 5*2 to 10+10)
	Print("Count[2]: " & iCount2)
Next

For(iCount3 as Numeric = 5*2 to 10+10)
	Print("Count[3]: " & iCount3)
Next
%><%HorizontalRule()%>

<% ;--(String Interpolation)------------------------------
var sHelloString as String = "Hello World"
Print($"Is it customary for an application to say ${sHelloString} in an example!")
%><%HorizontalRule()%>

<% ;--(List / Vector Functionality)------------------------------
var listItems as List

Print("Adding Items:")
listItems.Add("Hello")
listItems.Add("Cruel")
listItems.Add("World")

Print("\tCount: " & listItems.Count())
Print("\tAllocated: " & listItems.Allocated())
Print("\tContains 'Cruel': " & listItems.Contains("Cruel"))
Print("\tContains 'Decent': " & listItems.Contains("Decent"))
Print("\tIndexOf 'Cruel': " & listItems.IndexOf("Cruel"))
Print("\tIndexOf 'Decent': " & listItems.IndexOf("Decent"))

Print("\nEnumeration:")
For(i as Numeric to listItems.Count())
	Print("\tList[" & i & "]: " & listItems[i])
Next

Print("\nDelete 'Cruel'.")
listItems.DeleteAll("Cruel")

Print("\tCount: " & listItems.Count())
Print("\tAllocated: " & listItems.Allocated())
Print("\tContains 'Cruel': " & listItems.Contains("Cruel"))
Print("\tContains 'Decent': " & listItems.Contains("Decent"))
Print("\tIndexOf 'Cruel': " & listItems.IndexOf("Cruel"))
Print("\tIndexOf 'Decent': " & listItems.IndexOf("Decent"))

Print("\nEnumeration:")
For(i as Numeric to listItems.Count())
	Print("\tList[" & i & "]: " & listItems[i])
Next

Print("\nDelete 'Compact'.")
listItems.Compact()

Print("\tCount: " & listItems.Count())
Print("\tAllocated: " & listItems.Allocated())
Print("\tContains 'Cruel': " & listItems.Contains("Cruel"))
Print("\tContains 'Decent': " & listItems.Contains("Decent"))
Print("\tIndexOf 'Cruel': " & listItems.IndexOf("Cruel"))
Print("\tIndexOf 'Decent': " & listItems.IndexOf("Decent"))

Print("\nEnumeration:")
For(i as Numeric to listItems.Count())
	Print("\tList[" & i & "]: " & listItems[i])
Next

Print("\nDeleteAt '0'.")
listItems.DeleteAt(0)

Print("\tCount: " & listItems.Count())
Print("\tAllocated: " & listItems.Allocated())
Print("\tContains 'Cruel': " & listItems.Contains("Cruel"))
Print("\tContains 'Decent': " & listItems.Contains("Decent"))
Print("\tIndexOf 'Cruel': " & listItems.IndexOf("Cruel"))
Print("\tIndexOf 'Decent': " & listItems.IndexOf("Decent"))

Print("\nEnumeration:")
For(i as Numeric to listItems.Count())
	Print("\tList[" & i & "]: " & listItems[i])
Next

%><%HorizontalRule()%>

<% ;---(Test Arrays for Built-in Types)-------------------------------

var strings as string[10 * 10]

strings[0] = "hhh"
strings[0] = "This is a test"
strings[1] = "Hello World"

TestArray(strings)

Print("'" & strings[0] & "'")
Print("'" & strings[1] & "'")
Print("'" & strings[3] & "'") ;Should be empty

var numbers as Numeric[10 * 10]

numbers[0] = 100

numbers[0]--

numbers[0] = 200
numbers[1] = 300

numbers[0]++

numbers[1] += numbers[0]
numbers[1] -= numbers[3]

Print("'" & numbers[0] & "'")
Print("'" & numbers[1] & "'")
Print("'" & numbers[3] & "'") ;Should be empty

Function TestArray(strArray as string[])

	strArray[0] = "Changed by TestArray()"

	Print("'" & strArray[0] & "'")
	Print("'" & strArray[1] & "'")

End Function

%><%HorizontalRule()%>

<% ;---(Horizontal Rule Support Function)-------------------------------
Function HorizontalRule()
	var sLine as String
	For(i as Numeric = 0 to 300)
		sLine.Append("-")
	Next
	Print(sLine & CrLf)
End Function
%>

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
<%="--End of File--" & CrLf%>
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
