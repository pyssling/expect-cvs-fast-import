#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include <initguid.h>
#include <comdef.h>

#include <ObjModel\appauto.h>
#include <ObjModel\appdefs.h>
#include <ObjModel\appguid.h>
#include <ObjModel\dbgauto.h>
#include <ObjModel\dbgguid.h>
#include <ObjModel\dbgdefs.h>

/* local global */
static IApplication *pApp = NULL;

//Uninitialize COM and VC++
void
UnInitializeCOMandVC()
{
    // Quit from Visual C++
    pApp->Quit();
    pApp = NULL;
    // Uninitialize COM libraries
    CoUninitialize();
}


void nameMangledAndTooCPlusPlusishToBeExternC (void)
{
    VARIANT_BOOL visibility = VARIANT_TRUE;
    HRESULT hr;
    CComPtr<IDispatch> iDisp = NULL;

    if (pApp == NULL) {
	// Initialize COM libraries
	hr = CoInitialize(NULL);
	if(FAILED(hr)) {
	    OutputDebugString("Failed to initialize the COM libraries\n");
	    return;
	}
    
	// Obtain the IApplication pointer
	hr = CoCreateInstance(CLSID_Application, NULL, CLSCTX_LOCAL_SERVER,
		IID_IApplication, (void**)&pApp);
	if(FAILED(hr)) {
	    OutputDebugString("Failed to create an instance of MSDEV\n");
	    CoUninitialize();
	    return;
	}

	// Set the visibility of MSDEV to TRUE
	hr = pApp->put_Visible(visibility);
	if(FAILED(hr)) {
	    OutputDebugString("Failed to set visibility of MSDEV\n");
	    //Uninitialize COM libraries and quit from Visual C++
	    UnInitializeCOMandVC();
	    return;
	}

	// Set the current directory
	CComBSTR dir("D:\\expect_wslive\\expect_win32_take2\\win");
	hr = pApp->put_CurrentDirectory(dir);
	if(FAILED(hr)) {
	    OutputDebugString("Failed to set current directory\n");
	    //Uninitialize COM libraries and quit from Visual C++
	    UnInitializeCOMandVC();
	    return;
	}

	// Obtain the IDocuments pointer using smart pointer classes
	pApp->get_Documents(&iDisp);
	CComQIPtr<IDocuments, &IID_IDocuments> pDocs(iDisp);
    
	// Open the slavedrv workspace
	CComBSTR fname("slavedrv.dsp");
	CComVariant type="Auto";
	CComVariant read="False";
	iDisp=NULL;
	hr = pDocs->Open(fname,type,read,&iDisp);
	if(FAILED(hr)) {
	    OutputDebugString("Failed to open the workspace slavedrv.dsp\n");
	    // Uninitialize COM libraries and quit from Visual C++
	    UnInitializeCOMandVC();
	    return;
	}
    }

    // Obtain the IDebugger pointer using smart pointer classes
    pApp->get_Debugger(&iDisp);
    CComQIPtr<IDebugger, &IID_IDebugger> pDebug(iDisp);
    DsExecutionState state;

    pDebug->get_State(&state);
    if (state != dsNoDebugee) pDebug->Stop();
    hr = pDebug->Go();
    if(FAILED(hr)) {
	OutputDebugString("Failed to start the debugger\n");
	// Uninitialize COM libraries and quit from Visual C++
	UnInitializeCOMandVC();
	return;
    }
}

extern "C" void
ExpWinDbgLaunch(void)
{
    __try {
	nameMangledAndTooCPlusPlusishToBeExternC();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
    }
}
