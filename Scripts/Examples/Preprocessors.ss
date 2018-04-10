<%

#define TEST_STR	"This is a Test!"	
#define TEST_DEC	1024*1024 ;1,048,576
#define TEST_HEX	0xff ;255 in Hex
#define TEST_MATH	((((TEST_HEX*2)+2)/2)*4) ;1,024

Print("String: " & TEST_STR)
Print("Dec: " & TEST_DEC)
Print("Hex: " & TEST_HEX)
Print("Math: " & TEST_MATH)

;#undef TEST_MATH ;TEST_MATH above will not exist if this line is uncommented because it is a preprocessor.

%>