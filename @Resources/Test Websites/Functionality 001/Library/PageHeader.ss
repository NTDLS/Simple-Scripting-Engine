<html>
	<head>
        <title>Home Page</title>
        <link rel="stylesheet" type="text/css" href="/Style/Default.css">
        <meta http-equiv="content-language" content="en">
        <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
        <meta name="author" content="NetworkDLS">
	</head>
	<body >
		<table border="0" cellpadding="0" cellspacing="0" height="400" width="100%">
			<tr height="30">
				<td align="bottom" valign="left" bgcolor="#c2cbcf"><img src="/Images/Logo.png"></td>
			</tr>
			<tr height="1">
				<td align="bottom" valign="left" bgcolor="#000000"><img src="/Images/pixil.gif"></td>
			</tr>
			<tr>
				<td align="left" valign="top">
                    <table border="0" cellpadding="0" cellspacing="0" width="100%" height="100%">
                        <tr>
                            <td align="Left" valign="top">
                                <table border="0" cellpadding="0" cellspacing="0" height="100%" width="100%">
                                    <tr>
                                        <td valign="top" Width="140">
                                            <table border="0" cellpadding="5" cellspacing="0">
                                                <tr>
                                                    <td Width="100%">
                                                        <%#Include "Library/Menu.ss"%>
                                                    </td>
                                                </tr>
                                            </table>
                                        </td>
                                        <td valign="top" bgcolor="#000000" Width="1"></td>
                                        <td valign="top">
											&nbsp;<font size="3"><%=Engine.Name%> v<%=Engine.Version%> on <!--#echo var="SERVER_SOFTWARE"--> v<!--#echo var="SERVER_VERSION"--> (Build: <%=Engine.Build%>)</font>
                                            <table border="0" cellpadding="0" cellspacing="0" width="100%">
                                                <tr>
                                                    <td>
