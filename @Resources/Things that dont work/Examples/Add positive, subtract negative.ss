
<%

;Print(1--1) ;Broken: "Failed to parse inline increment or decrement"
Print(1 - -1) ;Works properly

;Print(1++1) ;Broken: "Failed to parse inline increment or decrement"
Print(1+ +1) ;Works properly

Print(1-+1) ;Works properly
Print(1+-1) ;Works properly

var i as Numeric = 67

print(i++10) ;Broken: The figures 67 and 10 are appended resulting in "6710"
print(i--10) ;Broken: The figures 68 and 10 are appended resulting in "6810". Note that 67 was incremented.

%>
