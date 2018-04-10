<%

Print("File: \"" & File.Name(Code.File()) & "\"")

;Print("Global: " & Global) ;Not defined until #include is reached.

Print("Before " & Add(10, 15)) ;Functions in included files are always available.
#Include "Example - Include - SubFile.ss"
Print("After " & Add(10, 15)) ;Functions in included files are always available.

Print("Global: " & Global)


%>