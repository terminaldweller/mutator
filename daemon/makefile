
##################################VARS#################################
CC=clang
CC_FLAGS=
LD_FLAGS=
TARGETD=mutatord
TARGETS=mutatorserver
TARGETC=mutatorclient
##################################RULES################################
.DEFAULT:all

.PHONY:all clean help $(TARGETC) $(TARGETS) $(TARGETD)

all:$(TARGETC) $(TARGETS) $(TARGETD)

.c.o:
	$(CC) $(CC_FLAGS) -c $< -o $@

$(TARGETC): $(TARGETC).o
	$(CC) $^ $(LD_FLAGS) -o $@

$(TARGETS): $(TARGETS).o daemon_aux.o
	$(CC) $^ $(LD_FLAGS) -o $@

$(TARGETD): $(TARGETD).o daemon_aux.o
	$(CC) $^ $(LD_FLAGS) -o $@

clean:
	rm -f *.o *~ $(TARGETD) $(TARGETS) $(TARGETC)

help:
	@echo 'all builds the daemon, the server and the client. all is the default.'
	@echo 'mutatord builds the daemon with the server'
	@echo 'mutatorc builds the client'
	@echo 'mutators builds the standalone server'
	@echo 'clean runs clean.'
	@echo 'help runs help.'

