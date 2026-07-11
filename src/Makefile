#
# MAKEFILE for Rigal in C, version 4.45.1
#

# C compiler to use (Compiler path can be changed by the user)
CCFLAGS = -g -D LE -m32 -fcommon -I/usr/include/malloc/
PCA =  cc $(CCFLAGS) -D STATIC_CCFLAGS='"$(CCFLAGS)"' -c -D xsun=1 -I../include/
# Remove -DLE for Big-endian architectures (Little-endian is now the default in the Makefile)!
# Specify -Dbbs for BBS-version of scaner (LaTeX version is default)!

LIBS = -lm
# This is for sin, cos, sqrt etc.

# -------------------------------------------------
#  MACRO DEFINITIONS
# -----------------------------------------------
# Object files for "rc"
RCF_SRC = defsun3.o ley_.o nef2.o ou2.o  sevice.o
# They are in src/ subdirectory

# Object files for "ic"
ICF_SRC = cim.o defsun3.o erm.o ic.o ley_.o scan.o scanmif.o usemod.o nef2.o ou2.o sevice.o


# Object files for "rc -c" (library)
CCF_SRC = defsun3.o ley_.o scan.o scanmif.o usemod.o c1.o c2.o c3.o c4.o nef2.o ou2.o sevice.o

DISTLIB =  defsun3.c ley_.c scan.c scanmif.c usemod.c c1.c c2.c c3.c c4.c nef2.c ou2.c sevice.c

# All include files
INCLIB_SRC = ../include/c1.h ../include/define.h ../include/defpage.h ../include/ley.h \
	../include/scan.h ../include/cim.h ../include/def180.h ../include/nef2.h \
	../include/usemod.h ../include/globrig.h

#--------------------------------------------------------
# TARGET DEFINITIONS
#---------------------------------------------------------

# Normally builds all necessary after unpack and performs test.
all: bins riglib.a anrig genrigd rig_lint test clean

bins: bin rc ic v

bin:
	mkdir -p ../bin

rc: $(RCF_SRC) rc_.o
	cc $(CCFLAGS)  -o ../bin/rc rc_.o $(RCF_SRC)

ic: $(ICF_SRC)
	cc  $(CCFLAGS) -o ../bin/ic    $(ICF_SRC) $(LIBS)

v: $(RCF_SRC) v.o
	cc  $(CCFLAGS) -o ../bin/v  v.o  $(RCF_SRC) $(LIBS)

riglib.a: $(CCF_SRC)
	@echo '*Starting creating library'
	mkdir -p ../lib
	rm -f ../lib/riglib.a
	ar rv ../lib/riglib.a $(CCF_SRC)
	ranlib ../lib/riglib.a
	@echo '*Library created'

anrig: $(INCLIB_SRC) ../lib/riglib.a
	cc  $(CCFLAGS) -w \
		-Ianrig/ -I../include/ \
		anrig/xcrg.c  anrig/xcrg_0.c \
		anrig/xcrg_1.c \
		anrig/xcrg_2.c \
		anrig/xcrg_3.c \
		anrig/xcrg_4.c \
		anrig/xcrg_5.c \
		anrig/xcrg_6.c \
		anrig/xcrg_7.c \
		anrig/xcrg_8.c \
		anrig/xcrg_9.c \
		-o ../bin/anrig ../lib/riglib.a $(LIBS)
		-rm -f anrig/*.o

genrigd:$(INCLIB_SRC) ../lib/riglib.a
	cc  $(CCFLAGS) -w \
		-Igenrig/ -I../include/ \
		genrig/xcrg.c \
		genrig/xcrg_1.c \
		genrig/xcrg_2.c \
		genrig/xcrg_3.c \
		genrig/xcrg_4.c \
		genrig/xcrg_5.c \
		genrig/xcrg_6.c \
		genrig/xcrg_7.c \
		genrig/xcrg_8.c \
		genrig/xcrg_9.c \
		genrig/xcrg_10.c \
		-o ../bin/genrigd ../lib/riglib.a $(LIBS)
		-rm -f genrig/*.o

rig_lint: ../lint/rig_lint.rig ../bin/rc ../lib/riglib.a ../bin/anrig ../bin/genrigd
	(cd ../lint;../bin/rc rig_lint -c -P '-O';cd ..)
	-mv ../lint/rig_lint ../bin/

test: ../test/rigcrf.rig ../test/tail.rig ../test/tail.expected ../bin/rc ../lib/riglib.a ../bin/anrig ../bin/genrigd
	(cd ../test; \
	 ../bin/rc rigcrf; \
	 ../bin/ic rigcrf rigcrf; \
	 ../bin/rc  rigcrf -c; \
	 ./rigcrf rigcrf)
	(cd ../test; \
	 rm -f tail.rsc tail.out; \
	 ../bin/rc tail && \
	 ../bin/ic tail tail > tail.out 2>&1 && \
	 diff -u tail.expected tail.out && \
	 rm -f tail.rsc tail.out)

##------------------ Hidden possibilities
z:       anrigz genrigz

anrigz:
	(cd anrig;\
	 ../../bin/rc anrig;\
	 ../../bin/anrig;\
         ../../bin/genrigd S -p N.TMP;\
	 /bin/rm *.RC2 *.RC4 *.TMP)

genrigz:
	(cd genrig;\
	 ../../bin/rc genrigd;\
	 ../../bin/anrig;\
         ../../bin/genrigd S -p N.TMP;\
	 /bin/rm *.RC2 *.RC4 *.TMP)
##---------------------------------------
VERSION = 446
tar:	clean
	-rm -f ../rigsc.$(VERSION).tar.Z
	(cd ..;tar -cvf rigsc.$(VERSION).tar include src examples lint doc test)
	compress ../rigsc.$(VERSION).tar
	cp ../rigsc.$(VERSION).tar.Z /info/ftp/pub/labs/pelab/rigal/
##--------------------------------------
clean:
	mv ../test/rigcrf.rig .
	rm -f *.o */*.o  *.RC2 */*.RC2  *.RC4 */*.RC4    */*.rsc  ../lint/*.o ../lint/*.RC2 ../lint/*.RC4  \
	 ../examples/hanoi ../*/*.rsc ../*/*.out ../*/*/*.out ../test/*
	mv rigcrf.rig ../test/

fullclean: clean
	rm -rf ../bin ../lib ../*.tar

##--------------------------------------
%.o: %.c $(INCLIB_SRC)
	$(PCA) $< -o $@
##----------------------------------------
# Prepare distribution
mkdist:
	@test -n "$(RFILE)" -a -n "$(RDIR)" || \
		{ echo '*** to prepare for distribution write make mkdist RFILE=fff RDIR=/u/vaden/xyz/xyz/'; exit 1; }
	(cd $(RDIR);\
	../bin/rc $(RFILE);\
	../bin/anrig;\
	../bin/genrigd S -p N.TMP;\
	/bin/rm *.RC2 *.RC4 *.TMP)
	cp ../include/*  $(RDIR)
	cp $(DISTLIB) $(RDIR)
	(cd $(RDIR);\
	echo 'cc -O  -w  -D xsun=1 ' $(DISTLIB) ' xc*.c -o ' $(RFILE) $(LIBS)   > cmd)
mkdist2:
	(cd 	$(RDIR);\
	cc -O -w -D xsun=1  $(DISTLIB)  xc*.c -o $(RFILE) $(LIBS)  ;/bin/rm *.o)
