CC=clang
CC?=clang
CC_FLAGS=
CC_EXTRA?=
CC_FLAGS+=$(CC_EXTRA)
SRCS=$(wildcard *.c)
TBG_OBJLIST=$(patsubst %.c, %.o , $(wildcard *.c))

.DEFAULT:all

.PHONY:all clean help

all:$(TBG_OBJLIST)
	@echo $(TBG_OBJLIST)
	@echo $(TBG_OBJLIST_INC)

depend:.depend

.depend:$(SRCS)
	rm -rf .depend
	$(CC) -MM $(CC_FLAGS) $^ > ./.depend

-include ./.depend

.c.o:
	$(CC) $(CC_FLAGS) -c $< -o $@ 

clean:
	rm -f *.o *~ $(TARGET)
	rm .depend

help:
	@echo "all is the default target"
	@echo "there is clean."
