#MACRO DEFINITIONS FOR MUTATOR BUILD

CXX=clang++

SRCS=./mutator_aux.cpp ./mutator-lvl1.cpp ./mutator-lvl0.cpp ./mutator-lvl2.cpp ./mutator-lvl0.h ./mutator_aux.h ./daemon/mutatord.h ./daemon/mutatorclient.c ./daemon/mutatorclient.h ./daemon/daemon_aux.h ./daemon/daemon_aux.c ./daemon/mutatord.c ./daemon/mutatorserver.c ./daemon/mutatorserver.h

CTAGS=ctags --c++-kinds=+p --fields=+iaS --extra=+q 
