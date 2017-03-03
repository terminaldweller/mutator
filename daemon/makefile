
##################################VARS#################################
CC=clang
CC_FLAGS=
LD_FLAGS=
TARGETD=mutatord
TARGETS=mutatorserver
TARGETC=mutatorclient
##################################RULES################################
.DEFAULT: all

.PHONY: all clean help

all: $(TARGETD) $(TARGETC) $(TARGETS)

.c.o:
	$(CC) $(CC_FLAGS) -c $< -o $@

TARGETD:
	$(CC) $^ $(LD_FLAGS) -o $@

TARGETS:
	$(CC) $^ $(LD_FLAGS) -o $@

TARGETC:
	$(CC) $^ $(LD_FLAGS) -o $@

clean:
	rm -f *.o *~ $(TARGETD) $(TARGETS) $(TARGETC)

help:
	@echo 'all builds the daemon, the server and the client. all is the default.'
	@echo 'mutatord build the daemon'
	@echo 'mutatorc builds the client'
	@echo 'mutators builds the client'
	@echo 'clean runs clean.'
	@echo 'help runs help.'

