/*
 ------------------------------------------------------------------------------
 * TclAdapter.hpp --
 *
 *	Defines one templated class for doing the grunt work for making
 *	C++ extensions.
 *
 * Copyright (c) 1999-2001 TomaSoft Engineering
 *
 * See the file "license.txt" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * RCS: @(#) $Id: TclAdapter.hpp,v 1.1.2.1 2002/03/11 00:31:56 davygrvy Exp $
 ------------------------------------------------------------------------------
 */
#ifndef INC_TclAdapter_hpp__
#define INC_TclAdapter_hpp__

#include "tcl.h"

// We need at least the Tcl_Obj interface that was started in 8.0
#if TCL_MAJOR_VERSION < 8
#   error "we need Tcl 8.0 or greater to build this"

// Check for Stubs compatibility when asked for it.
#elif defined(USE_TCL_STUBS) && TCL_MAJOR_VERSION == 8 && \
	(TCL_MINOR_VERSION == 0 || \
        (TCL_MINOR_VERSION == 1 && TCL_RELEASE_LEVEL != TCL_FINAL_RELEASE))
#   error "Stubs interface doesn't work in 8.0 and alpha/beta 8.1; only 8.1.0+"
#endif

#ifdef _MSC_VER
    // Only do this when MSVC++ is compiling us.
#   ifdef USE_TCL_STUBS
	// Mark this .obj as needing tcl's Stubs library.
#	pragma comment(lib, "tclstub" \
	    STRINGIFY(JOIN(TCL_MAJOR_VERSION,TCL_MINOR_VERSION)) ".lib")
#	if !defined(_MT) || !defined(_DLL) || defined(_DEBUG)
	    // This fixes a bug with how the Stubs library was compiled.
	    // The requirement for msvcrt.lib from tclstubXX.lib should
	    // be removed.
#	    pragma comment(linker, "-nodefaultlib:msvcrt.lib")
#	endif
#   else
    // Mark this .obj needing the import library
#   pragma comment(lib, "tcl" \
	STRINGIFY(JOIN(TCL_MAJOR_VERSION,TCL_MINOR_VERSION)) ".lib")
#   endif
#endif

// We only need one Win32 API in here so we can be super defensive
// with the destructor.
#ifdef __WIN32__
#   include <windows.h>
#   ifdef _MSC_VER
#	pragma comment (lib, "kernel32.lib")
#   endif
#endif


// short cut
#define NewTclCmd(a,b,c) \
	Tcl_CreateObjCommand((a), (b), CmdDemux, CmdInfo((c), this), CmdDelete)


namespace Tcl {
    template <class T>
	class Adapter
    {
	typedef struct {
	    T *ext;
	    int (T::*cmd)(int, struct Tcl_Obj * CONST []);
	} MPLEXDATA, *LPMPLEXDATA;

    protected:
	Adapter(Tcl_Interp *_interp);
	~Adapter();

	Tcl_Interp *interp;

	virtual void DoCleanup(void) = 0;

	// Create the multiplexor data that we save in the ClientData portion
	// of the Tcl command.
	ClientData CmdInfo(int (T::*cmd)(int, struct Tcl_Obj * CONST []), T *that)
	{
	    LPMPLEXDATA mplex = new MPLEXDATA;
	    
	    mplex->ext = that;
	    mplex->cmd = cmd;
	    return static_cast <ClientData>(mplex);
	}
	static Tcl_InterpDeleteProc InterpDeleting;
	static Tcl_ExitProc Exiting;
	static Tcl_ObjCmdProc CmdDemux;
	static Tcl_CmdDeleteProc CmdDelete;
    };


    template <class T>
	Adapter<T>::Adapter(Tcl_Interp *_interp)
	: interp(_interp)
    {
	Tcl_CallWhenDeleted(interp, InterpDeleting, this);
	Tcl_CreateExitHandler(Exiting, this);
    }


    template <class T>
	Adapter<T>::~Adapter()
    {
	Tcl_DeleteExitHandler(Exiting, this);
    }


    template <class T> void
	Adapter<T>::InterpDeleting (ClientData clientData, Tcl_Interp *)
    {
	T *adapt = static_cast <T *>(clientData);
	adapt->DoCleanup();
	delete adapt;
    }


    template <class T> void
	Adapter<T>::Exiting (ClientData clientData)
    {
#ifdef __WIN32__
	// It can happen that the HEAP could have already been unloaded from an
	// awkward teardown caused by a Ctrl+C or other.  Win32 seems to do a
	// teardown in reverse order and by the time Tcl knows what's going on
	// and Tcl_Finalize calls the exit handlers, this extension's data has
	// already been unloaded by the OS.  Do a quick legal check on the
	// pointer first.
	if (IsBadReadPtr(clientData, sizeof(T *))) return;
#endif
	delete static_cast <T *>(clientData);
    }


    template <class T> int
	Adapter<T>::CmdDemux (ClientData clientData, Tcl_Interp *, int objc,
			      struct Tcl_Obj * CONST objv[])
    {
	LPMPLEXDATA demux = static_cast <LPMPLEXDATA>(clientData);
	// We aleady know what the interp pointer is (saved in the class
	// instance), so don't bother with it here. Call the member function
	// of the extension instance as saved in the MPLEXDATA struct when we
	// created this command using the somewhat obscure C++ pointer-to-
	// member method.
	//
	// This is a demultiplexor or 'demux' for short.
	return ((demux->ext) ->* (demux->cmd)) (objc,objv);
    }


    template <class T> void
	Adapter<T>::CmdDelete (ClientData clientData)
    {
	// clean-up the MPLEXDATA structure from the commands.
	delete static_cast <LPMPLEXDATA>(clientData);
    }
}
#endif
