<%
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; This is a very basic web server implementation in Simple Script. A lot of the script can be
;;  optimized but has been laid out in the following format to showcase scripting functionality.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

Try
	RunServer(8080, "C:\\inetpub\\wwwroot\\", "iisstart.htm")
Catch(ex)
	Print(ex.GetText())
End Try

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

Function RunServer(listenPort as Numeric, rootDirectory as String, defaultPage as String)

	var socketServer as Socket.Server
	
	socketServer.Start(10, listenPort)

	While(True) ;Keep the server running.
		var socketClient as Socket.Client

		If(socketServer.Connection(@socketClient))

			var receivedData as String = socketClient.TryRecv()
			
			If(receivedData.Length() > 0)
					
				var requestedPath as String = GetHttpHeaderRequest(receivedData)
				
				If(requestedPath.Equals("/") or requestedPath.Length() = 0)
					requestedPath = defaultPage
				End If
				
				var localRequestedPath as String = Replace(Replace(rootDirectory & requestedPath, "/", "\\"), "\\\\", "\\")
	
				SendFile(socketClient, localRequestedPath)
		
				socketClient.Disconnect()

				Debug.Evaluate("FormatNumeric(iConnections)")
			End If
		End If
		
		Sleep(1) ;Give CPU time to breathe.
	WEnd

	socketServer.Stop()
	
End Function

Function GetHttpHeaderRequest(receivedData as String) as String

	var requestHeaderStartPosition as Numeric = receivedData.IndexOf("GET ", 0, False)

	If(requestHeaderStartPosition >= 0)
		requestHeaderStartPosition += 4 ;Skip the "GET" tag and the trailing space.
		
		var requestHeaderEndPosition as Numeric = receivedData.IndexOf(" HTTP/", requestHeaderStartPosition, False)

		If(requestHeaderEndPosition >= 0)
			return (Web.URLDecode(receivedData.SubString(requestHeaderStartPosition, requestHeaderEndPosition - requestHeaderStartPosition)))
		End If
	End If

	Return ("/")

End Function

Function SendText(socketClient as Socket.Client, sText as string) as Numeric

	If(SendHeader(socketClient, "text/html", sText.Length()))
		Return(socketClient.Send(sText))
	End If

End Function

Function SendFile(socketClient as Socket.Client, sFileName as String)

	Try
		var fileObject as File
			
		var fileExtension as String = File.Extension(sFileName)
		var contentType as String = "application/octet-stream"
		
		If(fileExtension.Equals(".htm") or fileExtension.Equals(".html"))
			contentType = "text/html"
		Else If(fileExtension.Equals(".gif"))
			contentType = "image/gif"
		Else If(fileExtension.Equals(".png"))
			contentType = "image/png"
		Else If(fileExtension.Equals(".jpg"))
			contentType = "image/jpeg"
		Else If(fileExtension.Equals(".jpeg"))
			contentType = "image/pjpeg"
		Else If(fileExtension.Equals(".css"))
			contentType = "text/css"
		End If
			
		If(fileObject.Open(sFileName, FileAccess.Read))
			var fileData as String = fileObject.Read()
		
			If(SendHeader(socketClient, contentType, fileData.Length()))
				socketClient.Send(fileData)

				While(socketClient.IsSendPending())
					;Print("Waiting on data to be sent")
					Sleep(1)
				WEnd
			End If
			
			fileObject.Close()
		End If
	Catch(ex)
		socketClient.Send(ex.GetText())
	End Try
	
End Function

Function SendHeader(socketClient as Socket.Client, contentType as String, contentLength as Numeric) as Numeric

	var httpHeader as String
	
	httpHeader.Append("HTTP/1.0 200 OK" & "\n")
	httpHeader.Append("Content-Type: " & contentType & "\n")
	httpHeader.Append("Content-Length: " & contentLength & "\n")
	httpHeader.Append("Connection: Keep-Alive\n")
	httpHeader.Append(CrLf)

	socketClient.Send(httpHeader)
	
	Return(True)
	
End Function

%>
