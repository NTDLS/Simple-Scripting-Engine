<%

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

%>