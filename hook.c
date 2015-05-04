#include <HsFFI.h>
#include <dlfcn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef __GLASGOW_HASKELL__
#include "Hook_stub.h"
extern void __stginit_Hook(void);
#endif

static bool haskell = false;

void stop_the_haskell(void) {
	if (haskell)
		hs_exit();
}

void start_the_haskell(void) {
	hs_init(NULL, NULL);

#ifdef __GLASGOW_HASKELL__
	hs_add_root(__stginit_Hook);
#endif

	atexit(stop_the_haskell);
}

/*
Initialization and exiting multiple times is currently not possible according to
The Glorious Glasgow Haskell Compilation System User's Guide's section 13.1.1.8.
*/

#define GENERATE(type, procedure) \
	static type (* the_##procedure)(void) = NULL; \
\
	static void get_the_##procedure(void) { \
		the_##procedure = dlsym(RTLD_NEXT, "fork"); \
		if (the_##procedure == NULL) \
			fprintf(stderr, "dlsym: %s\n", dlerror()); \
	} \
\
	type procedure(void) { \
		if (the_##procedure == NULL) \
			get_the_##procedure(); \
\
		if (!haskell) \
			start_the_haskell(); \
\
		if (hook(#procedure) != 0) \
			fprintf(stderr, "hook: %s: Failure\n", #procedure); \
\
		return the_##procedure(); \
	} \

GENERATE(pid_t, fork)
GENERATE(pid_t, vfork)
