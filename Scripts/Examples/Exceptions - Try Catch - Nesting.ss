<%

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
			Catch(ex)
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

%>