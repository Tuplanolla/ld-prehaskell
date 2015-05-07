#include <HsFFI.h>
#include <dlfcn.h>
#include <signal.h>
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
section 13.1.1.8, so we have to perform a little shuffle and
hope abnormal termination does not cause trouble.
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

/*
The following conjuration delays the expansion of a preprocessor directive,
making it is possible to pass variable-length argument lists around.
*/

#define DELAY(...) __VA_ARGS__

#define HOOK(type, name, parameters, ...) \
	static type (* the_##name)(parameters) = NULL; \
\
	static void get_the_##name(void) { \
		if (the_##name == NULL) { \
			the_##name = dlsym(RTLD_NEXT, #name); \
			if (the_##name == NULL) \
				fprintf(stderr, "dlsym: %s\n", dlerror()); \
		} \
	} \
\
	type name(parameters) { \
		int result; \
\
		get_the_##name(); \
\
		start_the_haskell(); \
\
		result = hook(#name); \
		if (result != 0) \
			fprintf(stderr, "hook: %s: error %d\n", #name, result); \
\
		__VA_ARGS__; \
	}

/*
Create a hook for a procedure that is called and
whose return value is passed through.
*/
#define HOOK_CALL(type, name, parameters, arguments) \
	HOOK(type, name, DELAY(parameters), return the_##name(arguments))

/*
Create a hook for a procedure that is called, but
does not have a return value.
*/
#define HOOK_CALL_VOID(name, parameters, arguments) \
	HOOK(void, name, DELAY(parameters), the_##name(arguments))

/*
Create a hook for a procedure that is not called, but
return a fixed value instead.
*/
#define HOOK_OMIT(type, name, parameters, value) \
	HOOK(type, name, DELAY(parameters), return value)

/*
Create a hook for a procedure that is not called and
does not have a return value.
*/
#define HOOK_OMIT_VOID(name, parameters) \
	HOOK(void, name, DELAY(parameters))

/*
Examples to tamper with random numbers and
monitor creating and controlling processes follow.
*/

HOOK_CALL(pid_t, fork, DELAY(void), DELAY())
HOOK_CALL(pid_t, vfork, DELAY(void), DELAY())
HOOK_CALL(int, kill, DELAY(pid_t pid, int sig), DELAY(pid, sig))

HOOK_CALL_VOID(abort, DELAY(void), DELAY())
HOOK_CALL_VOID(exit, DELAY(int status), DELAY(status))

HOOK_OMIT(int, rand, DELAY(), 42)

HOOK_OMIT_VOID(srand, DELAY(unsigned int seed))
