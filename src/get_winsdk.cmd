@echo OFF
REM ===============================================
REM Query the windows registry to get the installation folder for the Windows SDK
REM ===============================================
set WindowsSdkDir=""
for /F "tokens=1,2*" %%i in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Microsoft SDKs\Windows\v8.0" /v "InstallationFolder"') DO (
	if "%%i"=="InstallationFolder" (
		@SET "WindowsSdkDir=%%k" 
		)
)
@echo %WindowsSdkDir%