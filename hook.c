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

/*
Initialization and exiting multiple times is not possible according to
The Glorious Glasgow Haskell Compilation System User's Guide's
section 13.1.1.8, so we have to do a little shuffle.
*/

static bool haskell = false;

void stop_the_haskell(void) {
	if (haskell) {
		hs_exit();

		haskell = false;
	}
}

void start_the_haskell(void) {
	if (!haskell) {
		hs_init(NULL, NULL);

#ifdef __GLASGOW_HASKELL__
		hs_add_root(__stginit_Hook);
#endif

		atexit(stop_the_haskell);

		haskell = true;
	}
}

#define GENE(type, procedure, ...) \
	static type (* the_##procedure)(__VA_ARGS__) = NULL; \
\
	static void get_the_##procedure(void) { \
		if (the_##procedure == NULL) { \
			the_##procedure = dlsym(RTLD_NEXT, #procedure); \
			if (the_##procedure == NULL) \
				fprintf(stderr, "dlsym: %s\n", dlerror()); \
		} \
	} \
\
	type procedure(__VA_ARGS__) { \
		int result; \
\
		get_the_##procedure(); \
\
		start_the_haskell(); \
\
		result = hook(#procedure); \
		if (result != 0) \
			fprintf(stderr, "hook: %s: error %d\n", #procedure, result); \

#define RATE(procedure, ...) \
		return the_##procedure(__VA_ARGS__); \
	}

#define RATED(procedure, ...) \
		the_##procedure(__VA_ARGS__); \
	}

GENE(void, abort, void) RATED(abort)
GENE(void, exit, int status) RATED(exit, status)
GENE(void, _exit, int status) RATED(_exit, status)
GENE(void, _Exit, int status) RATED(_Exit, status)
GENE(pid_t, fork, void) RATE(fork)
GENE(pid_t, vfork, void) RATE(vfork)
