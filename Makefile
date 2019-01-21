all: compile 

OBJS = parser.o  \
       codegen.o \
       main.o    \
       tokens.o  \
       corefn.o  \
	   native.o  \

LLVMCONFIG = llvm-config
CPPFLAGS = `$(LLVMCONFIG) --cppflags` -std=c++11 -g
LDFLAGS = `$(LLVMCONFIG) --ldflags` -lpthread -ldl -lz -lncurses -rdynamic
LIBS = `$(LLVMCONFIG) --libs`

clean:
	$(RM) -rf parser.cpp parser.hpp parser tokens.cpp $(OBJS)
	cd runtime && make clean
	$(RM) -f build/*

parser.cpp: parser.y
	bison -d -o $@ $^
	
parser.hpp: parser.cpp

tokens.cpp: tokens.l parser.hpp
	flex -o $@ $^

%.o: %.cpp
	clang++ -c $(CPPFLAGS) -o $@ $<

parser: $(OBJS)
	clang++ -o parser  $(OBJS) $(LIBS) $(LDFLAGS)

gc:
	+$(MAKE) -C runtime

compile: parser gc example.txt
	./parser example.txt 2>&1 | sed -u 1,/BEGIN_IR/d > build/out.ll
	# Perform statepoint relocation opt pass
	opt -rewrite-statepoints-for-gc -S -o build/out-sp.ll build/out.ll
	# # Assemble IR
	llc -o build/out-sp.s build/out-sp.ll
	clang++ -c -o build/out-sp.o build/out-sp.s
	objcopy --globalize-symbol=__LLVM_StackMaps build/out-sp.o build/out-sp-globalised.o
	clang++ -o build/a.out build/out-sp-globalised.o runtime/allocator.o runtime/statepoint.o
