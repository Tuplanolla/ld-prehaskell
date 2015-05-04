CC=gcc
GHC=ghc

all: Hook.so

clean:
	$(RM) *_stub.h *.hi *.o *.so

run: Hook.so
	LD_PRELOAD=./Hook.so find . -maxdepth 1 -not -name '.*' -exec file {} \;

Hook.so: Hook.o hook.o
	$(GHC) -dynamic -fPIC -o $@ -shared $^ \
		-optl-Wl,-rpath,/usr/lib/ghc -ldl -lHSrts_debug-ghc7.6.3

hook.o: hook.c Hook_stub.h
	$(GHC) -c -dynamic -fPIC -optc -D_GNU_SOURCE $<

Hook.o Hook_stub.h: Hook.hs
	$(GHC) -c -dynamic -fPIC $<

.PHONY: all clean run
