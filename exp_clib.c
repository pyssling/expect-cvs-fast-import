/* exp_clib.c - top-level functions in the expect C library, libexpect.a

Written by: Don Libes, libes@cme.nist.gov, NIST, 12/3/90

Design and implementation of this program was paid for by U.S. tax
dollars.  Therefore it is public domain.  However, the author and NIST
would appreciate credit if this program or parts of it are used.
*/

#include "expect_cf.h"
#include <stdio.h>
#include <setjmp.h>
#ifdef HAVE_INTTYPES_H
#  include <inttypes.h>
#endif
#include <sys/types.h>
#include <sys/ioctl.h>

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#ifdef CRAY
# ifndef TCSETCTTY
#  if defined(HAVE_TERMIOS)
#   include <termios.h>
#  else
#   include <termio.h>
#  endif
# endif
#endif

#ifdef HAVE_SYS_FCNTL_H
#  include <sys/fcntl.h>
#else
#  include <fcntl.h>
#endif

#ifdef HAVE_STRREDIR_H
#include <sys/strredir.h>
# ifdef SRIOCSREDIR
#  undef TIOCCONS
# endif
#endif

#include <signal.h>
/*#include <memory.h> - deprecated - ANSI C moves them into string.h */
#include "string.h"

#include <errno.h>

#ifdef NO_STDLIB_H

/*
 * Tcl's compat/stdlib.h
 */

/*
 * stdlib.h --
 *
 *	Declares facilities exported by the "stdlib" portion of
 *	the C library.  This file isn't complete in the ANSI-C
 *	sense;  it only declares things that are needed by Tcl.
 *	This file is needed even on many systems with their own
 *	stdlib.h (e.g. SunOS) because not all stdlib.h files
 *	declare all the procedures needed here (such as strtod).
 *
 * Copyright (c) 1991 The Regents of the University of California.
 * Copyright (c) 1994 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * RCS: @(#) $Id: exp_clib.c,v 5.28.1.1.2.4 1999/06/16 03:02:33 don Exp $
 */

#ifndef _STDLIB
#define _STDLIB

#include <tcl.h>

extern void		abort _ANSI_ARGS_((void));
extern double		atof _ANSI_ARGS_((CONST char *string));
extern int		atoi _ANSI_ARGS_((CONST char *string));
extern long		atol _ANSI_ARGS_((CONST char *string));
extern char *		calloc _ANSI_ARGS_((unsigned int numElements,
			    unsigned int size));
extern void		exit _ANSI_ARGS_((int status));
extern int		free _ANSI_ARGS_((char *blockPtr));
extern char *		getenv _ANSI_ARGS_((CONST char *name));
extern char *		malloc _ANSI_ARGS_((unsigned int numBytes));
extern void		qsort _ANSI_ARGS_((VOID *base, int n, int size,
			    int (*compar)(CONST VOID *element1, CONST VOID
			    *element2)));
extern char *		realloc _ANSI_ARGS_((char *ptr, unsigned int numBytes));
extern double		strtod _ANSI_ARGS_((CONST char *string, char **endPtr));
extern long		strtol _ANSI_ARGS_((CONST char *string, char **endPtr,
			    int base));
extern unsigned long	strtoul _ANSI_ARGS_((CONST char *string,
			    char **endPtr, int base));

#endif /* _STDLIB */

/*
 * end of Tcl's compat/stdlib.h
 */

#else
#include <stdlib.h>		/* for malloc */
#endif

/*
 * tcl.h --
 *
 *	This header file describes the externally-visible facilities
 *	of the Tcl interpreter.
 *
 * Copyright (c) 1987-1994 The Regents of the University of California.
 * Copyright (c) 1994-1997 Sun Microsystems, Inc.
 * Copyright (c) 1993-1996 Lucent Technologies.
 * Copyright (c) 1998-1999 Scriptics Corporation.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * RCS: @(#) $Id: exp_clib.c,v 5.28.1.1.2.4 1999/06/16 03:02:33 don Exp $
 */

#ifndef _TCL
#define _TCL

/*
 * The following defines are used to indicate the various release levels.
 */

#define TCL_ALPHA_RELEASE	0
#define TCL_BETA_RELEASE	1
#define TCL_FINAL_RELEASE	2

/*
 * When version numbers change here, must also go into the following files
 * and update the version numbers:
 *
 * README
 * library/init.tcl	(only if major.minor changes, not patchlevel)
 * unix/configure.in
 * win/makefile.bc	(only if major.minor changes, not patchlevel)
 * win/makefile.vc	(only if major.minor changes, not patchlevel)
 * win/README
 * win/README.binary
 * mac/README
 *
 */

#define TCL_MAJOR_VERSION   8
#define TCL_MINOR_VERSION   0
#define TCL_RELEASE_LEVEL   TCL_FINAL_RELEASE
#define TCL_RELEASE_SERIAL  5

#define TCL_VERSION	    "8.0"
#define TCL_PATCH_LEVEL	    "8.0.5"

/*
 * The following definitions set up the proper options for Windows
 * compilers.  We use this method because there is no autoconf equivalent.
 */

#ifndef __WIN32__
#   if defined(_WIN32) || defined(WIN32)
#	define __WIN32__
#   endif
#endif

#ifdef __WIN32__
#   ifndef STRICT
#	define STRICT
#   endif
#   ifndef USE_PROTOTYPE
#	define USE_PROTOTYPE 1
#   endif
#   ifndef HAS_STDARG
#	define HAS_STDARG 1
#   endif
#   ifndef USE_PROTOTYPE
#	define USE_PROTOTYPE 1
#   endif

/*
 * Under Windows we need to call Tcl_Alloc in all cases to avoid competing
 * C run-time library issues.
 */

#   ifndef USE_TCLALLOC
#	define USE_TCLALLOC 1
#   endif
#endif /* __WIN32__ */

/*
 * The following definitions set up the proper options for Macintosh
 * compilers.  We use this method because there is no autoconf equivalent.
 */

#ifdef MAC_TCL
#   ifndef HAS_STDARG
#	define HAS_STDARG 1
#   endif
#   ifndef USE_TCLALLOC
#	define USE_TCLALLOC 1
#   endif
#   ifndef NO_STRERROR
#	define NO_STRERROR 1
#   endif
#endif

/*
 * Utility macros: STRINGIFY takes an argument and wraps it in "" (double
 * quotation marks), JOIN joins two arguments.
 */

#define VERBATIM(x) x
#ifdef _MSC_VER
# define STRINGIFY(x) STRINGIFY1(x)
# define STRINGIFY1(x) #x
# define JOIN(a,b) JOIN1(a,b)
# define JOIN1(a,b) a##b
#else
# ifdef RESOURCE_INCLUDED
#  define STRINGIFY(x) STRINGIFY1(x)
#  define STRINGIFY1(x) #x
#  define JOIN(a,b) JOIN1(a,b)
#  define JOIN1(a,b) a##b
# else
#  ifdef __STDC__
#   define STRINGIFY(x) #x
#   define JOIN(a,b) a##b
#  else
#   define STRINGIFY(x) "x"
#   define JOIN(a,b) VERBATIM(a)VERBATIM(b)
#  endif
# endif
#endif

/* 
 * A special definition used to allow this header file to be included 
 * in resource files so that they can get obtain version information from
 * this file.  Resource compilers don't like all the C stuff, like typedefs
 * and procedure declarations, that occur below.
 */

#ifndef RESOURCE_INCLUDED

#ifndef BUFSIZ
#include <stdio.h>
#endif

/*
 * Definitions that allow Tcl functions with variable numbers of
 * arguments to be used with either varargs.h or stdarg.h.  TCL_VARARGS
 * is used in procedure prototypes.  TCL_VARARGS_DEF is used to declare
 * the arguments in a function definiton: it takes the type and name of
 * the first argument and supplies the appropriate argument declaration
 * string for use in the function definition.  TCL_VARARGS_START
 * initializes the va_list data structure and returns the first argument.
 */

#if defined(__STDC__) || defined(HAS_STDARG)
#   include <stdarg.h>

#   define TCL_VARARGS(type, name) (type name, ...)
#   define TCL_VARARGS_DEF(type, name) (type name, ...)
#   define TCL_VARARGS_START(type, name, list) (va_start(list, name), name)
#else
#   include <varargs.h>

#   ifdef __cplusplus
#	define TCL_VARARGS(type, name) (type name, ...)
#	define TCL_VARARGS_DEF(type, name) (type va_alist, ...)
#   else
#	define TCL_VARARGS(type, name) ()
#	define TCL_VARARGS_DEF(type, name) (va_alist)
#   endif
#   define TCL_VARARGS_START(type, name, list) \
	(va_start(list), va_arg(list, type))
#endif

/*
 * Macros used to declare a function to be exported by a DLL.
 * Used by Windows, maps to no-op declarations on non-Windows systems.
 * The default build on windows is for a DLL, which causes the DLLIMPORT
 * and DLLEXPORT macros to be nonempty. To build a static library, the
 * macro STATIC_BUILD should be defined.
 */

#ifdef STATIC_BUILD
# define DLLIMPORT
# define DLLEXPORT
#else
# if defined(__WIN32__) && (defined(_MSC_VER) || (defined(__GNUC__) && defined(__declspec)))
#   define DLLIMPORT __declspec(dllimport)
#   define DLLEXPORT __declspec(dllexport)
# else
#  define DLLIMPORT
#  define DLLEXPORT
# endif
#endif

/*
 * These macros are used to control whether functions are being declared for
 * import or export.  If a function is being declared while it is being built
 * to be included in a shared library, then it should have the DLLEXPORT
 * storage class.  If is being declared for use by a module that is going to
 * link against the shared library, then it should have the DLLIMPORT storage
 * class.  If the symbol is beind declared for a static build or for use from a
 * stub library, then the storage class should be empty.
 *
 * The convention is that a macro called BUILD_xxxx, where xxxx is the
 * name of a library we are building, is set on the compile line for sources
 * that are to be placed in the library.  When this macro is set, the
 * storage class will be set to DLLEXPORT.  At the end of the header file, the
 * storage class will be reset to DLLIMPORt.
 */

#undef TCL_STORAGE_CLASS
#ifdef BUILD_tcl
# define TCL_STORAGE_CLASS DLLEXPORT
#else
# ifdef USE_TCL_STUBS
#  define TCL_STORAGE_CLASS
# else
#  define TCL_STORAGE_CLASS DLLIMPORT
# endif
#endif

/*
 * Definitions that allow this header file to be used either with or
 * without ANSI C features like function prototypes.  */

#undef _ANSI_ARGS_
#undef CONST

#if ((defined(__STDC__) || defined(SABER)) && !defined(NO_PROTOTYPE)) || defined(__cplusplus) || defined(USE_PROTOTYPE)
#   define _USING_PROTOTYPES_ 1
#   define _ANSI_ARGS_(x)	x
#   define CONST const
#else
#   define _ANSI_ARGS_(x)	()
#   define CONST
#endif

#ifdef __cplusplus
#   define EXTERN extern "C" TCL_STORAGE_CLASS
#else
#   define EXTERN extern TCL_STORAGE_CLASS
#endif

/*
 * Macro to use instead of "void" for arguments that must have
 * type "void *" in ANSI C;  maps them to type "char *" in
 * non-ANSI systems.
 */
#ifndef __WIN32__
#ifndef VOID
#   ifdef __STDC__
#       define VOID void
#   else
#       define VOID char
#   endif
#endif
#else /* __WIN32__ */
/*
 * The following code is copied from winnt.h
 */
#ifndef VOID
#define VOID void
typedef char CHAR;
typedef short SHORT;
typedef long LONG;
#endif
#endif /* __WIN32__ */

/*
 * Miscellaneous declarations.
 */

#ifndef NULL
#define NULL 0
#endif

#ifndef _CLIENTDATA
#   if defined(__STDC__) || defined(__cplusplus)
    typedef void *ClientData;
#   else
    typedef int *ClientData;
#   endif /* __STDC__ */
#define _CLIENTDATA
#endif

/*
 * Data structures defined opaquely in this module. The definitions below
 * just provide dummy types. A few fields are made visible in Tcl_Interp
 * structures, namely those used for returning a string result from
 * commands. Direct access to the result field is discouraged in Tcl 8.0.
 * The interpreter result is either an object or a string, and the two
 * values are kept consistent unless some C code sets interp->result
 * directly. Programmers should use either the procedure Tcl_GetObjResult()
 * or Tcl_GetStringResult() to read the interpreter's result. See the
 * SetResult man page for details.
 * 
 * Note: any change to the Tcl_Interp definition below must be mirrored
 * in the "real" definition in tclInt.h.
 *
 * Note: Tcl_ObjCmdProc procedures do not directly set result and freeProc.
 * Instead, they set a Tcl_Obj member in the "real" structure that can be
 * accessed with Tcl_GetObjResult() and Tcl_SetObjResult().
 */

typedef struct Tcl_Interp {
    char *result;		/* If the last command returned a string
				 * result, this points to it. */
    void (*freeProc) _ANSI_ARGS_((char *blockPtr));
				/* Zero means the string result is
				 * statically allocated. TCL_DYNAMIC means
				 * it was allocated with ckalloc and should
				 * be freed with ckfree. Other values give
				 * the address of procedure to invoke to
				 * free the result. Tcl_Eval must free it
				 * before executing next command. */
    int errorLine;              /* When TCL_ERROR is returned, this gives
                                 * the line number within the command where
                                 * the error occurred (1 if first line). */
} Tcl_Interp;

typedef struct Tcl_AsyncHandler_ *Tcl_AsyncHandler;
typedef struct Tcl_Channel_ *Tcl_Channel;
typedef struct Tcl_Command_ *Tcl_Command;
typedef struct Tcl_Event Tcl_Event;
typedef struct Tcl_Pid_ *Tcl_Pid;
typedef struct Tcl_RegExp_ *Tcl_RegExp;
typedef struct Tcl_TimerToken_ *Tcl_TimerToken;
typedef struct Tcl_Trace_ *Tcl_Trace;
typedef struct Tcl_Var_ *Tcl_Var;

/*
 * When a TCL command returns, the interpreter contains a result from the
 * command. Programmers are strongly encouraged to use one of the
 * procedures Tcl_GetObjResult() or Tcl_GetStringResult() to read the
 * interpreter's result. See the SetResult man page for details. Besides
 * this result, the command procedure returns an integer code, which is 
 * one of the following:
 *
 * TCL_OK		Command completed normally; the interpreter's
 *			result contains	the command's result.
 * TCL_ERROR		The command couldn't be completed successfully;
 *			the interpreter's result describes what went wrong.
 * TCL_RETURN		The command requests that the current procedure
 *			return; the interpreter's result contains the
 *			procedure's return value.
 * TCL_BREAK		The command requests that the innermost loop
 *			be exited; the interpreter's result is meaningless.
 * TCL_CONTINUE		Go on to the next iteration of the current loop;
 *			the interpreter's result is meaningless.
 */

#define TCL_OK		0
#define TCL_ERROR	1
#define TCL_RETURN	2
#define TCL_BREAK	3
#define TCL_CONTINUE	4

#define TCL_RESULT_SIZE 200

/*
 * Argument descriptors for math function callbacks in expressions:
 */

typedef enum {TCL_INT, TCL_DOUBLE, TCL_EITHER} Tcl_ValueType;
typedef struct Tcl_Value {
    Tcl_ValueType type;		/* Indicates intValue or doubleValue is
				 * valid, or both. */
    long intValue;		/* Integer value. */
    double doubleValue;		/* Double-precision floating value. */
} Tcl_Value;

/*
 * Forward declaration of Tcl_Obj to prevent an error when the forward
 * reference to Tcl_Obj is encountered in the procedure types declared 
 * below.
 */

struct Tcl_Obj;

/*
 * Procedure types defined by Tcl:
 */

typedef int (Tcl_AppInitProc) _ANSI_ARGS_((Tcl_Interp *interp));
typedef int (Tcl_AsyncProc) _ANSI_ARGS_((ClientData clientData,
	Tcl_Interp *interp, int code));
typedef void (Tcl_ChannelProc) _ANSI_ARGS_((ClientData clientData, int mask));
typedef void (Tcl_CloseProc) _ANSI_ARGS_((ClientData data));
typedef void (Tcl_CmdDeleteProc) _ANSI_ARGS_((ClientData clientData));
typedef int (Tcl_CmdProc) _ANSI_ARGS_((ClientData clientData,
	Tcl_Interp *interp, int argc, char *argv[]));
typedef void (Tcl_CmdTraceProc) _ANSI_ARGS_((ClientData clientData,
	Tcl_Interp *interp, int level, char *command, Tcl_CmdProc *proc,
	ClientData cmdClientData, int argc, char *argv[]));
typedef void (Tcl_DupInternalRepProc) _ANSI_ARGS_((struct Tcl_Obj *srcPtr, 
        struct Tcl_Obj *dupPtr));
typedef int (Tcl_EventProc) _ANSI_ARGS_((Tcl_Event *evPtr, int flags));
typedef void (Tcl_EventCheckProc) _ANSI_ARGS_((ClientData clientData,
	int flags));
typedef int (Tcl_EventDeleteProc) _ANSI_ARGS_((Tcl_Event *evPtr,
        ClientData clientData));
typedef void (Tcl_EventSetupProc) _ANSI_ARGS_((ClientData clientData,
	int flags));
typedef void (Tcl_ExitProc) _ANSI_ARGS_((ClientData clientData));
typedef void (Tcl_FileProc) _ANSI_ARGS_((ClientData clientData, int mask));
typedef void (Tcl_FileFreeProc) _ANSI_ARGS_((ClientData clientData));
typedef void (Tcl_FreeInternalRepProc) _ANSI_ARGS_((struct Tcl_Obj *objPtr));
typedef void (Tcl_FreeProc) _ANSI_ARGS_((char *blockPtr));
typedef void (Tcl_IdleProc) _ANSI_ARGS_((ClientData clientData));
typedef void (Tcl_InterpDeleteProc) _ANSI_ARGS_((ClientData clientData,
	Tcl_Interp *interp));
typedef int (Tcl_MathProc) _ANSI_ARGS_((ClientData clientData,
	Tcl_Interp *interp, Tcl_Value *args, Tcl_Value *resultPtr));
typedef void (Tcl_NamespaceDeleteProc) _ANSI_ARGS_((ClientData clientData));
typedef int (Tcl_ObjCmdProc) _ANSI_ARGS_((ClientData clientData,
	Tcl_Interp *interp, int objc, struct Tcl_Obj * CONST objv[]));
typedef int (Tcl_PackageInitProc) _ANSI_ARGS_((Tcl_Interp *interp));
typedef void (Tcl_PanicProc) _ANSI_ARGS_(TCL_VARARGS(char *, format));
typedef void (Tcl_TcpAcceptProc) _ANSI_ARGS_((ClientData callbackData,
        Tcl_Channel chan, char *address, int port));
typedef void (Tcl_TimerProc) _ANSI_ARGS_((ClientData clientData));
typedef int (Tcl_SetFromAnyProc) _ANSI_ARGS_((Tcl_Interp *interp,
	struct Tcl_Obj *objPtr));
typedef void (Tcl_UpdateStringProc) _ANSI_ARGS_((struct Tcl_Obj *objPtr));
typedef char *(Tcl_VarTraceProc) _ANSI_ARGS_((ClientData clientData,
	Tcl_Interp *interp, char *part1, char *part2, int flags));

/*
 * The following structure represents a type of object, which is a
 * particular internal representation for an object plus a set of
 * procedures that provide standard operations on objects of that type.
 */

typedef struct Tcl_ObjType {
    char *name;			/* Name of the type, e.g. "int". */
    Tcl_FreeInternalRepProc *freeIntRepProc;
				/* Called to free any storage for the type's
				 * internal rep. NULL if the internal rep
				 * does not need freeing. */
    Tcl_DupInternalRepProc *dupIntRepProc;
    				/* Called to create a new object as a copy
				 * of an existing object. */
    Tcl_UpdateStringProc *updateStringProc;
    				/* Called to update the string rep from the
				 * type's internal representation. */
    Tcl_SetFromAnyProc *setFromAnyProc;
    				/* Called to convert the object's internal
				 * rep to this type. Frees the internal rep
				 * of the old type. Returns TCL_ERROR on
				 * failure. */
} Tcl_ObjType;

/*
 * One of the following structures exists for each object in the Tcl
 * system. An object stores a value as either a string, some internal
 * representation, or both.
 */

typedef struct Tcl_Obj {
    int refCount;		/* When 0 the object will be freed. */
    char *bytes;		/* This points to the first byte of the
				 * object's string representation. The array
				 * must be followed by a null byte (i.e., at
				 * offset length) but may also contain
				 * embedded null characters. The array's
				 * storage is allocated by ckalloc. NULL
				 * means the string rep is invalid and must
				 * be regenerated from the internal rep.
				 * Clients should use Tcl_GetStringFromObj
				 * to get a pointer to the byte array as a
				 * readonly value. */
    int length;			/* The number of bytes at *bytes, not
				 * including the terminating null. */
    Tcl_ObjType *typePtr;	/* Denotes the object's type. Always
				 * corresponds to the type of the object's
				 * internal rep. NULL indicates the object
				 * has no internal rep (has no type). */
    union {			/* The internal representation: */
	long longValue;		/*   - an long integer value */
	double doubleValue;	/*   - a double-precision floating value */
	VOID *otherValuePtr;	/*   - another, type-specific value */
	struct {		/*   - internal rep as two pointers */
	    VOID *ptr1;
	    VOID *ptr2;
	} twoPtrValue;
    } internalRep;
} Tcl_Obj;

/*
 * Macros to increment and decrement a Tcl_Obj's reference count, and to
 * test whether an object is shared (i.e. has reference count > 1).
 * Note: clients should use Tcl_DecrRefCount() when they are finished using
 * an object, and should never call TclFreeObj() directly. TclFreeObj() is
 * only defined and made public in tcl.h to support Tcl_DecrRefCount's macro
 * definition. Note also that Tcl_DecrRefCount() refers to the parameter
 * "obj" twice. This means that you should avoid calling it with an
 * expression that is expensive to compute or has side effects.
 */

EXTERN void		Tcl_IncrRefCount _ANSI_ARGS_((Tcl_Obj *objPtr));
EXTERN void		Tcl_DecrRefCount _ANSI_ARGS_((Tcl_Obj *objPtr));
EXTERN int		Tcl_IsShared _ANSI_ARGS_((Tcl_Obj *objPtr));

#ifdef TCL_MEM_DEBUG
#   define Tcl_IncrRefCount(objPtr) \
	Tcl_DbIncrRefCount(objPtr, __FILE__, __LINE__)
#   define Tcl_DecrRefCount(objPtr) \
	Tcl_DbDecrRefCount(objPtr, __FILE__, __LINE__)
#   define Tcl_IsShared(objPtr) \
	Tcl_DbIsShared(objPtr, __FILE__, __LINE__)
#else
#   define Tcl_IncrRefCount(objPtr) \
	++(objPtr)->refCount
#   define Tcl_DecrRefCount(objPtr) \
	if (--(objPtr)->refCount <= 0) TclFreeObj(objPtr)
#   define Tcl_IsShared(objPtr) \
	((objPtr)->refCount > 1)
#endif

/*
 * Macros and definitions that help to debug the use of Tcl objects.
 * When TCL_MEM_DEBUG is defined, the Tcl_New* declarations are 
 * overridden to call debugging versions of the object creation procedures.
 */

#ifdef TCL_MEM_DEBUG
#  define Tcl_NewBooleanObj(val) \
     Tcl_DbNewBooleanObj(val, __FILE__, __LINE__)
#  define Tcl_NewByteArrayObj(bytes, len) \
     Tcl_DbNewByteArrayObj(bytes, len, __FILE__, __LINE__)
#  define Tcl_NewDoubleObj(val) \
     Tcl_DbNewDoubleObj(val, __FILE__, __LINE__)
#  define Tcl_NewIntObj(val) \
     Tcl_DbNewLongObj(val, __FILE__, __LINE__)
#  define Tcl_NewListObj(objc, objv) \
     Tcl_DbNewListObj(objc, objv, __FILE__, __LINE__)
#  define Tcl_NewLongObj(val) \
     Tcl_DbNewLongObj(val, __FILE__, __LINE__)
#  define Tcl_NewObj() \
     Tcl_DbNewObj(__FILE__, __LINE__)
#  define Tcl_NewStringObj(bytes, len) \
     Tcl_DbNewStringObj(bytes, len, __FILE__, __LINE__)
#endif /* TCL_MEM_DEBUG */

/*
 * The following definitions support Tcl's namespace facility.
 * Note: the first five fields must match exactly the fields in a
 * Namespace structure (see tcl.h). 
 */

typedef struct Tcl_Namespace {
    char *name;                 /* The namespace's name within its parent
				 * namespace. This contains no ::'s. The
				 * name of the global namespace is ""
				 * although "::" is an synonym. */
    char *fullName;             /* The namespace's fully qualified name.
				 * This starts with ::. */
    ClientData clientData;      /* Arbitrary value associated with this
				 * namespace. */
    Tcl_NamespaceDeleteProc* deleteProc;
                                /* Procedure invoked when deleting the
				 * namespace to, e.g., free clientData. */
    struct Tcl_Namespace* parentPtr;
                                /* Points to the namespace that contains
				 * this one. NULL if this is the global
				 * namespace. */
} Tcl_Namespace;

/*
 * The following structure represents a call frame, or activation record.
 * A call frame defines a naming context for a procedure call: its local
 * scope (for local variables) and its namespace scope (used for non-local
 * variables; often the global :: namespace). A call frame can also define
 * the naming context for a namespace eval or namespace inscope command:
 * the namespace in which the command's code should execute. The
 * Tcl_CallFrame structures exist only while procedures or namespace
 * eval/inscope's are being executed, and provide a Tcl call stack.
 * 
 * A call frame is initialized and pushed using Tcl_PushCallFrame and
 * popped using Tcl_PopCallFrame. Storage for a Tcl_CallFrame must be
 * provided by the Tcl_PushCallFrame caller, and callers typically allocate
 * them on the C call stack for efficiency. For this reason, Tcl_CallFrame
 * is defined as a structure and not as an opaque token. However, most
 * Tcl_CallFrame fields are hidden since applications should not access
 * them directly; others are declared as "dummyX".
 *
 * WARNING!! The structure definition must be kept consistent with the
 * CallFrame structure in tclInt.h. If you change one, change the other.
 */

typedef struct Tcl_CallFrame {
    Tcl_Namespace *nsPtr;
    int dummy1;
    int dummy2;
    char *dummy3;
    char *dummy4;
    char *dummy5;
    int dummy6;
    char *dummy7;
    char *dummy8;
    int dummy9;
    char* dummy10;
} Tcl_CallFrame;

/*
 * Information about commands that is returned by Tcl_GetCommandInfo and
 * passed to Tcl_SetCommandInfo. objProc is an objc/objv object-based
 * command procedure while proc is a traditional Tcl argc/argv
 * string-based procedure. Tcl_CreateObjCommand and Tcl_CreateCommand
 * ensure that both objProc and proc are non-NULL and can be called to
 * execute the command. However, it may be faster to call one instead of
 * the other. The member isNativeObjectProc is set to 1 if an
 * object-based procedure was registered by Tcl_CreateObjCommand, and to
 * 0 if a string-based procedure was registered by Tcl_CreateCommand.
 * The other procedure is typically set to a compatibility wrapper that
 * does string-to-object or object-to-string argument conversions then
 * calls the other procedure.
 */
     
typedef struct Tcl_CmdInfo {
    int isNativeObjectProc;	 /* 1 if objProc was registered by a call to
				  * Tcl_CreateObjCommand; 0 otherwise.
				  * Tcl_SetCmdInfo does not modify this
				  * field. */
    Tcl_ObjCmdProc *objProc;	 /* Command's object-based procedure. */
    ClientData objClientData;	 /* ClientData for object proc. */
    Tcl_CmdProc *proc;		 /* Command's string-based procedure. */
    ClientData clientData;	 /* ClientData for string proc. */
    Tcl_CmdDeleteProc *deleteProc;
                                 /* Procedure to call when command is
                                  * deleted. */
    ClientData deleteData;	 /* Value to pass to deleteProc (usually
				  * the same as clientData). */
    Tcl_Namespace *namespacePtr; /* Points to the namespace that contains
				  * this command. Note that Tcl_SetCmdInfo
				  * will not change a command's namespace;
				  * use Tcl_RenameCommand to do that. */

} Tcl_CmdInfo;

/*
 * The structure defined below is used to hold dynamic strings.  The only
 * field that clients should use is the string field, and they should
 * never modify it.
 */

#define TCL_DSTRING_STATIC_SIZE 200
typedef struct Tcl_DString {
    char *string;		/* Points to beginning of string:  either
				 * staticSpace below or a malloced array. */
    int length;			/* Number of non-NULL characters in the
				 * string. */
    int spaceAvl;		/* Total number of bytes available for the
				 * string and its terminating NULL char. */
    char staticSpace[TCL_DSTRING_STATIC_SIZE];
				/* Space to use in common case where string
				 * is small. */
} Tcl_DString;

#define Tcl_DStringLength(dsPtr) ((dsPtr)->length)
#define Tcl_DStringValue(dsPtr) ((dsPtr)->string)
#define Tcl_DStringTrunc Tcl_DStringSetLength

/*
 * Definitions for the maximum number of digits of precision that may
 * be specified in the "tcl_precision" variable, and the number of
 * characters of buffer space required by Tcl_PrintDouble.
 */
 
#define TCL_MAX_PREC 17
#define TCL_DOUBLE_SPACE (TCL_MAX_PREC+10)

/*
 * Flag that may be passed to Tcl_ConvertElement to force it not to
 * output braces (careful!  if you change this flag be sure to change
 * the definitions at the front of tclUtil.c).
 */

#define TCL_DONT_USE_BRACES	1

/*
 * Flag that may be passed to Tcl_GetIndexFromObj to force it to disallow
 * abbreviated strings.
 */

#define TCL_EXACT	1

/*
 * Flag values passed to Tcl_RecordAndEval.
 * WARNING: these bit choices must not conflict with the bit choices
 * for evalFlag bits in tclInt.h!!
 */

#define TCL_NO_EVAL		0x10000
#define TCL_EVAL_GLOBAL		0x20000

/*
 * Special freeProc values that may be passed to Tcl_SetResult (see
 * the man page for details):
 */

#define TCL_VOLATILE	((Tcl_FreeProc *) 1)
#define TCL_STATIC	((Tcl_FreeProc *) 0)
#define TCL_DYNAMIC	((Tcl_FreeProc *) 3)

/*
 * Flag values passed to variable-related procedures.
 */

#define TCL_GLOBAL_ONLY		 1
#define TCL_NAMESPACE_ONLY	 2
#define TCL_APPEND_VALUE	 4
#define TCL_LIST_ELEMENT	 8
#define TCL_TRACE_READS		 0x10
#define TCL_TRACE_WRITES	 0x20
#define TCL_TRACE_UNSETS	 0x40
#define TCL_TRACE_DESTROYED	 0x80
#define TCL_INTERP_DESTROYED	 0x100
#define TCL_LEAVE_ERR_MSG	 0x200
#define TCL_PARSE_PART1		 0x400

/*
 * Types for linked variables:
 */

#define TCL_LINK_INT		1
#define TCL_LINK_DOUBLE		2
#define TCL_LINK_BOOLEAN	3
#define TCL_LINK_STRING		4
#define TCL_LINK_READ_ONLY	0x80

/*
 * The following declarations either map ckalloc and ckfree to
 * malloc and free, or they map them to procedures with all sorts
 * of debugging hooks defined in tclCkalloc.c.
 */

#ifdef TCL_MEM_DEBUG

#  define Tcl_Alloc(x) Tcl_DbCkalloc(x, __FILE__, __LINE__)
#  define Tcl_Free(x)  Tcl_DbCkfree(x, __FILE__, __LINE__)
#  define Tcl_Realloc(x,y) Tcl_DbCkrealloc((x), (y),__FILE__, __LINE__)
#  define ckalloc(x) Tcl_DbCkalloc(x, __FILE__, __LINE__)
#  define ckfree(x)  Tcl_DbCkfree(x, __FILE__, __LINE__)
#  define ckrealloc(x,y) Tcl_DbCkrealloc((x), (y),__FILE__, __LINE__)

#else

/*
 * If USE_TCLALLOC is true, then we need to call Tcl_Alloc instead of
 * the native malloc/free.  The only time USE_TCLALLOC should not be
 * true is when compiling the Tcl/Tk libraries on Unix systems.  In this
 * case we can safely call the native malloc/free directly as a performance
 * optimization.
 */

#  if USE_TCLALLOC
#     define ckalloc(x) Tcl_Alloc(x)
#     define ckfree(x) Tcl_Free(x)
#     define ckrealloc(x,y) Tcl_Realloc(x,y)
#  else
#     define ckalloc(x) malloc(x)
#     define ckfree(x)  free(x)
#     define ckrealloc(x,y) realloc(x,y)
#  endif
#  define Tcl_DumpActiveMemory(x)
#  define Tcl_ValidateAllMemory(x,y)

#endif /* !TCL_MEM_DEBUG */

/*
 * Forward declaration of Tcl_HashTable.  Needed by some C++ compilers
 * to prevent errors when the forward reference to Tcl_HashTable is
 * encountered in the Tcl_HashEntry structure.
 */

#ifdef __cplusplus
struct Tcl_HashTable;
#endif

/*
 * Structure definition for an entry in a hash table.  No-one outside
 * Tcl should access any of these fields directly;  use the macros
 * defined below.
 */

typedef struct Tcl_HashEntry {
    struct Tcl_HashEntry *nextPtr;	/* Pointer to next entry in this
					 * hash bucket, or NULL for end of
					 * chain. */
    struct Tcl_HashTable *tablePtr;	/* Pointer to table containing entry. */
    struct Tcl_HashEntry **bucketPtr;	/* Pointer to bucket that points to
					 * first entry in this entry's chain:
					 * used for deleting the entry. */
    ClientData clientData;		/* Application stores something here
					 * with Tcl_SetHashValue. */
    union {				/* Key has one of these forms: */
	char *oneWordValue;		/* One-word value for key. */
	int words[1];			/* Multiple integer words for key.
					 * The actual size will be as large
					 * as necessary for this table's
					 * keys. */
	char string[4];			/* String for key.  The actual size
					 * will be as large as needed to hold
					 * the key. */
    } key;				/* MUST BE LAST FIELD IN RECORD!! */
} Tcl_HashEntry;

/*
 * Structure definition for a hash table.  Must be in tcl.h so clients
 * can allocate space for these structures, but clients should never
 * access any fields in this structure.
 */

#define TCL_SMALL_HASH_TABLE 4
typedef struct Tcl_HashTable {
    Tcl_HashEntry **buckets;		/* Pointer to bucket array.  Each
					 * element points to first entry in
					 * bucket's hash chain, or NULL. */
    Tcl_HashEntry *staticBuckets[TCL_SMALL_HASH_TABLE];
					/* Bucket array used for small tables
					 * (to avoid mallocs and frees). */
    int numBuckets;			/* Total number of buckets allocated
					 * at **bucketPtr. */
    int numEntries;			/* Total number of entries present
					 * in table. */
    int rebuildSize;			/* Enlarge table when numEntries gets
					 * to be this large. */
    int downShift;			/* Shift count used in hashing
					 * function.  Designed to use high-
					 * order bits of randomized keys. */
    int mask;				/* Mask value used in hashing
					 * function. */
    int keyType;			/* Type of keys used in this table. 
					 * It's either TCL_STRING_KEYS,
					 * TCL_ONE_WORD_KEYS, or an integer
					 * giving the number of ints that
                                         * is the size of the key.
					 */
    Tcl_HashEntry *(*findProc) _ANSI_ARGS_((struct Tcl_HashTable *tablePtr,
	    CONST char *key));
    Tcl_HashEntry *(*createProc) _ANSI_ARGS_((struct Tcl_HashTable *tablePtr,
	    CONST char *key, int *newPtr));
} Tcl_HashTable;

/*
 * Structure definition for information used to keep track of searches
 * through hash tables:
 */

typedef struct Tcl_HashSearch {
    Tcl_HashTable *tablePtr;		/* Table being searched. */
    int nextIndex;			/* Index of next bucket to be
					 * enumerated after present one. */
    Tcl_HashEntry *nextEntryPtr;	/* Next entry to be enumerated in the
					 * the current bucket. */
} Tcl_HashSearch;

/*
 * Acceptable key types for hash tables:
 */

#define TCL_STRING_KEYS		0
#define TCL_ONE_WORD_KEYS	1

/*
 * Macros for clients to use to access fields of hash entries:
 */

#define Tcl_GetHashValue(h) ((h)->clientData)
#define Tcl_SetHashValue(h, value) ((h)->clientData = (ClientData) (value))
#define Tcl_GetHashKey(tablePtr, h) \
    ((char *) (((tablePtr)->keyType == TCL_ONE_WORD_KEYS) ? (h)->key.oneWordValue \
						: (h)->key.string))

/*
 * Macros to use for clients to use to invoke find and create procedures
 * for hash tables:
 */

#define Tcl_FindHashEntry(tablePtr, key) \
	(*((tablePtr)->findProc))(tablePtr, key)
#define Tcl_CreateHashEntry(tablePtr, key, newPtr) \
	(*((tablePtr)->createProc))(tablePtr, key, newPtr)

/*
 * Flag values to pass to Tcl_DoOneEvent to disable searches
 * for some kinds of events:
 */

#define TCL_DONT_WAIT		(1<<1)
#define TCL_WINDOW_EVENTS	(1<<2)
#define TCL_FILE_EVENTS		(1<<3)
#define TCL_TIMER_EVENTS	(1<<4)
#define TCL_IDLE_EVENTS		(1<<5)	/* WAS 0x10 ???? */
#define TCL_ALL_EVENTS		(~TCL_DONT_WAIT)

/*
 * The following structure defines a generic event for the Tcl event
 * system.  These are the things that are queued in calls to Tcl_QueueEvent
 * and serviced later by Tcl_DoOneEvent.  There can be many different
 * kinds of events with different fields, corresponding to window events,
 * timer events, etc.  The structure for a particular event consists of
 * a Tcl_Event header followed by additional information specific to that
 * event.
 */

struct Tcl_Event {
    Tcl_EventProc *proc;	/* Procedure to call to service this event. */
    struct Tcl_Event *nextPtr;	/* Next in list of pending events, or NULL. */
};

/*
 * Positions to pass to Tcl_QueueEvent:
 */

typedef enum {
    TCL_QUEUE_TAIL, TCL_QUEUE_HEAD, TCL_QUEUE_MARK
} Tcl_QueuePosition;

/*
 * Values to pass to Tcl_SetServiceMode to specify the behavior of notifier
 * event routines.
 */

#define TCL_SERVICE_NONE 0
#define TCL_SERVICE_ALL 1

/*
 * The following structure keeps is used to hold a time value, either as
 * an absolute time (the number of seconds from the epoch) or as an
 * elapsed time. On Unix systems the epoch is Midnight Jan 1, 1970 GMT.
 * On Macintosh systems the epoch is Midnight Jan 1, 1904 GMT.
 */

typedef struct Tcl_Time {
    long sec;			/* Seconds. */
    long usec;			/* Microseconds. */
} Tcl_Time;

/*
 * Bits to pass to Tcl_CreateFileHandler and Tcl_CreateChannelHandler
 * to indicate what sorts of events are of interest:
 */

#define TCL_READABLE	(1<<1)
#define TCL_WRITABLE	(1<<2)
#define TCL_EXCEPTION	(1<<3)

/*
 * Flag values to pass to Tcl_OpenCommandChannel to indicate the
 * disposition of the stdio handles.  TCL_STDIN, TCL_STDOUT, TCL_STDERR,
 * are also used in Tcl_GetStdChannel.
 */

#define TCL_STDIN		(1<<1)	
#define TCL_STDOUT		(1<<2)
#define TCL_STDERR		(1<<3)
#define TCL_ENFORCE_MODE	(1<<4)

/*
 * Typedefs for the various operations in a channel type:
 */

typedef int	(Tcl_DriverBlockModeProc) _ANSI_ARGS_((
		    ClientData instanceData, int mode));
typedef int	(Tcl_DriverCloseProc) _ANSI_ARGS_((ClientData instanceData,
		    Tcl_Interp *interp));
typedef int	(Tcl_DriverInputProc) _ANSI_ARGS_((ClientData instanceData,
		    char *buf, int toRead, int *errorCodePtr));
typedef int	(Tcl_DriverOutputProc) _ANSI_ARGS_((ClientData instanceData,
		    char *buf, int toWrite, int *errorCodePtr));
typedef int	(Tcl_DriverSeekProc) _ANSI_ARGS_((ClientData instanceData,
		    long offset, int mode, int *errorCodePtr));
typedef int	(Tcl_DriverSetOptionProc) _ANSI_ARGS_((
		    ClientData instanceData, Tcl_Interp *interp,
	            char *optionName, char *value));
typedef int	(Tcl_DriverGetOptionProc) _ANSI_ARGS_((
		    ClientData instanceData, Tcl_Interp *interp,
		    char *optionName, Tcl_DString *dsPtr));
typedef void	(Tcl_DriverWatchProc) _ANSI_ARGS_((
		    ClientData instanceData, int mask));
typedef int	(Tcl_DriverGetHandleProc) _ANSI_ARGS_((
		    ClientData instanceData, int direction,
		    ClientData *handlePtr));

/*
 * Enum for different end of line translation and recognition modes.
 */

typedef enum Tcl_EolTranslation {
    TCL_TRANSLATE_AUTO,			/* Eol == \r, \n and \r\n. */
    TCL_TRANSLATE_CR,			/* Eol == \r. */
    TCL_TRANSLATE_LF,			/* Eol == \n. */
    TCL_TRANSLATE_CRLF			/* Eol == \r\n. */
} Tcl_EolTranslation;

/*
 * struct Tcl_ChannelType:
 *
 * One such structure exists for each type (kind) of channel.
 * It collects together in one place all the functions that are
 * part of the specific channel type.
 */

typedef struct Tcl_ChannelType {
    char *typeName;			/* The name of the channel type in Tcl
                                         * commands. This storage is owned by
                                         * channel type. */
    Tcl_DriverBlockModeProc *blockModeProc;
    					/* Set blocking mode for the
                                         * raw channel. May be NULL. */
    Tcl_DriverCloseProc *closeProc;	/* Procedure to call to close
                                         * the channel. */
    Tcl_DriverInputProc *inputProc;	/* Procedure to call for input
                                         * on channel. */
    Tcl_DriverOutputProc *outputProc;	/* Procedure to call for output
                                         * on channel. */
    Tcl_DriverSeekProc *seekProc;	/* Procedure to call to seek
                                         * on the channel. May be NULL. */
    Tcl_DriverSetOptionProc *setOptionProc;
    					/* Set an option on a channel. */
    Tcl_DriverGetOptionProc *getOptionProc;
    					/* Get an option from a channel. */
    Tcl_DriverWatchProc *watchProc;	/* Set up the notifier to watch
                                         * for events on this channel. */
    Tcl_DriverGetHandleProc *getHandleProc;
					/* Get an OS handle from the channel
                                         * or NULL if not supported. */
    VOID *reserved;			/* reserved for future expansion */
} Tcl_ChannelType;

/*
 * The following flags determine whether the blockModeProc above should
 * set the channel into blocking or nonblocking mode. They are passed
 * as arguments to the blockModeProc procedure in the above structure.
 */

#define TCL_MODE_BLOCKING 0		/* Put channel into blocking mode. */
#define TCL_MODE_NONBLOCKING 1		/* Put channel into nonblocking
					 * mode. */

/*
 * Enum for different types of file paths.
 */

typedef enum Tcl_PathType {
    TCL_PATH_ABSOLUTE,
    TCL_PATH_RELATIVE,
    TCL_PATH_VOLUME_RELATIVE
} Tcl_PathType;

/*
 * These function have been renamed. The old names are deprecated, but we
 * define these macros for backwards compatibilty.
 */

#define Tcl_Ckalloc Tcl_Alloc
#define Tcl_Ckfree Tcl_Free
#define Tcl_Ckrealloc Tcl_Realloc
#define Tcl_Return Tcl_SetResult
#define Tcl_TildeSubst Tcl_TranslateFileName

/*
 * In later releases, Tcl_Panic will be the correct name to use.  For now
 * we leave it as panic to avoid breaking existing binaries.
 */

#define Tcl_Panic panic
#define Tcl_PanicVA panicVA

/*
 * The following constant is used to test for older versions of Tcl
 * in the stubs tables.
 */

#define TCL_STUB_MAGIC 0xFCA3BACF

/*
 * The following function is required to be defined in all stubs aware
 * extensions.  The function is actually implemented in the stub
 * library, not the main Tcl library, although there is a trivial
 * implementation in the main library in case an extension is statically
 * linked into an application.
 */

EXTERN char *		Tcl_InitStubs _ANSI_ARGS_((Tcl_Interp *interp,
			    char *version, int exact));

#if 0
/*
 * Include the public function declarations that are accessible via
 * the stubs table.
 */

#include "tclDecls.h"
#endif /*0*/

/*
 * Public functions that are not accessible via the stubs table.
 */

EXTERN void Tcl_InitMemory _ANSI_ARGS_((Tcl_Interp *interp));
EXTERN void Tcl_Main _ANSI_ARGS_((int argc, char **argv,
	Tcl_AppInitProc *appInitProc));

/*
 * Convenience declaration of Tcl_AppInit for backwards compatibility.
 * This function is not *implemented* by the tcl library, so the storage
 * class is neither DLLEXPORT nor DLLIMPORT
 */

#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS

EXTERN int		Tcl_AppInit _ANSI_ARGS_((Tcl_Interp *interp));

#endif /* RESOURCE_INCLUDED */

#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLIMPORT

#endif /* _TCL */



/*
 * end of Tcl definitions
 */




/*
 * regexp definitions - from tcl8.0/tclRegexp.h
 */

/*
 * Definitions etc. for regexp(3) routines.
 *
 * Caveat:  this is V8 regexp(3) [actually, a reimplementation thereof],
 * not the System V one.
 *
 * RCS: @(#) $Id: exp_clib.c,v 5.28.1.1.2.4 1999/06/16 03:02:33 don Exp $
 */

#ifndef _REGEXP
#define _REGEXP 1

#ifdef BUILD_tcl
# undef TCL_STORAGE_CLASS
# define TCL_STORAGE_CLASS DLLEXPORT
#endif

/*
 * NSUBEXP must be at least 10, and no greater than 117 or the parser
 * will not work properly.
 */

#define NSUBEXP  20

typedef struct regexp {
	char *startp[NSUBEXP];
	char *endp[NSUBEXP];
	char regstart;		/* Internal use only. */
	char reganch;		/* Internal use only. */
	char *regmust;		/* Internal use only. */
	int regmlen;		/* Internal use only. */
	char program[1];	/* Unwarranted chumminess with compiler. */
} regexp;

EXTERN regexp *TclRegComp _ANSI_ARGS_((char *exp));
EXTERN int TclRegExec _ANSI_ARGS_((regexp *prog, char *string, char *start));
EXTERN void TclRegSub _ANSI_ARGS_((regexp *prog, char *source, char *dest));
EXTERN void TclRegError _ANSI_ARGS_((char *msg));
EXTERN char *TclGetRegError _ANSI_ARGS_((void));

# undef TCL_STORAGE_CLASS
# define TCL_STORAGE_CLASS DLLIMPORT

#endif /* REGEXP */


/*
 * end of regexp definitions
 */

/*
 * regexp code - from tcl8.0/regexp.c
 */

/*
 * TclRegComp and TclRegExec -- TclRegSub is elsewhere
 *
 *	Copyright (c) 1986 by University of Toronto.
 *	Written by Henry Spencer.  Not derived from licensed software.
 *
 *	Permission is granted to anyone to use this software for any
 *	purpose on any computer system, and to redistribute it freely,
 *	subject to the following restrictions:
 *
 *	1. The author is not responsible for the consequences of use of
 *		this software, no matter how awful, even if they arise
 *		from defects in it.
 *
 *	2. The origin of this software must not be misrepresented, either
 *		by explicit claim or by omission.
 *
 *	3. Altered versions must be plainly marked as such, and must not
 *		be misrepresented as being the original software.
 *
 * Beware that some of this code is subtly aware of the way operator
 * precedence is structured in regular expressions.  Serious changes in
 * regular-expression syntax might require a total rethink.
 *
 * *** NOTE: this code has been altered slightly for use in Tcl: ***
 * *** 1. Use ckalloc and ckfree instead of  malloc and free.	 ***
 * *** 2. Add extra argument to regexp to specify the real	 ***
 * ***    start of the string separately from the start of the	 ***
 * ***    current search. This is needed to search for multiple	 ***
 * ***    matches within a string.				 ***
 * *** 3. Names have been changed, e.g. from regcomp to		 ***
 * ***    TclRegComp, to avoid clashes with other 		 ***
 * ***    regexp implementations used by applications. 		 ***
 * *** 4. Added errMsg declaration and TclRegError procedure	 ***
 * *** 5. Various lint-like things, such as casting arguments	 ***
 * ***	  in procedure calls.					 ***
 *
 * *** NOTE: This code has been altered for use in MT-Sturdy Tcl ***
 * *** 1. All use of static variables has been changed to access ***
 * ***    fields of a structure.                                 ***
 * *** 2. This in addition to changes to TclRegError makes the   ***
 * ***    code multi-thread safe.                                ***
 *
 * RCS: @(#) $Id: exp_clib.c,v 5.28.1.1.2.4 1999/06/16 03:02:33 don Exp $
 */

#if ACK
#include "tclInt.h"
#include "tclPort.h"
#endif /*ACK*/

/*
 * The variable below is set to NULL before invoking regexp functions
 * and checked after those functions.  If an error occurred then TclRegError
 * will set the variable to point to a (static) error message.  This
 * mechanism unfortunately does not support multi-threading, but the
 * procedures TclRegError and TclGetRegError can be modified to use
 * thread-specific storage for the variable and thereby make the code
 * thread-safe.
 */

static char *errMsg = NULL;

/*
 * The "internal use only" fields in regexp.h are present to pass info from
 * compile to execute that permits the execute phase to run lots faster on
 * simple cases.  They are:
 *
 * regstart	char that must begin a match; '\0' if none obvious
 * reganch	is the match anchored (at beginning-of-line only)?
 * regmust	string (pointer into program) that match must include, or NULL
 * regmlen	length of regmust string
 *
 * Regstart and reganch permit very fast decisions on suitable starting points
 * for a match, cutting down the work a lot.  Regmust permits fast rejection
 * of lines that cannot possibly match.  The regmust tests are costly enough
 * that TclRegComp() supplies a regmust only if the r.e. contains something
 * potentially expensive (at present, the only such thing detected is * or +
 * at the start of the r.e., which can involve a lot of backup).  Regmlen is
 * supplied because the test in TclRegExec() needs it and TclRegComp() is
 * computing it anyway.
 */

/*
 * Structure for regexp "program".  This is essentially a linear encoding
 * of a nondeterministic finite-state machine (aka syntax charts or
 * "railroad normal form" in parsing technology).  Each node is an opcode
 * plus a "next" pointer, possibly plus an operand.  "Next" pointers of
 * all nodes except BRANCH implement concatenation; a "next" pointer with
 * a BRANCH on both ends of it is connecting two alternatives.  (Here we
 * have one of the subtle syntax dependencies:  an individual BRANCH (as
 * opposed to a collection of them) is never concatenated with anything
 * because of operator precedence.)  The operand of some types of node is
 * a literal string; for others, it is a node leading into a sub-FSM.  In
 * particular, the operand of a BRANCH node is the first node of the branch.
 * (NB this is *not* a tree structure:  the tail of the branch connects
 * to the thing following the set of BRANCHes.)  The opcodes are:
 */

/* definition	number	opnd?	meaning */
#define	END	0	/* no	End of program. */
#define	BOL	1	/* no	Match "" at beginning of line. */
#define	EOL	2	/* no	Match "" at end of line. */
#define	ANY	3	/* no	Match any one character. */
#define	ANYOF	4	/* str	Match any character in this string. */
#define	ANYBUT	5	/* str	Match any character not in this string. */
#define	BRANCH	6	/* node	Match this alternative, or the next... */
#define	BACK	7	/* no	Match "", "next" ptr points backward. */
#define	EXACTLY	8	/* str	Match this string. */
#define	NOTHING	9	/* no	Match empty string. */
#define	STAR	10	/* node	Match this (simple) thing 0 or more times. */
#define	PLUS	11	/* node	Match this (simple) thing 1 or more times. */
#define	OPEN	20	/* no	Mark this point in input as start of #n. */
			/*	OPEN+1 is number 1, etc. */
#define	CLOSE	(OPEN+NSUBEXP)	/* no	Analogous to OPEN. */

/*
 * Opcode notes:
 *
 * BRANCH	The set of branches constituting a single choice are hooked
 *		together with their "next" pointers, since precedence prevents
 *		anything being concatenated to any individual branch.  The
 *		"next" pointer of the last BRANCH in a choice points to the
 *		thing following the whole choice.  This is also where the
 *		final "next" pointer of each individual branch points; each
 *		branch starts with the operand node of a BRANCH node.
 *
 * BACK		Normal "next" pointers all implicitly point forward; BACK
 *		exists to make loop structures possible.
 *
 * STAR,PLUS	'?', and complex '*' and '+', are implemented as circular
 *		BRANCH structures using BACK.  Simple cases (one character
 *		per match) are implemented with STAR and PLUS for speed
 *		and to minimize recursive plunges.
 *
 * OPEN,CLOSE	...are numbered at compile time.
 */

/*
 * A node is one char of opcode followed by two chars of "next" pointer.
 * "Next" pointers are stored as two 8-bit pieces, high order first.  The
 * value is a positive offset from the opcode of the node containing it.
 * An operand, if any, simply follows the node.  (Note that much of the
 * code generation knows about this implicit relationship.)
 *
 * Using two bytes for the "next" pointer is vast overkill for most things,
 * but allows patterns to get big without disasters.
 */
#define	OP(p)	(*(p))
#define	NEXT(p)	(((*((p)+1)&0377)<<8) + (*((p)+2)&0377))
#define	OPERAND(p)	((p) + 3)

/*
 * See regmagic.h for one further detail of program structure.
 */


/*
 * Utility definitions.
 */
#ifndef CHARBITS
#define	UCHARAT(p)	((int)*(unsigned char *)(p))
#else
#define	UCHARAT(p)	((int)*(p)&CHARBITS)
#endif

#define	FAIL(m)	{ TclRegError(m); return(NULL); }
#define	ISMULT(c)	((c) == '*' || (c) == '+' || (c) == '?')
#define	META	"^$.[()|?+*\\"

/*
 * Flags to be passed up and down.
 */
#define	HASWIDTH	01	/* Known never to match null string. */
#define	SIMPLE		02	/* Simple enough to be STAR/PLUS operand. */
#define	SPSTART		04	/* Starts with * or +. */
#define	WORST		0	/* Worst case. */

/*
 * Global work variables for TclRegComp().
 */
struct regcomp_state  {
    char *regparse;		/* Input-scan pointer. */
    int regnpar;		/* () count. */
    char *regcode;		/* Code-emit pointer; &regdummy = don't. */
    long regsize;		/* Code size. */
};

static char regdummy;

/*
 * The first byte of the regexp internal "program" is actually this magic
 * number; the start node begins in the second byte.
 */
#define	MAGIC	0234


/*
 * Forward declarations for TclRegComp()'s friends.
 */

static char *		reg _ANSI_ARGS_((int paren, int *flagp,
			    struct regcomp_state *rcstate));
static char *		regatom _ANSI_ARGS_((int *flagp,
			    struct regcomp_state *rcstate));
static char *		regbranch _ANSI_ARGS_((int *flagp,
			    struct regcomp_state *rcstate));
static void		regc _ANSI_ARGS_((int b,
			    struct regcomp_state *rcstate));
static void		reginsert _ANSI_ARGS_((int op, char *opnd,
			    struct regcomp_state *rcstate));
static char *		regnext _ANSI_ARGS_((char *p));
static char *		regnode _ANSI_ARGS_((int op,
			    struct regcomp_state *rcstate));
static void 		regoptail _ANSI_ARGS_((char *p, char *val));
static char *		regpiece _ANSI_ARGS_((int *flagp,
			    struct regcomp_state *rcstate));
static void 		regtail _ANSI_ARGS_((char *p, char *val));

#ifdef STRCSPN
static int strcspn _ANSI_ARGS_((char *s1, char *s2));
#endif

/*
 - TclRegComp - compile a regular expression into internal code
 *
 * We can't allocate space until we know how big the compiled form will be,
 * but we can't compile it (and thus know how big it is) until we've got a
 * place to put the code.  So we cheat:  we compile it twice, once with code
 * generation turned off and size counting turned on, and once "for real".
 * This also means that we don't allocate space until we are sure that the
 * thing really will compile successfully, and we never have to move the
 * code and thus invalidate pointers into it.  (Note that it has to be in
 * one piece because free() must be able to free it all.)
 *
 * Beware that the optimization-preparation code in here knows about some
 * of the structure of the compiled regexp.
 */
regexp *
TclRegComp(exp)
char *exp;
{
	register regexp *r;
	register char *scan;
	register char *longest;
	register int len;
	int flags;
	struct regcomp_state state;
	struct regcomp_state *rcstate= &state;

	if (exp == NULL)
		FAIL("NULL argument");

	/* First pass: determine size, legality. */
	rcstate->regparse = exp;
	rcstate->regnpar = 1;
	rcstate->regsize = 0L;
	rcstate->regcode = &regdummy;
	regc(MAGIC, rcstate);
	if (reg(0, &flags, rcstate) == NULL)
		return(NULL);

	/* Small enough for pointer-storage convention? */
	if (rcstate->regsize >= 32767L)		/* Probably could be 65535L. */
		FAIL("regexp too big");

	/* Allocate space. */
	r = (regexp *)ckalloc(sizeof(regexp) + (unsigned)rcstate->regsize);
	if (r == NULL)
		FAIL("out of space");

	/* Second pass: emit code. */
	rcstate->regparse = exp;
	rcstate->regnpar = 1;
	rcstate->regcode = r->program;
	regc(MAGIC, rcstate);
	if (reg(0, &flags, rcstate) == NULL)
		return(NULL);

	/* Dig out information for optimizations. */
	r->regstart = '\0';	/* Worst-case defaults. */
	r->reganch = 0;
	r->regmust = NULL;
	r->regmlen = 0;
	scan = r->program+1;			/* First BRANCH. */
	if (OP(regnext(scan)) == END) {		/* Only one top-level choice. */
		scan = OPERAND(scan);

		/* Starting-point info. */
		if (OP(scan) == EXACTLY)
			r->regstart = *OPERAND(scan);
		else if (OP(scan) == BOL)
			r->reganch++;

		/*
		 * If there's something expensive in the r.e., find the
		 * longest literal string that must appear and make it the
		 * regmust.  Resolve ties in favor of later strings, since
		 * the regstart check works with the beginning of the r.e.
		 * and avoiding duplication strengthens checking.  Not a
		 * strong reason, but sufficient in the absence of others.
		 */
		if (flags&SPSTART) {
			longest = NULL;
			len = 0;
			for (; scan != NULL; scan = regnext(scan))
				if (OP(scan) == EXACTLY && ((int) strlen(OPERAND(scan))) >= len) {
					longest = OPERAND(scan);
					len = strlen(OPERAND(scan));
				}
			r->regmust = longest;
			r->regmlen = len;
		}
	}

	return(r);
}

/*
 - reg - regular expression, i.e. main body or parenthesized thing
 *
 * Caller must absorb opening parenthesis.
 *
 * Combining parenthesis handling with the base level of regular expression
 * is a trifle forced, but the need to tie the tails of the branches to what
 * follows makes it hard to avoid.
 */
static char *
reg(paren, flagp, rcstate)
int paren;			/* Parenthesized? */
int *flagp;
struct regcomp_state *rcstate;
{
	register char *ret;
	register char *br;
	register char *ender;
	register int parno = 0;
	int flags;

	*flagp = HASWIDTH;	/* Tentatively. */

	/* Make an OPEN node, if parenthesized. */
	if (paren) {
		if (rcstate->regnpar >= NSUBEXP)
			FAIL("too many ()");
		parno = rcstate->regnpar;
		rcstate->regnpar++;
		ret = regnode(OPEN+parno,rcstate);
	} else
		ret = NULL;

	/* Pick up the branches, linking them together. */
	br = regbranch(&flags,rcstate);
	if (br == NULL)
		return(NULL);
	if (ret != NULL)
		regtail(ret, br);	/* OPEN -> first. */
	else
		ret = br;
	if (!(flags&HASWIDTH))
		*flagp &= ~HASWIDTH;
	*flagp |= flags&SPSTART;
	while (*rcstate->regparse == '|') {
		rcstate->regparse++;
		br = regbranch(&flags,rcstate);
		if (br == NULL)
			return(NULL);
		regtail(ret, br);	/* BRANCH -> BRANCH. */
		if (!(flags&HASWIDTH))
			*flagp &= ~HASWIDTH;
		*flagp |= flags&SPSTART;
	}

	/* Make a closing node, and hook it on the end. */
	ender = regnode((paren) ? CLOSE+parno : END,rcstate);	
	regtail(ret, ender);

	/* Hook the tails of the branches to the closing node. */
	for (br = ret; br != NULL; br = regnext(br))
		regoptail(br, ender);

	/* Check for proper termination. */
	if (paren && *rcstate->regparse++ != ')') {
		FAIL("unmatched ()");
	} else if (!paren && *rcstate->regparse != '\0') {
		if (*rcstate->regparse == ')') {
			FAIL("unmatched ()");
		} else
			FAIL("junk on end");	/* "Can't happen". */
		/* NOTREACHED */
	}

	return(ret);
}

/*
 - regbranch - one alternative of an | operator
 *
 * Implements the concatenation operator.
 */
static char *
regbranch(flagp, rcstate)
int *flagp;
struct regcomp_state *rcstate;
{
	register char *ret;
	register char *chain;
	register char *latest;
	int flags;

	*flagp = WORST;		/* Tentatively. */

	ret = regnode(BRANCH,rcstate);
	chain = NULL;
	while (*rcstate->regparse != '\0' && *rcstate->regparse != '|' &&
				*rcstate->regparse != ')') {
		latest = regpiece(&flags, rcstate);
		if (latest == NULL)
			return(NULL);
		*flagp |= flags&HASWIDTH;
		if (chain == NULL)	/* First piece. */
			*flagp |= flags&SPSTART;
		else
			regtail(chain, latest);
		chain = latest;
	}
	if (chain == NULL)	/* Loop ran zero times. */
		(void) regnode(NOTHING,rcstate);

	return(ret);
}

/*
 - regpiece - something followed by possible [*+?]
 *
 * Note that the branching code sequences used for ? and the general cases
 * of * and + are somewhat optimized:  they use the same NOTHING node as
 * both the endmarker for their branch list and the body of the last branch.
 * It might seem that this node could be dispensed with entirely, but the
 * endmarker role is not redundant.
 */
static char *
regpiece(flagp, rcstate)
int *flagp;
struct regcomp_state *rcstate;
{
	register char *ret;
	register char op;
	register char *next;
	int flags;

	ret = regatom(&flags,rcstate);
	if (ret == NULL)
		return(NULL);

	op = *rcstate->regparse;
	if (!ISMULT(op)) {
		*flagp = flags;
		return(ret);
	}

	if (!(flags&HASWIDTH) && op != '?')
		FAIL("*+ operand could be empty");
	*flagp = (op != '+') ? (WORST|SPSTART) : (WORST|HASWIDTH);

	if (op == '*' && (flags&SIMPLE))
		reginsert(STAR, ret, rcstate);
	else if (op == '*') {
		/* Emit x* as (x&|), where & means "self". */
		reginsert(BRANCH, ret, rcstate);			/* Either x */
		regoptail(ret, regnode(BACK,rcstate));		/* and loop */
		regoptail(ret, ret);			/* back */
		regtail(ret, regnode(BRANCH,rcstate));		/* or */
		regtail(ret, regnode(NOTHING,rcstate));		/* null. */
	} else if (op == '+' && (flags&SIMPLE))
		reginsert(PLUS, ret, rcstate);
	else if (op == '+') {
		/* Emit x+ as x(&|), where & means "self". */
		next = regnode(BRANCH,rcstate);			/* Either */
		regtail(ret, next);
		regtail(regnode(BACK,rcstate), ret);		/* loop back */
		regtail(next, regnode(BRANCH,rcstate));		/* or */
		regtail(ret, regnode(NOTHING,rcstate));		/* null. */
	} else if (op == '?') {
		/* Emit x? as (x|) */
		reginsert(BRANCH, ret, rcstate);			/* Either x */
		regtail(ret, regnode(BRANCH,rcstate));		/* or */
		next = regnode(NOTHING,rcstate);		/* null. */
		regtail(ret, next);
		regoptail(ret, next);
	}
	rcstate->regparse++;
	if (ISMULT(*rcstate->regparse))
		FAIL("nested *?+");

	return(ret);
}

/*
 - regatom - the lowest level
 *
 * Optimization:  gobbles an entire sequence of ordinary characters so that
 * it can turn them into a single node, which is smaller to store and
 * faster to run.  Backslashed characters are exceptions, each becoming a
 * separate node; the code is simpler that way and it's not worth fixing.
 */
static char *
regatom(flagp, rcstate)
int *flagp;
struct regcomp_state *rcstate;
{
	register char *ret;
	int flags;

	*flagp = WORST;		/* Tentatively. */

	switch (*rcstate->regparse++) {
	case '^':
		ret = regnode(BOL,rcstate);
		break;
	case '$':
		ret = regnode(EOL,rcstate);
		break;
	case '.':
		ret = regnode(ANY,rcstate);
		*flagp |= HASWIDTH|SIMPLE;
		break;
	case '[': {
			register int clss;
			register int classend;

			if (*rcstate->regparse == '^') {	/* Complement of range. */
				ret = regnode(ANYBUT,rcstate);
				rcstate->regparse++;
			} else
				ret = regnode(ANYOF,rcstate);
			if (*rcstate->regparse == ']' || *rcstate->regparse == '-')
				regc(*rcstate->regparse++,rcstate);
			while (*rcstate->regparse != '\0' && *rcstate->regparse != ']') {
				if (*rcstate->regparse == '-') {
					rcstate->regparse++;
					if (*rcstate->regparse == ']' || *rcstate->regparse == '\0')
						regc('-',rcstate);
					else {
						clss = UCHARAT(rcstate->regparse-2)+1;
						classend = UCHARAT(rcstate->regparse);
						if (clss > classend+1)
							FAIL("invalid [] range");
						for (; clss <= classend; clss++)
							regc((char)clss,rcstate);
						rcstate->regparse++;
					}
				} else
					regc(*rcstate->regparse++,rcstate);
			}
			regc('\0',rcstate);
			if (*rcstate->regparse != ']')
				FAIL("unmatched []");
			rcstate->regparse++;
			*flagp |= HASWIDTH|SIMPLE;
		}
		break;
	case '(':
		ret = reg(1, &flags, rcstate);
		if (ret == NULL)
			return(NULL);
		*flagp |= flags&(HASWIDTH|SPSTART);
		break;
	case '\0':
	case '|':
	case ')':
		FAIL("internal urp");	/* Supposed to be caught earlier. */
		/* NOTREACHED */
	case '?':
	case '+':
	case '*':
		FAIL("?+* follows nothing");
		/* NOTREACHED */
	case '\\':
		if (*rcstate->regparse == '\0')
			FAIL("trailing \\");
		ret = regnode(EXACTLY,rcstate);
		regc(*rcstate->regparse++,rcstate);
		regc('\0',rcstate);
		*flagp |= HASWIDTH|SIMPLE;
		break;
	default: {
			register int len;
			register char ender;

			rcstate->regparse--;
			len = strcspn(rcstate->regparse, META);
			if (len <= 0)
				FAIL("internal disaster");
			ender = *(rcstate->regparse+len);
			if (len > 1 && ISMULT(ender))
				len--;		/* Back off clear of ?+* operand. */
			*flagp |= HASWIDTH;
			if (len == 1)
				*flagp |= SIMPLE;
			ret = regnode(EXACTLY,rcstate);
			while (len > 0) {
				regc(*rcstate->regparse++,rcstate);
				len--;
			}
			regc('\0',rcstate);
		}
		break;
	}

	return(ret);
}

/*
 - regnode - emit a node
 */
static char *			/* Location. */
regnode(op, rcstate)
int op;
struct regcomp_state *rcstate;
{
	register char *ret;
	register char *ptr;

	ret = rcstate->regcode;
	if (ret == &regdummy) {
		rcstate->regsize += 3;
		return(ret);
	}

	ptr = ret;
	*ptr++ = (char)op;
	*ptr++ = '\0';		/* Null "next" pointer. */
	*ptr++ = '\0';
	rcstate->regcode = ptr;

	return(ret);
}

/*
 - regc - emit (if appropriate) a byte of code
 */
static void
regc(b, rcstate)
int b;
struct regcomp_state *rcstate;
{
	if (rcstate->regcode != &regdummy)
		*rcstate->regcode++ = (char)b;
	else
		rcstate->regsize++;
}

/*
 - reginsert - insert an operator in front of already-emitted operand
 *
 * Means relocating the operand.
 */
static void
reginsert(op, opnd, rcstate)
int op;
char *opnd;
struct regcomp_state *rcstate;
{
	register char *src;
	register char *dst;
	register char *place;

	if (rcstate->regcode == &regdummy) {
		rcstate->regsize += 3;
		return;
	}

	src = rcstate->regcode;
	rcstate->regcode += 3;
	dst = rcstate->regcode;
	while (src > opnd)
		*--dst = *--src;

	place = opnd;		/* Op node, where operand used to be. */
	*place++ = (char)op;
	*place++ = '\0';
	*place = '\0';
}

/*
 - regtail - set the next-pointer at the end of a node chain
 */
static void
regtail(p, val)
char *p;
char *val;
{
	register char *scan;
	register char *temp;
	register int offset;

	if (p == &regdummy)
		return;

	/* Find last node. */
	scan = p;
	for (;;) {
		temp = regnext(scan);
		if (temp == NULL)
			break;
		scan = temp;
	}

	if (OP(scan) == BACK)
		offset = scan - val;
	else
		offset = val - scan;
	*(scan+1) = (char)((offset>>8)&0377);
	*(scan+2) = (char)(offset&0377);
}

/*
 - regoptail - regtail on operand of first argument; nop if operandless
 */
static void
regoptail(p, val)
char *p;
char *val;
{
	/* "Operandless" and "op != BRANCH" are synonymous in practice. */
	if (p == NULL || p == &regdummy || OP(p) != BRANCH)
		return;
	regtail(OPERAND(p), val);
}

/*
 * TclRegExec and friends
 */

/*
 * Global work variables for TclRegExec().
 */
struct regexec_state  {
    char *reginput;		/* String-input pointer. */
    char *regbol;		/* Beginning of input, for ^ check. */
    char **regstartp;	/* Pointer to startp array. */
    char **regendp;		/* Ditto for endp. */
};

/*
 * Forwards.
 */
static int 		regtry _ANSI_ARGS_((regexp *prog, char *string,
			    struct regexec_state *restate));
static int 		regmatch _ANSI_ARGS_((char *prog,
			    struct regexec_state *restate));
static int 		regrepeat _ANSI_ARGS_((char *p,
			    struct regexec_state *restate));

#ifdef DEBUG
int regnarrate = 0;
void regdump _ANSI_ARGS_((regexp *r));
static char *regprop _ANSI_ARGS_((char *op));
#endif

/*
 - TclRegExec - match a regexp against a string
 */
int
TclRegExec(prog, string, start)
register regexp *prog;
register char *string;
char *start;
{
	register char *s;
	struct regexec_state state;
	struct regexec_state *restate= &state;

	/* Be paranoid... */
	if (prog == NULL || string == NULL) {
		TclRegError("NULL parameter");
		return(0);
	}

	/* Check validity of program. */
	if (UCHARAT(prog->program) != MAGIC) {
		TclRegError("corrupted program");
		return(0);
	}

	/* If there is a "must appear" string, look for it. */
	if (prog->regmust != NULL) {
		s = string;
		while ((s = strchr(s, prog->regmust[0])) != NULL) {
			if (strncmp(s, prog->regmust, (size_t) prog->regmlen)
			    == 0)
				break;	/* Found it. */
			s++;
		}
		if (s == NULL)	/* Not present. */
			return(0);
	}

	/* Mark beginning of line for ^ . */
	restate->regbol = start;

	/* Simplest case:  anchored match need be tried only once. */
	if (prog->reganch)
		return(regtry(prog, string, restate));

	/* Messy cases:  unanchored match. */
	s = string;
	if (prog->regstart != '\0')
		/* We know what char it must start with. */
		while ((s = strchr(s, prog->regstart)) != NULL) {
			if (regtry(prog, s, restate))
				return(1);
			s++;
		}
	else
		/* We don't -- general case. */
		do {
			if (regtry(prog, s, restate))
				return(1);
		} while (*s++ != '\0');

	/* Failure. */
	return(0);
}

/*
 - regtry - try match at specific point
 */
static int			/* 0 failure, 1 success */
regtry(prog, string, restate)
regexp *prog;
char *string;
struct regexec_state *restate;
{
	register int i;
	register char **sp;
	register char **ep;

	restate->reginput = string;
	restate->regstartp = prog->startp;
	restate->regendp = prog->endp;

	sp = prog->startp;
	ep = prog->endp;
	for (i = NSUBEXP; i > 0; i--) {
		*sp++ = NULL;
		*ep++ = NULL;
	}
	if (regmatch(prog->program + 1,restate)) {
		prog->startp[0] = string;
		prog->endp[0] = restate->reginput;
		return(1);
	} else
		return(0);
}

/*
 - regmatch - main matching routine
 *
 * Conceptually the strategy is simple:  check to see whether the current
 * node matches, call self recursively to see whether the rest matches,
 * and then act accordingly.  In practice we make some effort to avoid
 * recursion, in particular by going through "ordinary" nodes (that don't
 * need to know whether the rest of the match failed) by a loop instead of
 * by recursion.
 */
static int			/* 0 failure, 1 success */
regmatch(prog, restate)
char *prog;
struct regexec_state *restate;
{
    register char *scan;	/* Current node. */
    char *next;		/* Next node. */

    scan = prog;
#ifdef DEBUG
    if (scan != NULL && regnarrate)
	fprintf(stderr, "%s(\n", regprop(scan));
#endif
    while (scan != NULL) {
#ifdef DEBUG
	if (regnarrate)
	    fprintf(stderr, "%s...\n", regprop(scan));
#endif
	next = regnext(scan);

	switch (OP(scan)) {
	    case BOL:
		if (restate->reginput != restate->regbol) {
		    return 0;
		}
		break;
	    case EOL:
		if (*restate->reginput != '\0') {
		    return 0;
		}
		break;
	    case ANY:
		if (*restate->reginput == '\0') {
		    return 0;
		}
		restate->reginput++;
		break;
	    case EXACTLY: {
		register int len;
		register char *opnd;

		opnd = OPERAND(scan);
		/* Inline the first character, for speed. */
		if (*opnd != *restate->reginput) {
		    return 0 ;
		}
		len = strlen(opnd);
		if (len > 1 && strncmp(opnd, restate->reginput, (size_t) len)
			!= 0) {
		    return 0;
		}
		restate->reginput += len;
		break;
	    }
	    case ANYOF:
		if (*restate->reginput == '\0'
			|| strchr(OPERAND(scan), *restate->reginput) == NULL) {
		    return 0;
		}
		restate->reginput++;
		break;
	    case ANYBUT:
		if (*restate->reginput == '\0'
			|| strchr(OPERAND(scan), *restate->reginput) != NULL) {
		    return 0;
		}
		restate->reginput++;
		break;
	    case NOTHING:
		break;
	    case BACK:
		break;
	    case OPEN+1:
	    case OPEN+2:
	    case OPEN+3:
	    case OPEN+4:
	    case OPEN+5:
	    case OPEN+6:
	    case OPEN+7:
	    case OPEN+8:
	    case OPEN+9: {
		register int no;
		register char *save;

	doOpen:
		no = OP(scan) - OPEN;
		save = restate->reginput;

		if (regmatch(next,restate)) {
		    /*
		     * Don't set startp if some later invocation of the
		     * same parentheses already has.
		     */
		    if (restate->regstartp[no] == NULL) {
			restate->regstartp[no] = save;
		    }
		    return 1;
		} else {
		    return 0;
		}
	    }
	    case CLOSE+1:
	    case CLOSE+2:
	    case CLOSE+3:
	    case CLOSE+4:
	    case CLOSE+5:
	    case CLOSE+6:
	    case CLOSE+7:
	    case CLOSE+8:
	    case CLOSE+9: {
		register int no;
		register char *save;

	doClose:
		no = OP(scan) - CLOSE;
		save = restate->reginput;

		if (regmatch(next,restate)) {
				/*
				 * Don't set endp if some later
				 * invocation of the same parentheses
				 * already has.
				 */
		    if (restate->regendp[no] == NULL)
			restate->regendp[no] = save;
		    return 1;
		} else {
		    return 0;
		}
	    }
	    case BRANCH: {
		register char *save;

		if (OP(next) != BRANCH) { /* No choice. */
		    next = OPERAND(scan); /* Avoid recursion. */
		} else {
		    do {
			save = restate->reginput;
			if (regmatch(OPERAND(scan),restate))
			    return(1);
			restate->reginput = save;
			scan = regnext(scan);
		    } while (scan != NULL && OP(scan) == BRANCH);
		    return 0;
		}
		break;
	    }
	    case STAR:
	    case PLUS: {
		register char nextch;
		register int no;
		register char *save;
		register int min;

		/*
		 * Lookahead to avoid useless match attempts
		 * when we know what character comes next.
		 */
		nextch = '\0';
		if (OP(next) == EXACTLY)
		    nextch = *OPERAND(next);
		min = (OP(scan) == STAR) ? 0 : 1;
		save = restate->reginput;
		no = regrepeat(OPERAND(scan),restate);
		while (no >= min) {
		    /* If it could work, try it. */
		    if (nextch == '\0' || *restate->reginput == nextch)
			if (regmatch(next,restate))
			    return(1);
		    /* Couldn't or didn't -- back up. */
		    no--;
		    restate->reginput = save + no;
		}
		return(0);
	    }
	    case END:
		return(1);	/* Success! */
	    default:
		if (OP(scan) > OPEN && OP(scan) < OPEN+NSUBEXP) {
		    goto doOpen;
		} else if (OP(scan) > CLOSE && OP(scan) < CLOSE+NSUBEXP) {
		    goto doClose;
		}
		TclRegError("memory corruption");
		return 0;
	}

	scan = next;
    }

    /*
     * We get here only if there's trouble -- normally "case END" is
     * the terminating point.
     */
    TclRegError("corrupted pointers");
    return(0);
}

/*
 - regrepeat - repeatedly match something simple, report how many
 */
static int
regrepeat(p, restate)
char *p;
struct regexec_state *restate;
{
	register int count = 0;
	register char *scan;
	register char *opnd;

	scan = restate->reginput;
	opnd = OPERAND(p);
	switch (OP(p)) {
	case ANY:
		count = strlen(scan);
		scan += count;
		break;
	case EXACTLY:
		while (*opnd == *scan) {
			count++;
			scan++;
		}
		break;
	case ANYOF:
		while (*scan != '\0' && strchr(opnd, *scan) != NULL) {
			count++;
			scan++;
		}
		break;
	case ANYBUT:
		while (*scan != '\0' && strchr(opnd, *scan) == NULL) {
			count++;
			scan++;
		}
		break;
	default:		/* Oh dear.  Called inappropriately. */
		TclRegError("internal foulup");
		count = 0;	/* Best compromise. */
		break;
	}
	restate->reginput = scan;

	return(count);
}

/*
 - regnext - dig the "next" pointer out of a node
 */
static char *
regnext(p)
register char *p;
{
	register int offset;

	if (p == &regdummy)
		return(NULL);

	offset = NEXT(p);
	if (offset == 0)
		return(NULL);

	if (OP(p) == BACK)
		return(p-offset);
	else
		return(p+offset);
}

#ifdef DEBUG

static char *regprop();

/*
 - regdump - dump a regexp onto stdout in vaguely comprehensible form
 */
void
regdump(r)
regexp *r;
{
	register char *s;
	register char op = EXACTLY;	/* Arbitrary non-END op. */
	register char *next;


	s = r->program + 1;
	while (op != END) {	/* While that wasn't END last time... */
		op = OP(s);
		printf("%2d%s", s-r->program, regprop(s));	/* Where, what. */
		next = regnext(s);
		if (next == NULL)		/* Next ptr. */
			printf("(0)");
		else 
			printf("(%d)", (s-r->program)+(next-s));
		s += 3;
		if (op == ANYOF || op == ANYBUT || op == EXACTLY) {
			/* Literal string, where present. */
			while (*s != '\0') {
				putchar(*s);
				s++;
			}
			s++;
		}
		putchar('\n');
	}

	/* Header fields of interest. */
	if (r->regstart != '\0')
		printf("start `%c' ", r->regstart);
	if (r->reganch)
		printf("anchored ");
	if (r->regmust != NULL)
		printf("must have \"%s\"", r->regmust);
	printf("\n");
}

/*
 - regprop - printable representation of opcode
 */
static char *
regprop(op)
char *op;
{
	register char *p;
	static char buf[50];

	(void) strcpy(buf, ":");

	switch (OP(op)) {
	case BOL:
		p = "BOL";
		break;
	case EOL:
		p = "EOL";
		break;
	case ANY:
		p = "ANY";
		break;
	case ANYOF:
		p = "ANYOF";
		break;
	case ANYBUT:
		p = "ANYBUT";
		break;
	case BRANCH:
		p = "BRANCH";
		break;
	case EXACTLY:
		p = "EXACTLY";
		break;
	case NOTHING:
		p = "NOTHING";
		break;
	case BACK:
		p = "BACK";
		break;
	case END:
		p = "END";
		break;
	case OPEN+1:
	case OPEN+2:
	case OPEN+3:
	case OPEN+4:
	case OPEN+5:
	case OPEN+6:
	case OPEN+7:
	case OPEN+8:
	case OPEN+9:
		sprintf(buf+strlen(buf), "OPEN%d", OP(op)-OPEN);
		p = NULL;
		break;
	case CLOSE+1:
	case CLOSE+2:
	case CLOSE+3:
	case CLOSE+4:
	case CLOSE+5:
	case CLOSE+6:
	case CLOSE+7:
	case CLOSE+8:
	case CLOSE+9:
		sprintf(buf+strlen(buf), "CLOSE%d", OP(op)-CLOSE);
		p = NULL;
		break;
	case STAR:
		p = "STAR";
		break;
	case PLUS:
		p = "PLUS";
		break;
	default:
		if (OP(op) > OPEN && OP(op) < OPEN+NSUBEXP) {
		    sprintf(buf+strlen(buf), "OPEN%d", OP(op)-OPEN);
		    p = NULL;
		    break;
		} else if (OP(op) > CLOSE && OP(op) < CLOSE+NSUBEXP) {
		    sprintf(buf+strlen(buf), "CLOSE%d", OP(op)-CLOSE);
		    p = NULL;
		} else {
		    TclRegError("corrupted opcode");
		}
		break;
	}
	if (p != NULL)
		(void) strcat(buf, p);
	return(buf);
}
#endif

/*
 * The following is provided for those people who do not have strcspn() in
 * their C libraries.  They should get off their butts and do something
 * about it; at least one public-domain implementation of those (highly
 * useful) string routines has been published on Usenet.
 */
#ifdef STRCSPN
/*
 * strcspn - find length of initial segment of s1 consisting entirely
 * of characters not from s2
 */

static int
strcspn(s1, s2)
char *s1;
char *s2;
{
	register char *scan1;
	register char *scan2;
	register int count;

	count = 0;
	for (scan1 = s1; *scan1 != '\0'; scan1++) {
		for (scan2 = s2; *scan2 != '\0';)	/* ++ moved down. */
			if (*scan1 == *scan2++)
				return(count);
		count++;
	}
	return(count);
}
#endif

/*
 *----------------------------------------------------------------------
 *
 * TclRegError --
 *
 *	This procedure is invoked by the regexp code when an error
 *	occurs.  It saves the error message so it can be seen by the
 *	code that called Spencer's code.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The value of "string" is saved in "errMsg".
 *
 *----------------------------------------------------------------------
 */

void
TclRegError(string)
    char *string;			/* Error message. */
{
    errMsg = string;
}

char *
TclGetRegError()
{
    return errMsg;
}






/*
 * end of regexp definitions and code
 */

/*
 * expect-specific definitions and code
 */

#include "expect.h"
#include "exp_int.h"


#define EXP_MATCH_MAX	2000
/* public */
char *exp_buffer = 0;
char *exp_buffer_end = 0;
char *exp_match = 0;
char *exp_match_end = 0;
int exp_match_max = EXP_MATCH_MAX;	/* bytes */
int exp_full_buffer = FALSE;		/* don't return on full buffer */
int exp_remove_nulls = TRUE;
int exp_timeout = 10;			/* seconds */
int exp_pty_timeout = 5;		/* seconds - see CRAY below */
int exp_autoallocpty = TRUE;		/* if TRUE, we do allocation */
int exp_pty[2];				/* master is [0], slave is [1] */
int exp_pid;
char *exp_stty_init = 0;		/* initial stty args */
int exp_ttycopy = TRUE;			/* copy tty parms from /dev/tty */
int exp_ttyinit = TRUE;			/* set tty parms to sane state */
int exp_console = FALSE;		/* redirect console */
void (*exp_child_exec_prelude)() = 0;

jmp_buf exp_readenv;		/* for interruptable read() */
int exp_reading = FALSE;	/* whether we can longjmp or not */

int exp_is_debugging = TRUE;
FILE *exp_debugfile = 0;

FILE *exp_logfile = 0;
int exp_logfile_all = FALSE;	/* if TRUE, write log of all interactions */
int exp_loguser = TRUE;		/* if TRUE, user sees interactions on stdout */


char *exp_printify();
int exp_getptymaster();
int exp_getptyslave();
int Exp_StringMatch();

#define sysreturn(x)	return(errno = x, -1)

void exp_init_pty();

/*
   The following functions are linked from the Tcl library.  They
   don't cause anything else in the library to be dragged in, so it
   shouldn't cause any problems (e.g., bloat).

   The functions are relatively small but painful enough that I don't care
   to recode them.  You may, if you absolutely want to get rid of any
   vestiges of Tcl.
*/
extern char *TclGetRegError();
extern void TclRegError();
char *Tcl_ErrnoMsg();



static unsigned int bufsiz = 2*EXP_MATCH_MAX;

static struct f {
	int valid;

	char *buffer;		/* buffer of matchable chars */
	char *buffer_end;	/* one beyond end of matchable chars */
	/*char *match;		/* start of matched string */
	char *match_end;	/* one beyond end of matched string */
	int msize;		/* size of allocate space */
				/* actual size is one larger for null */
} *fs = 0;

static int fd_alloc_max = -1;	/* max fd allocated */

/* translate fd or fp to fd */
static struct f *
fdfp2f(fd,fp)
int fd;
FILE *fp;
{
	if (fd == -1) return(fs + fileno(fp));
	else return(fs + fd);
}

static struct f *
fd_new(fd)
int fd;
{
	int i, low;
	struct f *fp;
	struct f *newfs;	/* temporary, so we don't lose old fs */

	if (fd > fd_alloc_max) {
		if (!fs) {	/* no fd's yet allocated */
			newfs = (struct f *)malloc(sizeof(struct f)*(fd+1));
			low = 0;
		} else {		/* enlarge fd table */
			newfs = (struct f *)realloc((char *)fs,sizeof(struct f)*(fd+1));
			low = fd_alloc_max+1;
		}
		fs = newfs;
		fd_alloc_max = fd;
		for (i = low; i <= fd_alloc_max; i++) { /* init new entries */
			fs[i].valid = FALSE;
		}
	}

	fp = fs+fd;

	if (!fp->valid) {
		/* initialize */
		fp->buffer = malloc((unsigned)(bufsiz+1));
		if (!fp->buffer) return 0;
		fp->msize = bufsiz;
		fp->valid = TRUE;
	}
	fp->buffer_end = fp->buffer;
	fp->match_end = fp->buffer;
	return fp;

}

static
void
exp_setpgrp()
{
#ifdef MIPS_BSD
    /* required on BSD side of MIPS OS <jmsellen@watdragon.waterloo.edu> */
#   include <sysv/sys.s>
    syscall(SYS_setpgrp);
#endif

#ifdef SETPGRP_VOID
    (void) setpgrp();
#else
    (void) setpgrp(0,0);
#endif
}

/* returns fd of master side of pty */
int
exp_spawnv(file,argv)
char *file;
char *argv[];	/* some compiler complains about **argv? */
{
	int cc;
	int errorfd;	/* place to stash fileno(stderr) in child */
			/* while we're setting up new stderr */
	int ttyfd;
	int sync_fds[2];
	int sync2_fds[2];
	char sync_byte;
#ifdef PTYTRAP_DIES
	int slave_write_ioctls = 1;
		/* by default, slave will be write-ioctled this many times */
#endif

	static int first_time = TRUE;

	if (first_time) {
		first_time = FALSE;
		exp_init_pty();
		exp_init_tty();
	}

	if (!file || !argv) sysreturn(EINVAL);
	if (!argv[0] || strcmp(file,argv[0])) {
		exp_debuglog("expect: warning: file (%s) != argv[0] (%s)\n",
			file,
			argv[0]?argv[0]:"");
	}

#ifdef PTYTRAP_DIES
/* any extraneous ioctl's that occur in slave must be accounted for
when trapping, see below in child half of fork */
#if defined(TIOCSCTTY) && !defined(CIBAUD) && !defined(sun) && !defined(hp9000s300)
	slave_write_ioctls++;
#endif
#endif /*PTYTRAP_DIES*/

	if (exp_autoallocpty) {
		if (0 > (exp_pty[0] = getptymaster())) sysreturn(ENODEV);
	}
	fcntl(exp_pty[0],F_SETFD,1);	/* close on exec */
#ifdef PTYTRAP_DIES
	exp_slave_control(exp_pty[0],1);*/
#endif

	if (!fd_new(exp_pty[0])) {
		errno = ENOMEM;
		return -1;
	}

	if (-1 == (pipe(sync_fds))) {
		return -1;
	}
	if (-1 == (pipe(sync2_fds))) {
		return -1;
	}

	if ((exp_pid = fork()) == -1) return(-1);
	if (exp_pid) {
		/* parent */
		close(sync_fds[1]);
		close(sync2_fds[0]);
		if (!exp_autoallocpty) close(exp_pty[1]);

#ifdef PTYTRAP_DIES
#ifdef HAVE_PTYTRAP
		if (exp_autoallocpty) {
			/* trap initial ioctls in a feeble attempt to not */
			/* block the initially.  If the process itself */
			/* ioctls /dev/tty, such blocks will be trapped */
			/* later during normal event processing */

			while (slave_write_ioctls) {
				int cc;

				cc = exp_wait_for_slave_open(exp_pty[0]);
#if defined(TIOCSCTTY) && !defined(CIBAUD) && !defined(sun) && !defined(hp9000s300)
				if (cc == TIOCSCTTY) slave_write_ioctls = 0;
#endif
				if (cc & IOC_IN) slave_write_ioctls--;
				else if (cc == -1) {
					printf("failed to trap slave pty");
					return -1;
				}
			}
		}
#endif
#endif /*PTYTRAP_DIES*/

		/*
		 * wait for slave to initialize pty before allowing
		 * user to send to it
		 */ 

		exp_debuglog("parent: waiting for sync byte\r\n");
		cc = read(sync_fds[0],&sync_byte,1);
		if (cc == -1) {
			fprintf(stderr,"parent sync byte read: %s\r\n",Tcl_ErrnoMsg(errno));
			exit(-1);
		}

		/* turn on detection of eof */
		exp_slave_control(exp_pty[0],1);

		/*
		 * tell slave to go on now now that we have initialized pty
		 */

		exp_debuglog("parent: telling child to go ahead\r\n");
		cc = write(sync2_fds[1]," ",1);
		if (cc == -1) {
			exp_errorlog("parent sync byte write: %s\r\n",Tcl_ErrnoMsg(errno));
			exit(-1);
		}

		exp_debuglog("parent: now unsynchronized from child\r\n");

		close(sync_fds[0]);
		close(sync2_fds[1]);

		return(exp_pty[0]);
	}

	/* child process - do not return from here!  all errors must exit() */

	close(sync_fds[0]);
	close(sync2_fds[1]);

#ifdef CRAY
	(void) close(exp_pty[0]);
#endif

/* ultrix (at least 4.1-2) fails to obtain controlling tty if setsid */
/* is called.  setpgrp works though.  */
#if defined(POSIX) && !defined(ultrix)
#define DO_SETSID
#endif
#ifdef __convex__
#define DO_SETSID
#endif

#ifdef DO_SETSID
	setsid();
#else
#ifdef SYSV3
#ifndef CRAY
	exp_setpgrp();
#endif /* CRAY */
#else /* !SYSV3 */
	exp_setpgrp();

#ifdef TIOCNOTTY
	ttyfd = open("/dev/tty", O_RDWR);
	if (ttyfd >= 0) {
		(void) ioctl(ttyfd, TIOCNOTTY, (char *)0);
		(void) close(ttyfd);
	}
#endif /* TIOCNOTTY */

#endif /* SYSV3 */
#endif /* DO_SETSID */

	/* save error fd while we're setting up new one */
	errorfd = fcntl(2,F_DUPFD,3);
	/* and here is the macro to restore it */
#define restore_error_fd {close(2);fcntl(errorfd,F_DUPFD,2);}

	if (exp_autoallocpty) {

	    close(0);
	    close(1);
	    close(2);

	    /* since we closed fd 0, open of pty slave must return fd 0 */

	    if (0 > (exp_pty[1] = getptyslave(exp_ttycopy,exp_ttyinit,
						exp_stty_init))) {
		restore_error_fd
		fprintf(stderr,"open(slave pty): %s\n",Tcl_ErrnoMsg(errno));
		exit(-1);
	    }
	    /* sanity check */
	    if (exp_pty[1] != 0) {
		restore_error_fd
		fprintf(stderr,"getptyslave: slave = %d but expected 0\n",
								exp_pty[1]);
		exit(-1);
	    }
	} else {
		if (exp_pty[1] != 0) {
			close(0);	fcntl(exp_pty[1],F_DUPFD,0);
		}
		close(1);		fcntl(0,F_DUPFD,1);
		close(2);		fcntl(0,F_DUPFD,1);
		close(exp_pty[1]);
	}



/* The test for hpux may have to be more specific.  In particular, the */
/* code should be skipped on the hp9000s300 and hp9000s720 (but there */
/* is no documented define for the 720!) */

#if defined(TIOCSCTTY) && !defined(sun) && !defined(hpux)
	/* 4.3+BSD way to acquire controlling terminal */
	/* according to Stevens - Adv. Prog..., p 642 */
#ifdef __QNX__ /* posix in general */
	if (tcsetct(0, getpid()) == -1) {
#else
	if (ioctl(0,TIOCSCTTY,(char *)0) < 0) {
#endif
		restore_error_fd
		fprintf(stderr,"failed to get controlling terminal using TIOCSCTTY");
		exit(-1);
	}
#endif

#ifdef CRAY
 	(void) setsid();
 	(void) ioctl(0,TCSETCTTY,0);
 	(void) close(0);
 	if (open("/dev/tty", O_RDWR) < 0) {
		restore_error_fd
 		fprintf(stderr,"open(/dev/tty): %s\r\n",Tcl_ErrnoMsg(errno));
 		exit(-1);
 	}
 	(void) close(1);
 	(void) close(2);
 	(void) dup(0);
 	(void) dup(0);
	setptyutmp();	/* create a utmp entry */

	/* _CRAY2 code from Hal Peterson <hrp@cray.com>, Cray Research, Inc. */
#ifdef _CRAY2
	/*
	 * Interpose a process between expect and the spawned child to
	 * keep the slave side of the pty open to allow time for expect
	 * to read the last output.  This is a workaround for an apparent
	 * bug in the Unicos pty driver on Cray-2's under Unicos 6.0 (at
	 * least).
	 */
	if ((pid = fork()) == -1) {
		restore_error_fd
		fprintf(stderr,"second fork: %s\r\n",Tcl_ErrnoMsg(errno));
		exit(-1);
	}

	if (pid) {
 		/* Intermediate process. */
		int status;
		int timeout;
		char *t;

		/* How long should we wait? */
		timeout = exp_pty_timeout;

		/* Let the spawned process run to completion. */
 		while (wait(&status) < 0 && errno == EINTR)
			/* empty body */;

		/* Wait for the pty to clear. */
		sleep(timeout);

		/* Duplicate the spawned process's status. */
		if (WIFSIGNALED(status))
			kill(getpid(), WTERMSIG(status));

		/* The kill may not have worked, but this will. */
 		exit(WEXITSTATUS(status));
	}
#endif /* _CRAY2 */
#endif /* CRAY */

	if (exp_console) {
#ifdef SRIOCSREDIR
		int fd;

		if ((fd = open("/dev/console", O_RDONLY)) == -1) {
			restore_error_fd
			fprintf(stderr, "spawn %s: cannot open console, check permissions of /dev/console\n",argv[0]);
			exit(-1);
		}
		if (ioctl(fd, SRIOCSREDIR, 0) == -1) {
			restore_error_fd
			fprintf(stderr, "spawn %s: cannot redirect console, check permissions of /dev/console\n",argv[0]);
		}
		close(fd);
#endif

#ifdef TIOCCONS
		int on = 1;
		if (ioctl(0,TIOCCONS,(char *)&on) == -1) {
			restore_error_fd
			fprintf(stderr, "spawn %s: cannot open console, check permissions of /dev/console\n",argv[0]);
			exit(-1);
		}
#endif /* TIOCCONS */
	}

	/* tell parent that we are done setting up pty */
	/* The actual char sent back is irrelevant. */

	/* exp_debuglog("child: telling parent that pty is initialized\r\n");*/
	cc = write(sync_fds[1]," ",1);
	if (cc == -1) {
		restore_error_fd
		fprintf(stderr,"child: sync byte write: %s\r\n",Tcl_ErrnoMsg(errno));
		exit(-1);
	}
	close(sync_fds[1]);

	/* wait for master to let us go on */
	/* exp_debuglog("child: waiting for go ahead from parent\r\n"); */

/*	close(master);	/* force master-side close so we can read */
	cc = read(sync2_fds[0],&sync_byte,1);
	if (cc == -1) {
		restore_error_fd
		exp_errorlog("child: sync byte read: %s\r\n",Tcl_ErrnoMsg(errno));
		exit(-1);
	}
	close(sync2_fds[0]);

	/* exp_debuglog("child: now unsynchronized from parent\r\n"); */

	/* (possibly multiple) masters are closed automatically due to */
	/* earlier fcntl(,,CLOSE_ON_EXEC); */

	/* just in case, allow user to explicitly close other files */
	if (exp_close_in_child) (*exp_close_in_child)();

	/* allow user to do anything else to child */
	if (exp_child_exec_prelude) (*exp_child_exec_prelude)();

        (void) execvp(file,argv);
	/* Unfortunately, by now we've closed fd's to stderr, logfile and
		debugfile.
	   The only reasonable thing to do is to send back the error as
	   part of the program output.  This will be picked up in an
	   expect or interact command.
	*/
	fprintf(stderr,"execvp(%s): %s\n",file,Tcl_ErrnoMsg(errno));
	exit(-1);
	/*NOTREACHED*/
}

/* returns fd of master side of pty */
/*VARARGS*/
int
exp_spawnl TCL_VARARGS_DEF(char *,arg1)
/*exp_spawnl(va_alist)*/
/*va_dcl*/
{
	va_list args; /* problematic line here */
	int i;
	char *arg, **argv;

	arg = TCL_VARARGS_START(char *,arg1,args);
	/*va_start(args);*/
	for (i=1;;i++) {
		arg = va_arg(args,char *);
		if (!arg) break;
	}
	va_end(args);
	if (i == 0) sysreturn(EINVAL);
	if (!(argv = (char **)malloc((i+1)*sizeof(char *)))) sysreturn(ENOMEM);
	argv[0] = TCL_VARARGS_START(char *,arg1,args);
	/*va_start(args);*/
	for (i=1;;i++) {
		argv[i] = va_arg(args,char *);
		if (!argv[i]) break;
	}
	i = exp_spawnv(argv[0],argv+1);
	free((char *)argv);
	return(i);
}

/* allow user-provided fd to be passed to expect funcs */
int
exp_spawnfd(fd)
int fd;
{
	if (!fd_new(fd)) {
		errno = ENOMEM;
		return -1;
	}
	return fd;	
}

/* remove nulls from s.  Initially, the number of chars in s is c, */
/* not strlen(s).  This count does not include the trailing null. */
/* returns number of nulls removed. */
static int
rm_nulls(s,c)
char *s;
int c;
{
	char *s2 = s;	/* points to place in original string to put */
			/* next non-null character */
	int count = 0;
	int i;

	for (i=0;i<c;i++,s++) {
		if (0 == *s) {
			count++;
			continue;
		}
		if (count) *s2 = *s;
		s2++;
	}
	return(count);
}

static int i_read_errno;/* place to save errno, if i_read() == -1, so it
			   doesn't get overwritten before we get to read it */

/*ARGSUSED*/
static void
sigalarm_handler(n)
int n;			/* signal number, unused by us */
{
#ifdef REARM_SIG
	signal(SIGALRM,sigalarm_handler);
#endif

	longjmp(exp_readenv,1);
}

/* interruptable read */
static int
i_read(fd,fp,buffer,length,timeout)
int fd;
FILE *fp;
char *buffer;
int length;
int timeout;
{
	int cc = -2;

	/* since setjmp insists on returning 1 upon longjmp(,0), */
	/* longjmp(,2 (EXP_RESTART)) instead. */

	/* no need to set alarm if -1 (infinite) or 0 (poll with */
	/* guaranteed data) */

	if (timeout > 0) alarm(timeout);

	/* restart read if setjmp returns 0 (first time) or 2 (EXP_RESTART). */
	/* abort if setjmp returns 1 (EXP_ABORT). */
	if (EXP_ABORT != setjmp(exp_readenv)) {
		exp_reading = TRUE;
		if (fd == -1) {
			int c;
			c = getc(fp);
			if (c == EOF) {
/*fprintf(stderr,"<<EOF>>",c);fflush(stderr);*/
				if (feof(fp)) cc = 0;
				else cc = -1;
			} else {
/*fprintf(stderr,"<<%c>>",c);fflush(stderr);*/
				buffer[0] = c;
				cc = 1;
			}
		} else {
#ifndef HAVE_PTYTRAP
			cc = read(fd,buffer,length);
#else
#  include <sys/ptyio.h>

			fd_set rdrs;
			fd_set excep;

		restart:
			FD_ZERO(&rdrs);
			FD_ZERO(&excep);
			FD_SET(fd,&rdrs);
			FD_SET(fd,&excep);
			if (-1 == (cc = select(fd+1,
					 (SELECT_MASK_TYPE *)&rdrs,
					 (SELECT_MASK_TYPE *)0,
					 (SELECT_MASK_TYPE *)&excep,
					 (struct timeval *)0))) {
				/* window refreshes trigger EINTR, ignore */
				if (errno == EINTR) goto restart;
			}
			if (FD_ISSET(fd,&rdrs)) {
				cc = read(fd,buffer,length);
			} else if (FD_ISSET(fd,&excep)) {
				struct request_info ioctl_info;
				ioctl(fd,TIOCREQCHECK,&ioctl_info);
				if (ioctl_info.request == TIOCCLOSE) {
					cc = 0; /* indicate eof */
				} else {
					ioctl(fd, TIOCREQSET, &ioctl_info);
					/* presumably, we trapped an open here */
					goto restart;
				}
			}
#endif /* HAVE_PTYTRAP */
		}
#if 0
		/* can't get fread to return early! */
		else {
			if (!(cc = fread(buffer,1,length,fp))) {
				if (ferror(fp)) cc = -1;
			}
		}
#endif
		i_read_errno = errno;	/* errno can be overwritten by the */
					/* time we return */
	}
	exp_reading = FALSE;

	if (timeout > 0) alarm(0);
	return(cc);
}

/* I tried really hard to make the following two functions share the code */
/* that makes the ecase array, but I kept running into a brick wall when */
/* passing var args into the funcs and then again into a make_cases func */
/* I would very much appreciate it if someone showed me how to do it right */

/* takes triplets of args, with a final "exp_last" arg */
/* triplets are type, pattern, and then int to return */
/* returns negative value if error (or EOF/timeout) occurs */
/* some negative values can also have an associated errno */

/* the key internal variables that this function depends on are:
	exp_buffer
	exp_buffer_end
	exp_match_end
*/
static int
expectv(fd,fp,ecases)
int fd;
FILE *fp;
struct exp_case *ecases;
{
	int cc = 0;		/* number of chars returned in a single read */
	int buf_length;		/* numbers of chars in exp_buffer */
	int old_length;		/* old buf_length */
	int first_time = TRUE;	/* force old buffer to be tested before */
				/* additional reads */
	int polled = 0;		/* true if poll has caused read() to occur */

	struct exp_case *ec;	/* points to current ecase */

	time_t current_time;	/* current time (when we last looked)*/
	time_t end_time;	/* future time at which to give up */
	int remtime;		/* remaining time in timeout */

	struct f *f;
	int return_val;
	int sys_error = 0;
#define return_normally(x)	{return_val = x; goto cleanup;}
#define return_errno(x)	{sys_error = x; goto cleanup;}

	f = fdfp2f(fd,fp);
	if (!f) return_errno(ENOMEM);

	exp_buffer = f->buffer;
	exp_buffer_end = f->buffer_end;
	exp_match_end = f->match_end;

	buf_length = exp_buffer_end - exp_match_end;
	if (buf_length) {
		/*
		 * take end of previous match to end of buffer
		 * and copy to beginning of buffer
		 */
		memmove(exp_buffer,exp_match_end,buf_length);
	}			
	exp_buffer_end = exp_buffer + buf_length;
	*exp_buffer_end = '\0';

	if (!ecases) return_errno(EINVAL);

	/* compile if necessary */
	for (ec=ecases;ec->type != exp_end;ec++) {
		if ((ec->type == exp_regexp) && !ec->re) {
			TclRegError((char *)0);
			if (!(ec->re = TclRegComp(ec->pattern))) {
				fprintf(stderr,"regular expression %s is bad: %s",ec->pattern,TclGetRegError());
				return_errno(EINVAL);
			  }
		  }
	}

	/* get the latest buffer size.  Double the user input for two */
	/* reasons.  1) Need twice the space in case the match */
	/* straddles two bufferfuls, 2) easier to hack the division by */
	/* two when shifting the buffers later on */

	bufsiz = 2*exp_match_max;
	if (f->msize != bufsiz) {
		/* if truncated, forget about some data */
		if (buf_length > bufsiz) {
			/* copy end of buffer down */

			/* copy one less than what buffer can hold to avoid */
			/* triggering buffer-full handling code below */
			/* which will immediately dump the first half */
			/* of the buffer */
			memmove(exp_buffer,exp_buffer+(buf_length - bufsiz)+1,
				bufsiz-1);
			buf_length = bufsiz-1;
		}
		exp_buffer = realloc(exp_buffer,bufsiz+1);
		if (!exp_buffer) return_errno(ENOMEM);
		exp_buffer[buf_length] = '\0';
		exp_buffer_end = exp_buffer + buf_length;
		f->msize = bufsiz;
	}

	/* some systems (i.e., Solaris) require fp be flushed when switching */
	/* directions - do this again afterwards */
	if (fd == -1) fflush(fp);

	if (exp_timeout != -1) signal(SIGALRM,sigalarm_handler);

	/* remtime and current_time updated at bottom of loop */
	remtime = exp_timeout;

	time(&current_time);
	end_time = current_time + remtime;

	for (;;) {
		/* when buffer fills, copy second half over first and */
		/* continue, so we can do matches over multiple buffers */
		if (buf_length == bufsiz) {
			int first_half, second_half;

			if (exp_full_buffer) {
				exp_debuglog("expect: full buffer\r\n");
				exp_match = exp_buffer;
				exp_match_end = exp_buffer + buf_length;
				exp_buffer_end = exp_match_end;
				return_normally(EXP_FULLBUFFER);
			}
			first_half = bufsiz/2;
			second_half = bufsiz - first_half;

			memcpy(exp_buffer,exp_buffer+first_half,second_half);
			buf_length = second_half;
			exp_buffer_end = exp_buffer + second_half;
		}

		/*
		 * always check first if pattern is already in buffer
		 */
		if (first_time) {
			first_time = FALSE;
			goto after_read;
		}

		/*
		 * check for timeout
		 */
		if ((exp_timeout >= 0) && ((remtime < 0) || polled)) {
			exp_debuglog("expect: timeout\r\n");
			exp_match_end = exp_buffer;
			return_normally(EXP_TIMEOUT);
		}

		/*
		 * if timeout == 0, indicate a poll has
		 * occurred so that next time through loop causes timeout
		 */
		if (exp_timeout == 0) {
			polled = 1;
		}

		cc = i_read(fd,fp,
				exp_buffer_end,
				bufsiz - buf_length,
				remtime);

		if (cc == 0) {
			exp_debuglog("expect: eof\r\n");
			return_normally(EXP_EOF);	/* normal EOF */
		} else if (cc == -1) {			/* abnormal EOF */
			/* ptys produce EIO upon EOF - sigh */
			if (i_read_errno == EIO) {
				/* convert to EOF indication */
				exp_debuglog("expect: eof\r\n");
				return_normally(EXP_EOF);
			}
			exp_debuglog("expect: error (errno = %d)\r\n",i_read_errno);
			return_errno(i_read_errno);
		} else if (cc == -2) {
			exp_debuglog("expect: timeout\r\n");
			exp_match_end = exp_buffer;
			return_normally(EXP_TIMEOUT);
		}

		old_length = buf_length;
		buf_length += cc;
		exp_buffer_end += buf_length;

		if (exp_logfile_all || (exp_loguser && exp_logfile)) {
			fwrite(exp_buffer + old_length,1,cc,exp_logfile);
		}
		if (exp_loguser) fwrite(exp_buffer + old_length,1,cc,stdout);
		if (exp_debugfile) fwrite(exp_buffer + old_length,1,cc,exp_debugfile);

		/* if we wrote to any logs, flush them */
		if (exp_debugfile) fflush(exp_debugfile);
		if (exp_loguser) {
			fflush(stdout);
			if (exp_logfile) fflush(exp_logfile);
		}

		/* remove nulls from input, so we can use C-style strings */
		/* doing it here lets them be sent to the screen, just */
		/*  in case they are involved in formatting operations */
		if (exp_remove_nulls) {
			buf_length -= rm_nulls(exp_buffer + old_length, cc);
		}
		/* cc should be decremented as well, but since it will not */
		/* be used before being set again, there is no need */
		exp_buffer_end = exp_buffer + buf_length;
		*exp_buffer_end = '\0';
                exp_match_end = exp_buffer;

	after_read:
		exp_debuglog("expect: does {%s} match ",exp_printify(exp_buffer));
		/* pattern supplied */
		for (ec=ecases;ec->type != exp_end;ec++) {
			int matched = -1;

			exp_debuglog("{%s}? ",exp_printify(ec->pattern));
			if (ec->type == exp_glob) {
				int offset;
				matched = Exp_StringMatch(exp_buffer,ec->pattern,&offset);
				if (matched >= 0) {
					exp_match = exp_buffer + offset;
					exp_match_end = exp_match + matched;
				}
			} else if (ec->type == exp_exact) {
				char *p = strstr(exp_buffer,ec->pattern);
				if (p) {
					matched = 1;
					exp_match = p;
					exp_match_end = p + strlen(ec->pattern);
				}
			} else if (ec->type == exp_null) {
				char *p;

				for (p=exp_buffer;p<exp_buffer_end;p++) {
					if (*p == 0) {
						matched = 1;
						exp_match = p;
						exp_match_end = p+1;
					}
				}
			} else {
				TclRegError((char *)0);
				if (TclRegExec(ec->re,exp_buffer,exp_buffer)) {
					matched = 1;
					exp_match = ec->re->startp[0];
					exp_match_end = ec->re->endp[0];
				} else if (TclGetRegError()) {
			    		fprintf(stderr,"r.e. match (pattern %s) failed: %s",ec->pattern,TclGetRegError());
				}
			}

			if (matched != -1) {
				exp_debuglog("yes\nexp_buffer is {%s}\n",
						exp_printify(exp_buffer));
				return_normally(ec->value);
			} else exp_debuglog("no\n");
		}

		/*
		 * Update current time and remaining time.
		 * Don't bother if we are waiting forever or polling.
		 */
		if (exp_timeout > 0) {
			time(&current_time);
			remtime = end_time - current_time;
		}
	}
 cleanup:
	f->buffer     = exp_buffer;
	f->buffer_end = exp_buffer_end;
	f->match_end  = exp_match_end;

	/* some systems (i.e., Solaris) require fp be flushed when switching */
	/* directions - do this before as well */
	if (fd == -1) fflush(fp);

	if (sys_error) {
		errno = sys_error;
		return -1;
	}
	return return_val;
}

int
exp_fexpectv(fp,ecases)
FILE *fp;
struct exp_case *ecases;
{
	return(expectv(-1,fp,ecases));
}

int
exp_expectv(fd,ecases)
int fd;
struct exp_case *ecases;
{
	return(expectv(fd,(FILE *)0,ecases));
}

/*VARARGS*/
int
exp_expectl TCL_VARARGS_DEF(int,arg1)
/*exp_expectl(va_alist)*/
/*va_dcl*/
{
	va_list args;
	int fd;
	struct exp_case *ec, *ecases;
	int i;
	enum exp_type type;

	fd = TCL_VARARGS_START(int,arg1,args);
	/* va_start(args);*/
	/* fd = va_arg(args,int);*/
	/* first just count the arg sets */
	for (i=0;;i++) {
		type = va_arg(args,enum exp_type);
		if (type == exp_end) break;

		/* Ultrix 4.2 compiler refuses enumerations comparison!? */
		if ((int)type < 0 || (int)type >= (int)exp_bogus) {
			fprintf(stderr,"bad type (set %d) in exp_expectl\n",i);
			sysreturn(EINVAL);
		}

		va_arg(args,char *);		/* COMPUTED BUT NOT USED */
		if (type == exp_compiled) {
			va_arg(args,regexp *);	/* COMPUTED BUT NOT USED */
		}
		va_arg(args,int);		/* COMPUTED BUT NOT USED*/
	}
	va_end(args);

	if (!(ecases = (struct exp_case *)
				malloc((1+i)*sizeof(struct exp_case))))
		sysreturn(ENOMEM);

	/* now set up the actual cases */
	fd = TCL_VARARGS_START(int,arg1,args);
	/*va_start(args);*/
	/*va_arg(args,int);*/		/*COMPUTED BUT NOT USED*/
	for (ec=ecases;;ec++) {
		ec->type = va_arg(args,enum exp_type);
		if (ec->type == exp_end) break;
		ec->pattern = va_arg(args,char *);
		if (ec->type == exp_compiled) {
			ec->re = va_arg(args,regexp *);
		} else {
			ec->re = 0;
		}
		ec->value = va_arg(args,int);
	}
	va_end(args);
	i = expectv(fd,(FILE *)0,ecases);

	for (ec=ecases;ec->type != exp_end;ec++) {
		/* free only if regexp and we compiled it for user */
		if (ec->type == exp_regexp) {
			free((char *)ec->re);
		}
	}
	free((char *)ecases);
	return(i);
}

int
exp_fexpectl TCL_VARARGS_DEF(FILE *,arg1)
/*exp_fexpectl(va_alist)*/
/*va_dcl*/
{
	va_list args;
	FILE *fp;
	struct exp_case *ec, *ecases;
	int i;
	enum exp_type type;

	fp = TCL_VARARGS_START(FILE *,arg1,args);
	/*va_start(args);*/
	/*fp = va_arg(args,FILE *);*/
	/* first just count the arg-pairs */
	for (i=0;;i++) {
		type = va_arg(args,enum exp_type);
		if (type == exp_end) break;

		/* Ultrix 4.2 compiler refuses enumerations comparison!? */
		if ((int)type < 0 || (int)type >= (int)exp_bogus) {
			fprintf(stderr,"bad type (set %d) in exp_expectl\n",i);
			sysreturn(EINVAL);
		}

		va_arg(args,char *);		/* COMPUTED BUT NOT USED */
		if (type == exp_compiled) {
			va_arg(args,regexp *);	/* COMPUTED BUT NOT USED */
		}
		va_arg(args,int);		/* COMPUTED BUT NOT USED*/
	}
	va_end(args);

	if (!(ecases = (struct exp_case *)
					malloc((1+i)*sizeof(struct exp_case))))
		sysreturn(ENOMEM);

#if 0
	va_start(args);
	va_arg(args,FILE *);		/*COMPUTED, BUT NOT USED*/
#endif
	(void) TCL_VARARGS_START(FILE *,arg1,args);

	for (ec=ecases;;ec++) {
		ec->type = va_arg(args,enum exp_type);
		if (ec->type == exp_end) break;
		ec->pattern = va_arg(args,char *);
		if (ec->type == exp_compiled) {
			ec->re = va_arg(args,regexp *);
		} else {
			ec->re = 0;
		}
		ec->value = va_arg(args,int);
	}
	va_end(args);
	i = expectv(-1,fp,ecases);

	for (ec=ecases;ec->type != exp_end;ec++) {
		/* free only if regexp and we compiled it for user */
		if (ec->type == exp_regexp) {
			free((char *)ec->re);
		}
	}
	free((char *)ecases);
	return(i);
}

/* like popen(3) but works in both directions */
FILE *
exp_popen(program)
char *program;
{
	FILE *fp;
	int ec;

	if (0 > (ec = exp_spawnl("sh","sh","-c",program,(char *)0))) return(0);
	if (!(fp = fdopen(ec,"r+"))) return(0);
	setbuf(fp,(char *)0);
	return(fp);
}

int
exp_disconnect()
{
	int ttyfd;

#ifndef EALREADY
#define EALREADY 37
#endif

	/* presumably, no stderr, so don't bother with error message */
	if (exp_disconnected) sysreturn(EALREADY);
	exp_disconnected = TRUE;

	freopen("/dev/null","r",stdin);
	freopen("/dev/null","w",stdout);
	freopen("/dev/null","w",stderr);

#ifdef POSIX
	setsid();
#else
#ifdef SYSV3
	/* put process in our own pgrp, and lose controlling terminal */
	exp_setpgrp();
	signal(SIGHUP,SIG_IGN);
	if (fork()) exit(0);	/* first child exits (as per Stevens, */
	/* UNIX Network Programming, p. 79-80) */
	/* second child process continues as daemon */
#else /* !SYSV3 */
	exp_setpgrp();
/* Pyramid lacks this defn */
#ifdef TIOCNOTTY
	ttyfd = open("/dev/tty", O_RDWR);
	if (ttyfd >= 0) {
		/* zap controlling terminal if we had one */
		(void) ioctl(ttyfd, TIOCNOTTY, (char *)0);
		(void) close(ttyfd);
	}
#endif /* TIOCNOTTY */
#endif /* SYSV3 */
#endif /* POSIX */
	return(0);
}

/* send to log if open and debugging enabled */
/* send to stderr if debugging enabled */
/* use this function for recording unusual things in the log */
/*VARARGS*/
void
exp_debuglog TCL_VARARGS_DEF(char *,arg1)
{
    char *fmt;
    va_list args;

    fmt = TCL_VARARGS_START(char *,arg1,args);
    if (exp_debugfile) vfprintf(exp_debugfile,fmt,args);
    if (exp_is_debugging) {
	vfprintf(stderr,fmt,args);
	if (exp_logfile) vfprintf(exp_logfile,fmt,args);
    }

    va_end(args);
}


/* send to log if open */
/* send to stderr */
/* use this function for error conditions */
/*VARARGS*/
void
exp_errorlog TCL_VARARGS_DEF(char *,arg1)
{
    char *fmt;
    va_list args;
    
    fmt = TCL_VARARGS_START(char *,arg1,args);
    vfprintf(stderr,fmt,args);
    if (exp_debugfile) vfprintf(exp_debugfile,fmt,args);
    if (exp_logfile) vfprintf(exp_logfile,fmt,args);
    va_end(args);
}

#include <ctype.h>

/* exp_printify - printable versions of random ASCII strings */
char *
exp_printify(s) /* INTL */
char *s;
{
	static int destlen = 0;
	static char *dest = 0;
	char *d;		/* ptr into dest */
	unsigned int need;

	if (s == 0) return("<null>");

	/* worst case is every character takes 4 to printify */
	need = strlen(s)*6 + 1;
	if (need > destlen) {
		if (dest) ckfree(dest);
		dest = (char *)ckalloc(need);
		destlen = need;
	}

	for (d = dest;*s;) {
		if (*s == '\r') {
			strcpy(d,"\\r");		d += 2;
		} else if (*s == '\n') {
			strcpy(d,"\\n");		d += 2;
		} else if (*s == '\t') {
			strcpy(d,"\\t");		d += 2;
		} else if (isascii(*s) && isprint(*s)) {
			*d = *s;			d += 1;
		} else {
			sprintf(d,"\\x%02x",*s & 0xff);	d += 4;
		}
	}
	*d = '\0';
	return(dest);
}
