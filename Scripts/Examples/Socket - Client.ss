<%
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; This is a very basic web client implementation in Simple Script. Alot of the script can be
;;  optimized but has been laid out in the following format to showcase scripting functionality.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

Function MakeHTTPRequest(sHost as String, sRequest as String) as String
	
	var Server as Socket.Server
	var Client as Socket.Client
	
	Server.Start()
	
	Server.Connect(sHost, 80, @Client)

	If(Client.Send("GET " & sRequest & " HTTP/1.0" & CrLf & CrLf))
		While(Client.IsConnected() or Client.IsRecvPending())
			var ReceivedText as String = Client.TryRecv()
			If(ReceivedText.Length() > 0)
				Return(ReceivedText)
			End If
			Sleep(1)
		WEnd
		Client.Disconnect()
	End If
	
	Server.Stop()
	
	Return("")
	
End Function

Function ReportError(ex as Exception)
	Debug.Evaluate("\"Error Time: \" & Date.Now() & \" \" & Time.Now()")
	Debug.Evaluate("\"Error Text: \" & ex.GetText()")
	Debug.Evaluate("\"Error File: \" & ex.GetFile()")
	Debug.Evaluate("\"Error Line: \" & ex.GetLine()")
End Function

var iBytesReceived as Numeric

For(iHits as Numeric = 0 to 10000)

	Try
		iBytesReceived += Length(MakeHTTPRequest("www.NetworkDLS.com", "/"))
	Catch(ex)
		ReportError(ex)
	End Try

	Debug.Evaluate("FormatSize(iBytesReceived, 2)")
Next
%>
