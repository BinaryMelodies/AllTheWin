
# Windows 1 resource compiler
#WIN1RC =
# Windows 2 resource compiler
#WIN2RC =
# Watcom compiler for 8086
#WCL =
# Watcom compiler for 386
#WCL386 =
# Watcom linker
#WLINK =
# Watcom resource compiler
#WRC =
# Netwide Assembler
#NASM =
# MinGW 32-bit C compiler
#MINGW32CC =
#MINGW32RC =
#MINGW32LD =
#MINGW32CRT =
#MINGW32LDFLAGS =
# MinGW 64-bit C compiler
#MINGW64CC =
#MINGW64RC =
#MINGW64LD =
#MINGW64CRT =
#MINGW64LDFLAGS =
# MinGW CE ARM compiler
#MINGWARMCC =
#MINGWARMRC =
# MinGW CE x86 compiler
#MINGW386CC =
#MINGW386RC =

all: out/win1x.exe out/win2x.exe out/win3x.exe out/win4x.exe out/win5x.exe out/win6x.exe out/armce.exe out/x86ce.exe

really_all: all out/win1x0.exe out/win1x1.exe out/win4xw.exe out/gnu5x.exe out/win6x32.exe

force:
	rm -rf out *.obj *.exe *.o
	make all

clean:
	rm -rf out *.res *.obj *.o *.exe *.LOG

distclean: clean
	rm -rf *~

start16.obj: start16.asm
	$(NASM) -fobj $< -o $@

start32.obj: start32.asm
	$(NASM) -fobj $< -o $@

start32w.obj: start32.asm
	$(NASM) -fobj $< -o $@ -DUNICODE=1

start32w.o: start32.asm
	$(NASM) -fwin32 $< -o $@ -DUNICODE=1 -DGNU

start64.o: start64.asm
	$(NASM) -fwin64 $< -o $@ -DUNICODE=1 -DGNU

# Note: -zWs would also be sufficient
win1x.obj: main.c main.h
	$(WCL) -zW  -l=windows -q $< -c -fo=$@ -dVERSION_WIN1=1 -dTARGET_WIN16=1 -s

win2x.obj: main.c main.h
	$(WCL) -zW  -l=windows -q $< -c -fo=$@ -dVERSION_WIN2=1 -dTARGET_WIN16=1 -s

# Note: -zWs is still needed for the callbacks
win3x.obj: main.c main.h
	$(WCL) -zWs -l=windows -q $< -c -fo=$@ -dVERSION_WIN3=1 -dTARGET_WIN16=1 -s

win4x.obj: main.c main.h
	$(WCL386)   -l=nt_win  -q $< -c -fo=$@ -dVERSION_WIN4=1 -dTARGET_WIN32=1 -s

win4xw.obj: main.c main.h
	$(WCL386)   -l=nt_win  -q $< -c -fo=$@ -dVERSION_WIN4=1 -dTARGET_WIN32=1 -s -DUNICODE -D_UNICODE

win5x.obj: main.c main.h
	$(WCL386)   -l=nt_win  -q $< -c -fo=$@ -dVERSION_WIN5=1 -dTARGET_WIN32=1 -s -DUNICODE -D_UNICODE

win5x.o: main.c main.h
	$(MINGW32CC) -c $< -o $@ -DVERSION_WIN5=1 -DTARGET_WIN32=1 -s -DUNICODE -D_UNICODE

win6x32.o: main.c main.h
	$(MINGW32CC) -c $< -o $@ -DVERSION_WIN6=1 -DTARGET_WIN32=1 -s -DUNICODE -D_UNICODE

win6x.o: main.c main.h
	$(MINGW64CC) -c $< -o $@ -DVERSION_WIN6=1 -DTARGET_WIN64=1 -s -DUNICODE -D_UNICODE

armce.o: main.c main.h
	$(MINGWARMCC) -c $< -o $@ -DTARGET_WINCE=1 -s -DUNICODE -D_UNICODE

x86ce.o: main.c main.h
	$(MINGW386CC) -c $< -o $@ -DTARGET_WINCE=1 -s -DUNICODE -D_UNICODE

stdio.obj: stdio.c
	$(WCL) -zW  -l=windows -q $< -c -fo=$@ -s

# Note: the segments don't need to be movable
# Windows 1.x
win1x.exe: start16.obj win1x.obj stdio.obj
	$(WLINK) option quiet system windows name $@ $(patsubst %,file %,$^) option version=1 library clibs library windows option start=start segment TYPE CODE MOVEABLE segment TYPE DATA MOVEABLE

# Windows 2.x
win2x.exe: start16.obj win2x.obj stdio.obj
	$(WLINK) option quiet system windows name $@ $(patsubst %,file %,$^) option version=2 library clibs library windows option start=start segment TYPE CODE MOVEABLE segment TYPE DATA MOVEABLE

# Windows 3.x
win3x.exe: start16.obj win3x.obj stdio.obj
	$(WLINK) option quiet system windows name $@ $(patsubst %,file %,$^) option version=3 library clibs library windows option start=start

# Windows 95 (ANSI)
win4x.exe: start32.obj win4x.obj
	$(WLINK) option quiet system win95   name $@ $(patsubst %,file %,$^) option version=4 library clib3r library user32 library kernel32 library gdi32 option start=start

# Windows NT (Unicode)
win4xw.exe: start32w.obj win4xw.obj
	$(WLINK) option quiet system win95   name $@ $(patsubst %,file %,$^) option version=4 library clib3r library user32 library kernel32 library gdi32 option start=start

# Windows XP (Watcom)
win5x.exe: start32w.obj win5x.obj
	$(WLINK) option quiet system nt_win  name $@ $(patsubst %,file %,$^) option version=5 library clib3r library user32 library kernel32 library comctl32 option start=start

res5x.o: main.rc main.h win4.ico
	$(MINGW32RC) $< -o $@ -DVERSION_WIN5=1 -DTARGET_WIN32=1

res6x32.o: main.rc main.h win4.ico
	$(MINGW32RC) $< -o $@ -DVERSION_WIN6=1 -DTARGET_WIN32=1

res6x.o: main.rc main.h win4.ico
	$(MINGW64RC) $< -o $@ -DVERSION_WIN6=1 -DTARGET_WIN64=1

resarmce.o: main.rc main.h win4.ico
	$(MINGWARMRC) $< -o $@ -DTARGET_WINCE=1

resx86ce.o: main.rc main.h win4.ico
	$(MINGW386RC) $< -o $@ -DTARGET_WINCE=1

# Windows XP (GNU)
out/gnu5x.exe: start32w.o win5x.o res5x.o
	mkdir -p `dirname $@`
	$(MINGW32LD) -m i386pe -estart $(MINGW32CRT) $^ -o $@ $(MINGW32LDFLAGS) -lmingw32 -lgcc -lgcc_eh -lmingwex -lmsvcrt -lkernel32 -luser32 -lcomctl32 -lgdi32 --subsystem=windows

# Windows 7 (GNU, 32-bit)
out/win6x32.exe: start32w.o win6x32.o res6x32.o
	mkdir -p `dirname $@`
	$(MINGW32LD) -m i386pe -estart $(MINGW32CRT) $^ -o $@ $(MINGW32LDFLAGS) -lmingw32 -lgcc -lgcc_eh -lmingwex -lmsvcrt -lkernel32 -luser32 -lcomctl32 -lgdi32 --subsystem=windows

# Windows 7 (64-bit)
out/win6x.exe: start64.o win6x.o res6x.o
	mkdir -p `dirname $@`
	$(MINGW64LD) -m i386pep -estart $(MINGW64CRT) $^ -o $@ $(MINGW64LDFLAGS) -lmingw32 -lgcc -lgcc_eh -lmingwex -lmsvcrt -luser32 -lkernel32 -lcomctl32 -lgdi32 --subsystem=windows

out/armce.exe: armce.o resarmce.o
	mkdir -p `dirname $@`
	$(MINGWARMCC) $^ -o $@ -laygshell

out/x86ce.exe: x86ce.o resx86ce.o
	mkdir -p `dirname $@`
	$(MINGW386CC) $^ -o $@ -laygshell

win1x.res: main.rc win1.ico
	$(WIN1RC) -r $<
	mv `basename $< .rc | tr '[:lower:]' '[:upper:]'`.RES $@

win2x.res: main.rc win1.ico
	$(WIN2RC) -r $<
	mv `basename $< .rc | tr '[:lower:]' '[:upper:]'`.RES $@

# Windows 1.0 support
out/win1x.exe: win1x.res win1x.exe
	$(WRC) -q $^
	python3 fixver -fix -os 2 -ver 2.0 -mark MEMORY $(word 2,$^)
	mkdir -p `dirname $@`
	cp $(word 2,$^) $@
	chmod +x $@

# Windows 2.0 support
out/win2x.exe: win2x.res win2x.exe
	$(WRC) -q $^
	python3 fixver -fix -os 2 -ver 2.1 -mark MEMORY $(word 2,$^)
	mkdir -p `dirname $@`
	cp $(word 2,$^) $@
	chmod +x $@

# Windows 3.0 support
out/win3x.exe: main.rc win3x.exe win3.ico
	$(WRC) -q -bt=windows main.rc win3x.exe -30 -DVERSION_WIN3=1
	mv $(patsubst %.rc,%.res,$<) $(patsubst %.exe,%.res,$(word 2,$^))
	mkdir -p `dirname $@`
	cp $(word 2,$^) $@
	chmod +x $@

# Windows 95 support
out/win4x.exe: main.rc win4x.exe win4.ico
	$(WRC) -q -bt=nt      main.rc win4x.exe     -DVERSION_WIN4=1 -DTARGET_WIN32=1
	mv $(patsubst %.rc,%.res,$<) $(patsubst %.exe,%.res,$(word 2,$^))
	mkdir -p `dirname $@`
	cp $(word 2,$^) $@
	chmod +x $@

# Windows XP support
out/win5x.exe: main.rc win5x.exe win4.ico
	$(WRC) -q -bt=nt      main.rc win5x.exe     -DVERSION_WIN5=1 -DTARGET_WIN32=1
	mv $(patsubst %.rc,%.res,$<) $(patsubst %.exe,%.res,$(word 2,$^))
	mkdir -p `dirname $@`
	cp $(word 2,$^) $@
	chmod +x $@

# Testing out the 0.0 version stamp: confuses 3.0 and later, XP refuses to launch it
out/win1x0.exe: out/win1x.exe
	cp $< $@
	python3 fixver -fix -os 0 -ver 0.0 $@

# Testing out the 1.0 version stamp: confuses 3.1 and later, XP refuses to launch it
out/win1x1.exe: out/win1x.exe
	cp $< $@
	python3 fixver -fix -os 0 -ver 1.0 $@

# Windows NT 4.0 support
out/win4xw.exe: main.rc win4xw.exe win4.ico
	$(WRC) -q -bt=nt      main.rc win4xw.exe    -DVERSION_WIN4=1
	mv $(patsubst %.rc,%.res,$<) $(patsubst %.exe,%.res,$(word 2,$^))
	mkdir -p `dirname $@`
	cp $(word 2,$^) $@
	chmod +x $@

win1.ico:
	python3 genicon.py

win3.ico:
	python3 genicon.py

win4.ico:
	python3 genicon.py

.PHONY: all force clean distclean

