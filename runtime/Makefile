CC := clang++
OPT_CC := -fomit-frame-pointer
OBJS = allocator.o 

all: $(OBJS) statepoints

statepoints: 
	clang -c includes/statepoint.c

clean: 
	rm -f $(OBJS)
