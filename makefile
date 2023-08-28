#
# Project: USBAudio2
#

CC = SDK:gcc/bin/gcc

BIN = USBAudio2
LOWBIN = usbaudio2
REVISION := `type $(BIN)rev.rev`

OBJ = \
	usbaudio2.o      \
	actions.o         \
	midisupport.o     \
	myargs.o          \
	oca.o             \
	projectrev.o      \
	storestring.o     \
	vumetersupport.o  \
	wavesupport.o     \

VERSION = 53

INCPATH =

CFLAGS = $(INCPATH) -Wall -Wwrite-strings -gstabs

LDFLAGS =

HDRS =  

LIBS =
#	add any extra linker libraries you want here

.PHONY: normal touch all all-before all-after clean clean-custom revision

normal: all-before $(BIN) all-after

touch:
	@rm -f ?#?.o

all: touch all-before $(BIN) all-after

all-before:
#	You can add rules here to execute before the project is built
	@bumprev -r $(REVISION) -i h $(VERSION) $(BIN)

all-after:
#	You can add rules here to execute after the project is built
	@list $(BIN) nohead

clean: clean-custom
	rm $(OBJ) $(BIN) $(BIN).debug

revision:
	bumprev -i h $(VERSION) $(BIN)

$(BIN): $(OBJ)
#	Debug builds require the -g or -gstabs option in CFLAGS
#	You may also need to move the LDFLAGS variable depending on the contents
	$(CC) -N -o $(BIN).debug $(OBJ) $(LDFLAGS) $(LIBS)
	@strip $(BIN).debug -o $(BIN)

projectrev.o:projectrev.c $(BIN)_rev.h
	$(CC) $(CFLAGS) -c -o $@ $<

%.o : %.c  makefile
	$(CC) $(CFLAGS) -c -o $@ $<

