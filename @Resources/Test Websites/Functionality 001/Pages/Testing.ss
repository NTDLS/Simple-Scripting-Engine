<h2>Server Tests</h2>
<hr />

<h3>Error Checking</h3>
<blockquote>
    <a href="/A Bad Link.html">A Bad Link</a><br />
    <a href="/../MyPage.Html">A Directory Traversal</a><br />
</blockquote>

<h3>External Scripting Engines</h3>
<blockquote>
    <a href="/Examples/Scripts/Windows Scripting (No header).vbs">Windows Scripting (No header) Test</a> <i>(disabled by default)</i><br />
    <a href="/Examples/Scripts/Windows Scripting.vbs">Windows Scripting Test</a> <i>(disabled by default)</i><br />
    <a href="/Examples/Scripts/Python.py">Python Test</a> <i>(not installed by default, <a href="http://www.python.org/">Download</a>)</i><br />
    <a href="/Examples/Scripts/PHP Test.php">PHP Test</a> <i>(not installed by default, <a href="http://www.php.net/">Download</a>)</i><br />
    <a href="/Examples/Scripts/Perl Test.Pl">PERL Test</a> <i>(not installed by default, <a href="http://www.perl.org/">Download</a>)</i><br />
    <a href="/Examples/Scripts/SSE.ss">SSE Test</a> <i>(not installed by default, <a href="http://www.networkdls.com/Software.Asp?Review=30">Download</a>)</i><br />
</blockquote>

<h3>(SSI) Server Size Includes</h3>
<blockquote>
    <a href="/Examples/SSI/Echo SSI Test.html">SSI Echo Test</a><br />
    <a href="/Examples/SSI/Extended SSI Test.html">SSI Exec/Include Test</a><br />
</blockquote>

<h3>CGI (Common Gateway Interface)</h3>
<blockquote>
    <a href="/Examples/Executables/ConCGI.exe">Console CGI Test</a><br />
    <a href="/Examples/Executables/WinCGI.exe">Windows CGI Test</a><br />
    <a href="/Examples/Executables/PrintEnvVars.exe">Environment Variables</a><br />
    <a href="/Examples/Executables/PostTest.exe?First=Hello World&Second=Hello Again">Post to CGI Test</a><br />
</blockquote>

<h3>Post Test</h3>
<form method="Post" action="/Examples/Executables/PostTest.exe">
    <table border="0" cellpadding="2" cellspacing="0">
        <tr>
            <td class="DefaultTableHeader" colspan="2"><center><b>Post Test</b></center></td>
        </tr>
        <tr>
            <td class="DefaultTable">Username :</td>
            <td class="DefaultTable"><input type="input" name="UserName" size="17" value="Administrator"></td>
        </tr>
        <tr>
            <td class="DefaultTable">Password :</td>
            <td class="DefaultTable"><input type="password" name="PassWord" size="17" value="MyPass"></td>
        </tr>
        <tr>
            <td class="DefaultTable">&nbsp;</td>
            <td class="DefaultTable"><input type="reset" Value=" Reset "> <input type="submit" value= " Login "></td>
        </tr>
    </table>
<form>

<br /><br />
<!--#include virtual="@PageBottom.Html"-->
