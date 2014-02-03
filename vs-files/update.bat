REM REM is comments
REM USAGE: Open Command Prompt. From ArrayFire-Examples folder:
REM ArrayFire-Examples> .\vs-files\update.bat

@echo off

REM List full path
REM for /R %%f in (*.cpp) do echo %%f
REM List only filename without extension
REM for /R %%f in (*.cpp) do echo %%~nf
REM List only filename with extension
REM for /R %%f in (*.cpp) do echo %%~nxf
REM List only relative
REM for /R %%f in (*.cpp) do echo %%~pf

set VSDIR=%~dp0
set SEARCH=XXX

REM Copy Consolidated Solutions
copy /y %VSDIR%AF_Examples_vs*.sln .\

for /R %%f in (*.cpp) do (
    echo %%f
    PUSHD %%~pf
    copy /y %VSDIR%vs2012.sln .\%%~nf_vs2012.sln
    copy /y %VSDIR%vs2010.sln .\%%~nf_vs2010.sln
    copy /y %VSDIR%vs2008.sln .\%%~nf_vs2008.sln
    copy /y %VSDIR%vs2012.vcxproj .\%%~nf_vs2012.vcxproj
    copy /y %VSDIR%vs2010.vcxproj .\%%~nf_vs2010.vcxproj
    copy /y %VSDIR%vs2008.vcproj .\%%~nf_vs2008.vcproj
    %VSDIR%\fnr.exe %SEARCH% %%~nf .\%%~nf_vs2012.sln
    %VSDIR%\fnr.exe %SEARCH% %%~nf .\%%~nf_vs2010.sln
    %VSDIR%\fnr.exe %SEARCH% %%~nf .\%%~nf_vs2008.sln
    %VSDIR%\fnr.exe %SEARCH% %%~nf .\%%~nf_vs2012.vcxproj
    %VSDIR%\fnr.exe %SEARCH% %%~nf .\%%~nf_vs2010.vcxproj
    %VSDIR%\fnr.exe %SEARCH% %%~nf .\%%~nf_vs2008.vcproj
    POPD
)
