# exp.decls --
#
#	This file contains the declarations for all supported public
#	functions that are exported by the Expect library via the stubs table.
#	This file is used to generate the expDecls.h, expPlatDecls.h,
#	expIntDecls.h, and expStub.c files.
#
# RCS: @(#) $Id: tcl.decls,v 1.63 2001/09/28 01:21:53 dgp Exp $

library exp

# Define the tcl interface with several sub interfaces:
#     expPlat	 - platform specific public
#     expInt	 - generic private
#     expPlatInt - platform specific private

interface exp
hooks {expPlat expInt expIntPlat}

# Declare each of the functions in the public Expect interface.  Note that
# the an index should never be reused for a different function in order
# to preserve backwards compatibility.

declare 0 generic {
	int Expect_Init (Tcl_Interp *interp)
}
declare 1 generic {
	int Expect_SafeInit (Tcl_Interp *interp)
}

interface expPlat

#====================================================================================
# UNIX specific publics.

declare 0 unix {
}

#====================================================================================
# WIN32 specific publics.



#====================================================================================
# MAC specific publics.

### You nutts!!  can't do Mac... sorry..
