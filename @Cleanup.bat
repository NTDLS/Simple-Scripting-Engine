@Echo Off

set OLDDIR=%CD%

cd Source\Engine\
call Cleanup.bat
chdir /d %OLDDIR%

cd Source\HelpFileParser\
call Cleanup.bat
chdir /d %OLDDIR%

cd Source\IDE\
call Cleanup.bat
chdir /d %OLDDIR%

rd Setup\Output /S /Q
