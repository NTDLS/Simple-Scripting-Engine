<%
#ContentType "text/html"

#Include "Library/Config.ss"
#Include "Library/Routines.ss"
#Include "Library/PageHeader.ss"

If(Env.Get("QUERY_STRING").Equals("Home"))
	#Include "Pages/Home.ss"
Else If(Env.Get("QUERY_STRING").Equals("Copyrights"))
	#Include "Pages/Copyrights.ss"
Else If(Env.Get("QUERY_STRING").Equals("EULA"))
	#Include "Pages/EULA.ss"
Else If(Env.Get("QUERY_STRING").Equals("FAQ"))
	#Include "Pages/FAQ.ss"
Else If(Env.Get("QUERY_STRING").Equals("Features"))
	#Include "Pages/Features.ss"
Else If(Env.Get("QUERY_STRING").Equals("PreEULA"))
	#Include "Pages/PreEULA.ss"
Else If(Env.Get("QUERY_STRING").Equals("PrivacyPolicy"))
	#Include "Pages/PrivacyPolicy.ss"
Else If(Env.Get("QUERY_STRING").Equals("Security"))
	#Include "Pages/Security.ss"
Else If(Env.Get("QUERY_STRING").Equals("Testing"))
	#Include "Pages/Testing.ss"
Else
	#Include "Pages/Signup.ss"
End If

#Include "Library/PageFooter.ss"
%>
