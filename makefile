CC=gcc
GHC=ghc
LDPATH=/usr/lib/ghc

all: Hook.so

clean:
	$(RM) *_stub.h *.hi *.o *.so

run: Hook.so
	LD_PRELOAD=./Hook.so find . -maxdepth 1 -not -name '.*' -exec file {} \;

Hook.so: Hook.o hook.o
	$(GHC) -dynamic -fPIC -o $@ -shared $^ -ldl \
		-optl-Wl,-rpath,'$(LDPATH)' \
		-l"`find '$(LDPATH)' -name '*HSrts*.so' \
		-exec basename -s .so {} \; -quit | head -n 1 | cut -c 4-`"

hook.o: hook.c Hook_stub.h
	$(GHC) -c -dynamic -fPIC -optc-D_GNU_SOURCE -optc-w $<

Hook.o Hook_stub.h: Hook.hs
	$(GHC) -c -dynamic -fPIC $<

.PHONY: all clean run
