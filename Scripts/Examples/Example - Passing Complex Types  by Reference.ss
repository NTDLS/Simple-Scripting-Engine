<%
;;
;; Please understand that this script was intentionally written poorly to
;;		demonstrate the possibility of passing a complex type multiple times.
;;

#define _SERVER_NAME "www.NetworkDLS.com"
#define _SERVER_PORT 80

var gServer as Socket.Server
var gClient as Socket.Client

MakeRequest(gServer, gClient)

gServer.Stop()

Function DoConnect(server as Socket.Server, client as Socket.Client)

	server.Start()
	server.Connect(_SERVER_NAME, _SERVER_PORT, client)
	
End Function

Function MakeRequest(server as Socket.Server, client as Socket.Client)

	DoConnect(server, client)

	var sHeader as String = _
		"GET / HTTP/1.1" & CrLf & _
		"Connection: close" & CrLf & _
		"Host: " & _SERVER_NAME & CrLf & _
		CrLf
	
	If(client.Send(sHeader))
		While(client.IsConnected() OR client.IsRecvPending())
			var ReceivedText as String = client.Recv()
			If(ReceivedText.Length() > 0)
				Print(ReceivedText)
			End If
		WEnd
	End If
	
	client.Disconnect()

End Function

%>
