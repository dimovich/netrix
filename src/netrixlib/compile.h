// Compilation config

#ifndef __NXLIBCOMPILE_H__
#define __NXLIBCOMPILE_H__

#include "../global_compile.h"

//for faster build
#define WIN32_LEAN_AND_MEAN
#define NOSOUND
#define NORPC
#define NODRIVERS
#define NOPROFILER
#define NOMETAFILE
#define NOSYSCOMMANDS

// In case we want a clean, non-debug compilation
//#define CLEAN_COMPILE

#ifdef CLEAN_COMPILE
	// tweaks to build a smaller executable
	#pragma comment( linker, "/MERGE:.rdata=.text" )
	#pragma comment( linker, "/NODEFAULTLIB" )

	#pragma warning( disable: 4090 )
	#pragma warning( disable: 4047 )
	#pragma warning( disable: 4018 )
	#pragma warning( disable: 4013 )
	#pragma warning( disable: 4244 )

#else

	#define DEBUG_COMPILE
	#define ENABLE_LOG
#endif


#endif