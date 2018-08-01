<%

var testString as String = "Test1"

Code.Scope.Enter()
var testString as String = "Test2"
Print("testString: " & testString)
Code.Scope.Exit()

Code.Scope.Enter()
var testString as String = "Test3"
Print("testString: " & testString)
Code.Scope.Exit()

Print("testString: " & testString)

%>