@echo OFF
REM creates .lib files required for compiling against mono-2.0.dll and R.dll

@set THIS_DIR=%~d0%~p0

REM is there a return code for errors from cmd executions?
REM @if not exist %THIS_DIR%setup_vcpp.cmd xcopy %THIS_DIR%setup_vcpp.in %THIS_DIR%setup_vcpp.cmd /Y /R
@call %THIS_DIR%setup_vcpp.cmd

@set LIBDIR32=%THIS_DIR%libfiles\i386
@set LIBDIR64=%THIS_DIR%libfiles\x64
@if not exist %LIBDIR32% mkdir %LIBDIR32%
@if not exist %LIBDIR64% mkdir %LIBDIR64%

@set LIB_EXE=lib

%LIB_EXE% /nologo /def:%THIS_DIR%R.def /out:%LIBDIR64%\Rdll.lib /machine:x64
%LIB_EXE% /nologo /def:%THIS_DIR%R.def /out:%LIBDIR32%\Rdll.lib /machine:x86

%LIB_EXE% /nologo /def:%THIS_DIR%mono.def /out:%LIBDIR32%\mono-2.0.lib /machine:x86
