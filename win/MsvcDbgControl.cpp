/* ----------------------------------------------------------------------------
 * MsvcDbgControl.cpp --
 *
 *	Debugger friendly replacements for CreateProcess() on the parent side
 *	and GetCommandLine() on the child side.
 *
 *	This stuff may not be perfect, but the intent is to avoid all
 *	the manual-ness of having to set a specific commandline by hand
 *	while debugging the system.
 *
 *	See the note on line 259 for needing to set a soft break on the
 *	child side.
 *
 * ----------------------------------------------------------------------------
 *
 * Written by: Don Libes, libes@cme.nist.gov, NIST, 12/3/90
 * 
 * Design and implementation of this program was paid for by U.S. tax
 * dollars.  Therefore it is public domain.  However, the author and NIST
 * would appreciate credit if this program or parts of it are used.
 * 
 * Copyright (c) 1997 Mitel Corporation
 *	work by Gordon Chaffee <chaffee@bmrc.berkeley.edu> for the WinNT port.
 *
 * Copyright (c) 2001-2002 Telindustrie, LLC
 *	work by David Gravereaux <davygrvy@pobox.com> for any Win32 OS.
 *
 * ----------------------------------------------------------------------------
 * URLs:    http://expect.sf.net/
 *	    http://expect.nist.gov/
 *	    http://bmrc.berkeley.edu/people/chaffee/expectnt.html
 * ----------------------------------------------------------------------------
 * RCS: @(#) $Id: MsvcDbgControl.cpp,v 1.1.2.6 2002/03/12 04:37:39 davygrvy Exp $
 * ----------------------------------------------------------------------------
 */

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
bstr2oem(BSTR bstr)
{
    int size, len = SysStringLen(bstr);
    char *buf;

    size = WideCharToMultiByte(CP_OEMCP, 0, bstr, len, 0, 0, NULL, NULL);
    buf = new char [size+1];
    len = WideCharToMultiByte(CP_OEMCP, 0, bstr, len, buf, size, NULL, NULL);
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
	char *keyName;
	HKEY root;

	hr = pDebug->Evaluate(askpid, &gotpid);
	thePid = _wtoi(gotpid.m_str);
	keyName = bstr2oem(gotpid);

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, "Software\\Tomasoft\\MsDevDbgCtrl",
		0, REG_NONE, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, 0L, &root,
		0L);

	// Save the commandline in the registry using the PID as the key.
	RegSetValueEx(root, keyName, 0,	REG_SZ,
		(BYTE *) Tcl_DStringValue(cmdline),
		Tcl_DStringLength(cmdline));

	delete keyName;
	RegFlushKey(root);
	RegCloseKey(root);
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
