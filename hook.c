#include <HsFFI.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef __GLASGOW_HASKELL__
#include "Hook_stub.h"
extern void __stginit_Hook(void);
#endif

/*
Initialization and exiting multiple times is not possible according to
The Glorious Glasgow Haskell Compilation System User's Guide's
section 13.1.1.8, so we have to do a little shuffle.
*/

static int haskell = 0;

void stop_the_haskell(void) {
	if (haskell != 0) {
		hs_exit();

		haskell = 0;
	}
}

void start_the_haskell(void) {
	if (haskell == 0) {
		hs_init(NULL, NULL);

#ifdef __GLASGOW_HASKELL__
		hs_add_root(__stginit_Hook);
#endif

		atexit(stop_the_haskell);

		haskell = 1;
	}
}

#define GENERATE(type, procedure) \
	static type (* the_##procedure)(void) = NULL; \
\
	static void get_the_##procedure(void) { \
		if (the_##procedure == NULL) { \
			the_##procedure = dlsym(RTLD_NEXT, #procedure); \
			if (the_##procedure == NULL) \
				fprintf(stderr, "dlsym: %s\n", dlerror()); \
		} \
	} \
\
	type procedure(void) { \
		int result; \
\
		get_the_##procedure(); \
\
		start_the_haskell(); \
\
		result = hook(#procedure); \
		if (result != 0) \
			fprintf(stderr, "hook: %s: error %d\n", #procedure, result); \
\
		return the_##procedure(); \
	} \

GENERATE(pid_t, fork)
GENERATE(pid_t, vfork)

/*
GENERATE(exit)
GENERATE(_exit)
GENERATE(_Exit)
GENERATE(clone)
GENERATE(__clone2)
GENERATE(posix_spawn)
GENERATE(posix_spawnp)
*/
