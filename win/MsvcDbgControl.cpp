#include "tclPort.h"
extern "C" {
    #include "expWin.h"
}

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


CRITICAL_SECTION cs;
CRITICAL_SECTION *pcs = 0L;

//Uninitialize COM and VC++
static void
VCDbgClose(void *token)
{
    IApplication *pApp = reinterpret_cast<IApplication *>(token);
    // Quit from Visual C++
    pApp->Quit();
    pApp = 0L;
    // Uninitialize COM libraries
    CoUninitialize();
}

static char *
bstr2utf8(BSTR bstr)
{
    int size, len = SysStringLen(bstr);
    char *buf;

    size = WideCharToMultiByte(CP_UTF8, 0, bstr, len, 0, 0, NULL, NULL);
    buf = new char [size+1];
    len = WideCharToMultiByte(CP_UTF8, 0, bstr, len, buf, size, NULL, NULL);
    *(buf+len) = '\0';
    return buf;
}

static int
nameMangledAndTooCPlusPlusishToBeExternC
    (const CHAR *wrkspace, Tcl_DString *cmdline, void **token)
{
    IApplication *pApp = reinterpret_cast<IApplication *>(*token);
    VARIANT_BOOL visibility = VARIANT_TRUE;
    HRESULT hr;
    DWORD dwRet;
    CComPtr<IDispatch> iDisp = NULL;
    size_t i;
    int thePid = -1;

    if (pApp == 0L) {
	// Initialize COM libraries
	hr = CoInitialize(NULL);
	if(FAILED(hr)) {
	    OutputDebugString("Failed to initialize the COM libraries\n");
	    return thePid;
	}
    
	// Obtain the IApplication pointer
	hr = CoCreateInstance(CLSID_Application, NULL, CLSCTX_LOCAL_SERVER,
		IID_IApplication, token);
	if(FAILED(hr)) {
	    OutputDebugString("Failed to create an instance of MSDEV\n");
	    CoUninitialize();
	    return thePid;
	}

	// reset pApp.  Only a convenience.
	pApp = reinterpret_cast<IApplication *>(*token);

	// Set the visibility of MSDEV to TRUE
	hr = pApp->put_Visible(visibility);
	if(FAILED(hr)) {
	    OutputDebugString("Failed to set visibility of MSDEV\n");
	    //Uninitialize COM libraries and quit from Visual C++
	    VCDbgClose(*token);
	    return thePid;
	}

	for (i = (strlen(wrkspace) - 1);; i--) {
	    if (wrkspace[i] == '\\') {
		break;
	    }
	}
	i++;

	// Set the current directory
	CComBSTR dir(i, wrkspace);
	hr = pApp->put_CurrentDirectory(dir);
	if(FAILED(hr)) {
	    OutputDebugString("Failed to set current directory\n");
	    //Uninitialize COM libraries and quit from Visual C++
	    VCDbgClose(*token);
	    return thePid;
	}

	// Obtain the IDocuments pointer using smart pointer classes
	pApp->get_Documents(&iDisp);
	CComQIPtr<IDocuments, &IID_IDocuments> pDocs(iDisp);
    
	// Open the slavedrv workspace
	CComBSTR fname(wrkspace+i);
	CComVariant type="Auto";
	CComVariant read="False";
	iDisp=NULL;
	hr = pDocs->Open(fname,type,read,&iDisp);
	if(FAILED(hr)) {
	    OutputDebugString("Failed to open the workspace\n");
	    // Uninitialize COM libraries and quit from Visual C++
	    VCDbgClose(*token);
	    return thePid;
	}
    }

    // Obtain the IDebugger pointer.
    pApp->get_Debugger(&iDisp);
    CComQIPtr<IDebugger, &IID_IDebugger> pDebug(iDisp);

    if (pcs == 0L) {
	InitializeCriticalSection(pcs = &cs);
    }

    // We need to syncronize entry as we're using a kernel event by name.
    EnterCriticalSection(&cs);

    // Create the event we'll block for.
    HANDLE event1 = CreateEvent(0L, FALSE, FALSE, "SpawnStartup");

    hr = pDebug->Go();
    if(FAILED(hr)) {
	OutputDebugString("Failed to start the debugger\n");
	// Uninitialize COM libraries and quit from Visual C++
	VCDbgClose(pApp);
	goto end;
    }

    // Wait forever as we might need to rebuild or some such.
    // Just because Go() returned doesn't mean it's running...
    dwRet = WaitForSingleObject(event1, INFINITE);
    if (dwRet != WAIT_OBJECT_0) goto end;

    // Just to make sure AppB hit the soft break.
    Sleep(300);

    // Now that the child hit the soft break, ask it what its pid
    // global variable has in it.
    {
	CComBSTR askpid("pid");
	CComBSTR gotpid;
	Tcl_DString keyDS;
	char *keyName;
	HKEY root;

	hr = pDebug->Evaluate(askpid, &gotpid);
	thePid = _wtoi(gotpid.m_str);
	keyName = bstr2utf8(gotpid);
	Tcl_DStringInit(&keyDS);
	Tcl_WinUtfToTChar(keyName, -1, &keyDS);
	delete keyName;

	// associate the pid we just got from AppB to the commandline
	// and when AppB asks us for it, we know what to send in return.

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, "Software\\Tomasoft\\MsDevDbgCtrl",
		0, REG_NONE, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, 0L, &root,
		0L);

	(*expWinProcs->regSetValueExProc)(root, Tcl_DStringValue(&keyDS), 0, REG_SZ,
		(BYTE *) Tcl_DStringValue(cmdline),
		Tcl_DStringLength(cmdline));

	RegFlushKey(root);
	RegCloseKey(root);
	Tcl_DStringFree(&keyDS);
    }
    
    // continue AppB from the soft break.
    hr = pDebug->Go();

end:
    CloseHandle(event1);
    LeaveCriticalSection(&cs);
    return thePid;
}

extern "C" int
MsvcDbg_Launch(const CHAR *wrkspace, Tcl_DString *cmdline, void **token)
{
    // Protect from all the COM wierdness taken us down
    __try {
	return nameMangledAndTooCPlusPlusishToBeExternC(wrkspace, cmdline, token);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
	return -1;
    }
}

extern "C" WCHAR *
MsvcDbg_GetCommandLine(void)
{
    HKEY root;
    HANDLE event1;
    WCHAR pidChar[33], *buf;
    DWORD type = REG_SZ, size;
    LONG ret;
    int pid;     // <- this is read by the parent's debugger.

    pid = GetCurrentProcessId();

    event1 = CreateEvent(0L, FALSE, FALSE, "SpawnStartup");
    SetEvent(event1);
    CloseHandle(event1);

    // >>>>   IMPORTANT!   <<<<

    // Set a soft break on the next line for this to work.
    // It is essential that the app stops here during startup
    // and syncs to the parent properly.
    __asm nop;

    // >>>> END IMPORTANT! <<<<

    _itow(pid, pidChar, 10);
    ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Tomasoft\\MsDevDbgCtrl", 0, KEY_ALL_ACCESS, &root);
    size = 0;
    ret = RegQueryValueExW(root, pidChar, 0, &type, 0L, &size);
    buf = (WCHAR *) HeapAlloc(GetProcessHeap(), 0, size);
    ret = RegQueryValueExW(root, pidChar, 0, &type, (LPBYTE)buf, &size);
    ret = RegDeleteValueW(root, pidChar);
    RegCloseKey(root);
    return buf;
}
