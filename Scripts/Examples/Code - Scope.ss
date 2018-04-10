<%

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

%>