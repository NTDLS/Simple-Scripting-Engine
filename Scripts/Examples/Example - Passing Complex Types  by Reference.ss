<%
;;
;; Please understand that this script was intentionally written poorly to
;;		demonstrate the possibility of passing a complex type multiple times.
;;

#define _SERVER_NAME "www.NetworkDLS.com"
#define _SERVER_PORT 80

var socketServer as Socket.Server
var socketClient as Socket.Client

MakeRequest(socketServer, socketClient)

socketServer.Stop()

Function DoConnect(socketServer as Socket.Server, socketClient as Socket.Client)

	socketServer.Start()
	socketServer.Connect(_SERVER_NAME, _SERVER_PORT, socketClient)
	
End Function

Function MakeRequest(socketServer as Socket.Server, socketClient as Socket.Client)

	DoConnect(socketServer, socketClient)

	var sHeader as String = _
		"GET / HTTP/1.1" & CrLf & _
		"Connection: close" & CrLf & _
		"Host: " & _SERVER_NAME & CrLf & _
		CrLf
	
	If(socketClient.Send(sHeader))
		While(socketClient.IsConnected() OR socketClient.IsRecvPending())
			var ReceivedText as String = socketClient.Recv()
			If(ReceivedText.Length() > 0)
				Print(ReceivedText)
			End If
		WEnd
	End If
	
	socketClient.Disconnect()

End Function

%>
