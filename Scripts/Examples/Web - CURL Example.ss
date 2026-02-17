<%
	
	;Can be called from command line to doload the content of a URL.
	
	;Example usage: curl.ss "http://networkdls.com/Software/Release/View/Simple_Scripting_Engine/"
	;Please note that SSE will have to be installed with execute as the default "open with" behavior.
%>
<%
var serverName as String
var uri as String = "/"
var serverPort as Numeric = 80

If(Env.CommandLine.Count() > 1)
	serverName = Env.CommandLine.Get(1)
	If(Env.CommandLine.Count() > 2)
		serverPort = Env.CommandLine.Get(2)
	End If
End If

If(serverName.ToLower().IndexOf("http://") = 0)
	serverName = serverName.SubString(7)
	
	var indexOfSlash as Numeric = serverName.IndexOf("/")
	If(indexOfSlash > 0)
		uri = serverName.SubString(indexOfSlash)
		serverName = serverName.SubString(0, indexOfSlash)
	End If
End If

var server as Socket.Server
server.Start()

var client as Socket.Client
server.Connect(serverName, serverPort, client)

var getHeader as String = _
	$"GET {uri} HTTP/1.1{CrLf}" _
	$"Connection: close{CrLf}" _
	$"Host: {serverName}{CrLf}{CrLf}"

If(client.Send(getHeader))

	While(client.IsConnected() OR client.IsRecvPending())

		var receivedText as String = client.Recv()
		If(receivedText.Length() > 0)
			Print(receivedText)
		End If
	WEnd
End If

client.Disconnect()
server.Stop()
%>
