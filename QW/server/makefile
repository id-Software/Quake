
#CFLAGS = -g -Wall -DDEBUG -I../client -I. -DSERVERONLY
#CFLAGS = -mpentium -O6 -Wall -I../client -I. -DSERVERONLY -fomit-frame-pointer -fno-strength-reduce
#CFLAGS = -mpentium -O2 -Wall -I../client -I. -DSERVERONLY -fomit-frame-pointer -fno-strength-reduce
CFLAGS=-DSERVERONLY -I../client -V2.7.2.1 -bi486-linux/ -O6 -Wall -fomit-frame-pointer -fno-strength-reduce

EXE = qwsv

OFILES =\
         pr_cmds.o \
		 pr_edict.o \
		 pr_exec.o \
		 sv_init.o \
		 sv_main.o \
		 sv_ents.o \
		 sv_send.o \
		 sv_move.o \
		 sv_phys.o \
		 sv_user.o \
		 sv_ccmds.o \
		 world.o \
         sys_unix.o \
		 model.o \
		 cmd.o \
		 common.o \
		 crc.o \
		 cvar.o \
		 mathlib.o \
		 zone.o \
		 pmove.o \
		 pmovetst.o \
		 net_chan.o \
		 net_udp.o 

LDFLAGS = -lm

$(EXE) : $(OFILES)
	cc $(CFLAGS) -o $(EXE) $(OFILES) $(LDFLAGS) 

clean:
	rm -f $(OFILES) $(EXE)

app:
	make "CFLAGS = -O4 -g -Wall  -I../client  -DSERVERONLY"

profile:
	make "CFLAGS = -g -pg -O -Wall -I../client -DPROFILE"
	cp $(EXE) /LocalApps

.c.o: ; cc -c $(CFLAGS) -o $@ $*.c
.s.o: ; cc -c $(CFLAGS) -o $@ $*.s

