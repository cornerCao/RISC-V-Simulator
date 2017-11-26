objects = Simulation.o Read_Elf.o cache.o memory.o
GCC = g++

simulator: $(objects)
        $(GCC) -o simulator $(objects) -lm

simulation.o: Simulation.cpp Simulation.h
        $(GCC) -c Simulation.cpp

read_elf.o: Read_Elf.cpp Read_Elf.h
        $(GCC) -c Read_Elf.cpp
cache.o: cache.cc cache.h def.h
        $(GCC) -c cache.cc
memory.o: memory.cc memory.h
        $(GCC) -c memory.cc

.PHONY : clean
clean :
        -rm simulator $(objects)
