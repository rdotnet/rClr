@if "%VS110COMNTOOLS%"=="" goto error_no_VS110COMNTOOLSDIR
REM for instance C:\bin\VS2012\Common7\Tools\
set VSDEVENV="%VS110COMNTOOLS%..\..\VC\vcvarsall.bat"
@if not exist "%VSDEVENV%" goto error_no_vcvarsall
@call %VSDEVENV%
@goto end

:error_no_VS110COMNTOOLSDIR
@echo ERROR: setup_vcpp: cannot determine the location of the VS Common Tools folder.
@goto end

:error_no_vcvarsall
@echo ERROR: Cannot find file %VSDEVENV%.
@goto end

:end

