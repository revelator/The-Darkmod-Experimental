#include "idlib/precompiled.h"
#include "renderer/tr_local.h"
#include "sys/linux/local.h"

#include <dlfcn.h>

dnl =====================================================
dnl code
dnl =====================================================

static void *glHandle = NULL;

/*
======================
GLimp_dlopen
======================
*/
bool GLimp_dlopen() {
	const char *driverName = r_glDriver.GetString()[0] ? r_glDriver.GetString() : "libGL.so.1";
	common->Printf("dlopen(%s)\n", driverName);
	if ( !( glHandle = dlopen( driverName, RTLD_NOW | RTLD_GLOBAL ) ) ) {
		common->DPrintf("dlopen(%s) failed: %s\n", driverName, dlerror());
		return false;
	}
	return true;
}

/*
======================
GLimp_dlclose
======================
*/
void GLimp_dlclose() {
	if ( !glHandle ) {
		common->DPrintf("dlclose: GL handle is NULL\n");
	} else {	
		dlclose( glHandle );
		glHandle = NULL;
	}
}
