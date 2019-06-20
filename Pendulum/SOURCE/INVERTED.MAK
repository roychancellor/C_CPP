PROJ	=INVERTED
DEBUG	=1
CC	=qcl
AS	=qcl
CFLAGS_G	= /AH /W4 /Ze /FPi87 
CFLAGS_D	= /Zi /Zr /Gi$(PROJ).mdt /Od 
CFLAGS_R	= /O /Ot /DNDEBUG 
CFLAGS	=$(CFLAGS_G) $(CFLAGS_D)
AFLAGS_G	=/Cx /P1
AFLAGS_D	=/Zi
AFLAGS_R	=/DNDEBUG
AFLAGS	=$(AFLAGS_G) $(AFLAGS_D)
LFLAGS_G	= /CP:0xfff /NOI /SE:0x80 /ST:0x2710 /NOE 
LFLAGS_D	= /CO /M /INCR 
LFLAGS_R	= 
LFLAGS	=$(LFLAGS_G) $(LFLAGS_D)
RUNFLAGS	=
OBJS_EXT = 	
LIBS_EXT = 	..\..\..\royware\pendulum\lib\pendsim.lib ..\..\..\lang\qc25\lib\royware.lib 

.asm.obj: ; $(AS) $(AFLAGS) -c $*.asm

all:	$(PROJ).EXE

inverted.obj:	inverted.c $(H)

$(PROJ).EXE:	inverted.obj $(OBJS_EXT)
	echo >NUL @<<$(PROJ).crf
inverted.obj +
$(OBJS_EXT)
$(PROJ).EXE

$(LIBS_EXT);
<<
	ilink -a -e "qlink $(LFLAGS) @$(PROJ).crf" $(PROJ)

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)

