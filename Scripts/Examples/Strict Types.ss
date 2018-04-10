<%

; Strict Types are compound types that were implemented to serve as a byte-width compatible
;	alternative to C/C++ structures. They are required when calling lower-level system APIs
;	such as GetSystemTime in Kernel32.dll

StrictType TypeTest
	xBoolean as StrictType.Boolean
	xByte as StrictType.Byte
	xChar as StrictType.Char
	xDouble as StrictType.Double
	xFloat as StrictType.Float
	xHandle as StrictType.Handle
	xInt as StrictType.Int
	xInt16 as StrictType.Int16
	xInt32 as StrictType.Int32
	xInt64 as StrictType.Int64
	xInteger as StrictType.Integer
	xLong as StrictType.Long
	xShort as StrictType.Short
	xString as StrictType.String
	xuInt16 as StrictType.uInt16           
	xuInt32 as StrictType.uInt32
	xuInt64 as StrictType.uInt64
End StrictType

var typeTest as TypeTest

typeTest.xBoolean = true
typeTest.xByte = 4
typeTest.xChar = 40
typeTest.xDouble = 123
typeTest.xFloat = 456
typeTest.xHandle = 789
typeTest.xInt = 101112
typeTest.xInt16 = 13
typeTest.xInt32 = 14
typeTest.xInt64 = 15
typeTest.xInteger = 16
typeTest.xLong = 171819
typeTest.xShort = 20
typeTest.xString = "this is a string"
typeTest.xuInt16 = 99
typeTest.xuInt32 = 88
typeTest.xuInt64 = 16

Print("xBoolean: " & typeTest.xBoolean & ", Length :" & DataLen(typeTest.xBoolean))
Print("xByte:" & typeTest.xByte & ", Length :" & DataLen(typeTest.xByte))
Print("xChar: " & typeTest.xChar & ", Length :" & DataLen(typeTest.xChar))
Print("xDouble:" & typeTest.xDouble & ", Length :" & DataLen(typeTest.xDouble))
Print("xFloat:" & typeTest.xFloat & ", Length :" & DataLen(typeTest.xFloat))
Print("xHandle:" & typeTest.xHandle & ", Length :" & DataLen(typeTest.xHandle))
Print("xInt:" & typeTest.xInt & ", Length :" & DataLen(typeTest.xInt))
Print("xInt16:" & typeTest.xInt16 & ", Length :" & DataLen(typeTest.xInt16))
Print("xInt32: " & typeTest.xInt32 & ", Length :" & DataLen(typeTest.xInt32))
Print("xInt64:" & typeTest.xInt64 & ", Length :" & DataLen(typeTest.xInt64))
Print("xInteger:" & typeTest.xInteger & ", Length :" & DataLen(typeTest.xInteger))
Print("xLong:" & typeTest.xLong & ", Length :" & DataLen(typeTest.xLong))
Print("xShort:" & typeTest.xShort & ", Length :" & DataLen(typeTest.xShort))
Print("xString: " & typeTest.xString & ", Length :" & DataLen(typeTest.xString))
Print("xuInt16:" & typeTest.xuInt16 & ", Length :" & DataLen(typeTest.xuInt16))
Print("xuInt32: " & typeTest.xuInt32 & ", Length :" & DataLen(typeTest.xuInt32))
Print("xuInt64: " & typeTest.xuInt64 & ", Length :" & DataLen(typeTest.xuInt64))
%>