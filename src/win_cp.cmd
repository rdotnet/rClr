@echo off
REM This utility uses the native windows xcopy to overcome an issue that can arise with 'cp' coming with MinGW.
set unix_args=%*
set dos_paths=%unix_args:/=\%
robocopy /MT:1 /R:2 /NJS /NJH %dos_paths%
@set exit_code=0

REM http://ss64.com/nt/robocopy-exit.html
@if errorlevel 0 goto exit
@if errorlevel 1 goto exit

goto robocopy_fail

:robocopy_fail
@echo ERROR: robocopy exit code %errorlevel%. See http://ss64.com/nt/robocopy-exit.html for details.
@set exit_code=1
@goto exit

:exit
exit /b %exit_code%

