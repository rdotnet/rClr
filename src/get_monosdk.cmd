@REM ===============================================
@REM Query the windows registry to get the installation folder for the Mono SDK
@REM ===============================================
@set defaultClrVersion=
@set MonoHKey32bitsWin7=HKEY_LOCAL_MACHINE\SOFTWARE\Novell\Mono
@set monoSDKPath=
@set MonoHKey64bitsWin7=HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Novell\Mono
@REM TODO check on a Win server

@set defaultClrVersion=
@call :GetDefaultMonoCLRVersion %MonoHKey32bitsWin7% > nul 2>&1
@if errorlevel 1 call :GetDefaultMonoCLRVersion %MonoHKey64bitsWin7% > nul 2>&1
@REM if "%defaultClrVersion%" == "" echo "DefaultCLR not found"
@if "%defaultClrVersion%" == "" goto install_path_not_found

@set monoSDKPath=
@call :GetSdkInstallRoot %MonoHKey32bitsWin7% > nul 2>&1
@if errorlevel 1 call :GetSdkInstallRoot %MonoHKey64bitsWin7% > nul 2>&1

@if "%monoSDKPath%" == "" goto install_path_not_found

@REM It proved easier to substitute in DOS than with ash+sed. Sigh.
@set monoSDKPath=%monoSDKPath:\=/%
@echo %monoSDKPath%
@goto end

:GetDefaultMonoCLRVersion
for /F "tokens=1,2*" %%i in ('reg query %1 /v "DefaultCLR"') DO (
	if "%%i"=="DefaultCLR" (
		@SET "defaultClrVersion=%%k" 
		)
)
@if "%defaultClrVersion%"=="" exit /B 1
@exit /B 0

:GetSdkInstallRoot
for /F "tokens=1,2*" %%i in ('reg query %1\%defaultClrVersion% /v "SdkInstallRoot"') DO (
	if "%%i"=="SdkInstallRoot" (
		@SET "monoSDKPath=%%k" 
		)
)
@if "%monoSDKPath%"=="" exit /B 1
@exit /B 0

:install_path_not_found
@echo not found
@goto end

:end
