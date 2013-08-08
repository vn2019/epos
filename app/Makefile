CROSS=

CC=		$(CROSS)gcc
AS=		$(CROSS)as
LD=		$(CROSS)ld
OBJCOPY=	$(CROSS)objcopy
AR=		$(CROSS)ar
STRIP=		$(CROSS)strip
SIZE=		$(CROSS)size
STRINGS=	$(CROSS)strings
READELF=	$(CROSS)readelf
RANLIB=		$(CROSS)ranlib
NM=		$(CROSS)nm

PROG=	a

all: $(PROG).exe

CFLAGS=		-fno-builtin -ffreestanding -O #-Wall
LDFLAGS=	-nostdlib -nostartfiles -Wl,-Map,$(PROG).map

OBJS=		crt0.o utils.o vsprintf.o main.o

$(PROG).exe: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
	../../bin/winimage.exe /h ../image.flp  /i $@

vsprintf.o: ../vsprintf.c
	$(CC) -c $(CFLAGS) $^

utils.o: ../utils.c
	$(CC) -c $(CFLAGS) $^

clean:
	-$(RM)  *.o *.out *.bin *.*~ $(PROG).exe $(PROG).map