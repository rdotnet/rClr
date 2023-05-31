@echo off

REM -------------------------------------
REM NOTE: This file is a copy of 
REM https://github.com/jmp75/config-utils/blob/master/R/packages/msvs/exec/setup_vcpp.cmd
REM -------------------------------------

@set exit_code=0


REM load Visual Studio 2017 developer command prompt setup has changed compared to previous versions. 
REM Inspired from: https://github.com/ctaggart  via https://github.com/Microsoft/visualfsharp/pull/2690/commits/bf52776167fe6a9f2354ea96094a025191dbd3e7

set VsDevCmdFile=\Common7\Tools\VsDevCmd.bat

set progf=%ProgramFiles%\Microsoft Visual Studio\2022\
if exist "%progf%" (
    goto foundVsDevCmdFile
)

set progf=%ProgramFiles(x86)%\Microsoft Visual Studio\2017\

:foundVsDevCmdFile

if exist "%progf%Enterprise%VsDevCmdFile%" (
    call "%progf%Enterprise%VsDevCmdFile%"
    goto end
)
if exist "%progf%Professional%VsDevCmdFile%" (
    call "%progf%Professional%VsDevCmdFile%"
    goto end
)
if exist "%progf%Community%VsDevCmdFile%" (
    call "%progf%Community%VsDevCmdFile%"
    goto end
)
if exist "%progf%BuildTools%VsDevCmdFile%" (
    call "%progf%BuildTools%VsDevCmdFile%"
    goto end
)

REM for instance C:\bin\VS2012\Common7\Tools\
if defined VSCOMNTOOLS (
    goto found
)

if defined VS140COMNTOOLS set VSCOMNTOOLS=%VS140COMNTOOLS%
if defined VS140COMNTOOLS goto found
if defined VS120COMNTOOLS set VSCOMNTOOLS=%VS140COMNTOOLS%
if defined VS120COMNTOOLS goto found
if defined VS110COMNTOOLS set VSCOMNTOOLS=%VS140COMNTOOLS%
if defined VS110COMNTOOLS goto found

@echo ERROR: Could not locate command prompt devenv setup for anything between VS2012 and VS2017
@set exit_code=127
@goto end

:found
set VSDEVENV=%VSCOMNTOOLS%..\..\VC\vcvarsall.bat
@if not exist "%VSDEVENV%" goto error_no_vcvarsall
@call "%VSDEVENV%"
@goto end

:error_no_VS110COMNTOOLSDIR
@echo ERROR: setup_vcpp cannot determine the location of the VS Common Tools folder.
@set exit_code=1
@goto end

:error_no_vcvarsall
@echo ERROR: Cannot find file %VSDEVENV%.
@set exit_code=1
@goto end

:end
exit /b %exit_code%

