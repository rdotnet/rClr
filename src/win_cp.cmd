@echo off
REM This utility uses the native windows xcopy to overcome an issue that can arise with 'cp' coming with MinGW.
set unix_args=%*
set dos_paths=%unix_args:/=\%
@echo robocopy %dos_paths%
robocopy %dos_paths%
