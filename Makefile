CXXFLAGS=-Wall -O2 -std=c++11 -DNDEBUG 
ZLIB=/mnt/lustre/groups/ukbiobank/PRSice-cpp_development/PRSice.code/lib/zlib-1.2.11/build/libz.a 
CXX=/opt/apps/compilers/gcc/6.2.0/bin/g++
INCLUDES := -I inc/ -isystem lib/ -isystem lib/zlib-1.2.11/
THREAD := -Wl,--whole-archive -lpthread
SERVER := -L /usr/lib/x86_64-redhat-linux5E/lib64
GCC := -Wl,--no-whole-archive  -static-libstdc++ -static-libgcc -static
CSRC := src/*.c
CPPSRC := src/*.cpp
OBJ := bgen_lib.o binaryplink.o genotype.o misc.o plink_set.o prslice.o regression.o snp.o binarygen.o commander.o main.o plink_common.o prsice.o region.o SFMT.o

%.o: src/%.c
		$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

%.o: src/%.cpp
		$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

PRSice: $(OBJ)
		$(CXX) $(INCLUDES) $(SERVER)  $^ $(ZLIB) $(THREAD) $(GCC) -o $@
