<h2>Securing the Server</h2>
<hr />

<H3>Extension Scanning tips</H3>
<UL>
    <LI><B>Deny executables that could run on the server</B></LI>
    <UL>
        <LI>.exe (Exe executable file)</LI>
        <LI>.bat (Executable batch file)</LI>
        <LI>.cmd (Windows NT Command Script)</LI>
        <LI>.com (MS-DOS Application)</LI>
    </UL>
</UL>

<UL>
    <LI><B>Deny infrequently used scripts</B></LI>
    <UL>
        <LI>.htw (Maps to webhits.dll, part of Index Server)</LI>
        <LI>.ida (Maps to idq.dll, part of Index Server)</LI>
        <LI>.idq (Maps to idq.dll, part of Index Server)</LI>
        <LI>.htr (Maps to ism.dll, a legacy administrative tool)</LI>
        <LI>.idc (Maps to httpodbc.dll, a legacy database access tool)</LI>
        <LI>.shtm (Maps to ssinc.dll, for Server Side Includes)</LI>
        <LI>.html (Maps to ssinc.dll, for Server Side Includes)</LI>
        <LI>.stm (Maps to ssinc.dll, for Server Side Includes)</LI>
        <LI>.printer (Maps to msw3prt.dll, for Internet Printing Services)</LI>
    </UL>
</UL>

<UL>
    <LI><B>Various static files</B></LI>
    <UL>
        <LI>.ini (Don't allow access to Configuration files)</LI>
        <LI>.log (Don't allow access to Log files)</LI>
        <LI>.pol (Don't allow access to Policy files)</LI>
        <LI>.dat (Don't allow access to Configuration files)</LI>
    </UL>
</UL>

<H3><B>URL Scanning Tips</B></H3>

<UL>
    <LI><B>URL Scanning</B></LI>
    <UL>
        <LI>.. (Don't allow directory traversals)</LI>
        <LI>/. (Don't allow trailing dot after a directory name)</LI>
        <LI>./ (Don't allow trailing dot on a directory name)</LI>
        <LI>\ (Don't allow backslashes in URL)</LI>
        <LI>: (Don't allow alternate stream access)</LI>
        <LI>%% (Don't allow escaping after normalization)</LI>
        <LI>&amp; (Don't allow multiple CGI processes to run on a single request)</LI>
    </UL>
</UL>
