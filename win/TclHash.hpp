/*
 ------------------------------------------------------------------------------
 * TclHash.hpp --
 *
 *   Tcl's hash table done as a template.
 *
 * Copyright (c) 1999-2001 Tomahawk Software Group
 *
 * See the file "license.txt" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * RCS: @(#) $Id: TclHash.hpp,v 1.9 2001/09/02 04:19:47 daveg Exp $
 ------------------------------------------------------------------------------
 */
#ifndef INC_tclhash_hpp__
#define INC_tclhash_hpp__

namespace Tcl {

    template <class T, int keytype = TCL_STRING_KEYS>
	class Hash
    {
    public:
	Hash () { Tcl_InitHashTable(&HashTbl, keytype); }
	~Hash () { Tcl_DeleteHashTable(&HashTbl); }
	Tcl_Obj *Stats ();
	int Add (void *key, T result);
	int Find (void *key, T *result);
	int Delete (void *key);

	Tcl_HashTable HashTbl;
    };

    template <class T, int keytype>
	Tcl_Obj *Hash<T, keytype>::Stats ()
    {
	char *stats;
	Tcl_Obj *oStats;

	stats = Tcl_HashStats(&HashTbl);
	oStats = Tcl_NewStringObj(stats, -1);
	ckfree(stats);
	return oStats;
    }

    template <class T, int keytype>
	int Hash<T, keytype>::Add (void *key, T result)
    {
	int created;
	Tcl_HashEntry *entryPtr;

	entryPtr = Tcl_CreateHashEntry(&HashTbl, static_cast<const char *>(key),
	    &created);

	if (!created) {
	    return TCL_ERROR;
	}
	Tcl_SetHashValue(entryPtr, result);
	return TCL_OK;
    }

    template <class T, int keytype>
	int Hash<T, keytype>::Find (void *key, T *result)
    {
	Tcl_HashEntry *entryPtr;

	entryPtr = Tcl_FindHashEntry(&HashTbl, static_cast<const char *>(key));
	if (entryPtr == 0L) {
	    return TCL_ERROR;
	}
	if (result != 0L) {
	    *result = static_cast<T>(Tcl_GetHashValue(entryPtr));
	}
	return TCL_OK;
    }

    template <class T, int keytype>
	int Hash<T, keytype>::Delete (void *key)
    {
	Tcl_HashEntry *entryPtr;

	entryPtr = Tcl_FindHashEntry(&HashTbl, static_cast<const char *>(key));
	if (entryPtr == 0L) {
	    return TCL_ERROR;
	}
	Tcl_DeleteHashEntry(entryPtr);
	return TCL_OK;
    }

};	// namespace Tcl
#endif	// #ifndef INC_tclhash_hpp__

