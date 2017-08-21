#include "rClr_windows_dotnet.h"
#include "rClr.h"

#ifdef MS_CLR

void start_ms_clr() {
    HRESULT hr;
    hr = CLRCreateInstance( CLSID_CLRMetaHost, IID_PPV_ARGS( &pMetaHost ) );
    if (FAILED( hr ))
    {
        error( "CLRCreateInstance failed" );
    }
    hr = pMetaHost->GetRuntime( pszVersion, IID_PPV_ARGS( &pRuntimeInfo ) );
    if (FAILED( hr ))
    {
        error( "ICLRMetaHost::GetRuntime failed" );
    }
    BOOL fLoadable;
    hr = pRuntimeInfo->IsLoadable( &fLoadable );
    if (FAILED( hr ))
    {
        error( "ICLRRuntimeInfo::IsLoadable failed" );
    }

    if (!fLoadable)
    {
        error( ".NET runtime cannot be loaded" );
    }
    hr = pRuntimeInfo->GetInterface( CLSID_CLRRuntimeHost,
        IID_PPV_ARGS( &pClrRuntimeHost ) );
    if (FAILED( hr ))
    {
        error( "ICLRRuntimeInfo::GetInterface failed w/hr 0x%08lx\n", hr );
        goto Cleanup;
    }

    // Start the CLR.
    hr = pClrRuntimeHost->Start();
    if (FAILED( hr ))
    {
        error( "CLR failed to start w/hr 0x%08lx\n", hr );
        goto Cleanup;
    }
    return;
Cleanup:
    ms_rclr_cleanup();
}

void ms_rclr_cleanup()
{
    if (pMetaHost)
    {
        pMetaHost->Release();
        pMetaHost = NULL;
    }
    if (pRuntimeInfo)
    {
        pRuntimeInfo->Release();
        pRuntimeInfo = NULL;
    }
    if (pClrRuntimeHost)
    {
        // Please note that after a call to Stop, the CLR cannot be 
        // reinitialized into the same process. This step is usually not 
        // necessary. You can leave the .NET runtime loaded in your process.
        //error("Stop the .NET runtime\n");
        //pClrRuntimeHost->Stop();

        pClrRuntimeHost->Release();
        pClrRuntimeHost = NULL;
    }
}

#endif