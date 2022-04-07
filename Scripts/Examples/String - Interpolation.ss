<%

var sHelloString as String = "Hello World"

;Strings which are prefixed with a '$' will b parsed for variable names
;	inclosed in curly brackets. Any found variables will be inlined.
Print($"Is it customary for an application to say {sHelloString} in an example!")

%>
