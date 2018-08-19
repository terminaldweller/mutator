CC=clang
CC?=clang
CC_FLAGS=-fpic
CC_EXTRA?=
CC_FLAGS+=$(CC_EXTRA)
SRCS=$(wildcard *.c)
TBG_OBJLIST=$(patsubst %.c, %.o , $(wildcard *.c))
TBG_OBJLIST_DBG=$(patsubst %.c, %.odbg , $(wildcard *.c))
TBG_OBJLIST_COV=$(patsubst %.c, %.ocov , $(wildcard *.c))
ADD_SANITIZERS_CC= -g -fsanitize=address -fno-omit-frame-pointer
ADD_SANITIZERS_LD= -g -fsanitize=address
MEM_SANITIZERS_CC= -g -fsanitize=memory -fno-omit-frame-pointer
MEM_SANITIZERS_LD= -g -fsanitize=memory
UB_SANITIZERS_CC= -g -fsanitize=undefined -fno-omit-frame-pointer
UB_SANITIZERS_LD= -g -fsanitize=undefined
COV_CC= -fprofile-instr-generate -fcoverage-mapping

.DEFAULT:all

.PHONY:all clean help

all:$(TBG_OBJLIST) $(TBG_OBJLIST_DBG) $(TBG_OBJLIST_COV)
	@echo $(TBG_OBJLIST)
	@echo $(TBG_OBJLIST_INC)

depend:.depend

.depend:$(SRCS)
	rm -rf .depend
	$(CC) -MM $(CC_FLAGS) $^ > ./.depend
	echo $(patsubst %.o:, %.odbg:, $(shell $(CC) -MM $(CC_FLAGS) $^)) | sed -r 's/[A-Za-z0-9\-\_]+\.odbg/\n&/g' >> ./.depend
	echo $(patsubst %.o:, %.ocov:, $(shell $(CC) -MM $(CC_FLAGS) $^)) | sed -r 's/[A-Za-z0-9\-\_]+\.ocov/\n&/g' >> ./.depend

-include ./.depend

.c.o:
	$(CC) $(CC_FLAGS) -c $< -o $@

%.odbg:%.c
	$(CC) $(CC_FLAGS) -g -c $< -o $@

%.ocov:%.c
	$(CC) $(CC_FLAGS) $(COV_CC) -c $< -o $@

clean:
	rm -f *.o *~ $(TARGET) *.ocov *.odbg
	rm .depend

help:
	@echo "all is the default target"
	@echo "there is clean."
