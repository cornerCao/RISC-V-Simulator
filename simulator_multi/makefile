objects = Simulation.o Read_Elf.o
GCC = g++

simulator: $(objects)
	$(GCC) -o simulator $(objects) -lm

simulation.o: Simulation.cpp Simulation.h
	$(GCC) -c Simulation.cpp

read_elf.o: Read_Elf.cpp Read_Elf.h
	$(GCC) -c Read_Elf.cpp

.PHONY : clean
clean :
	-rm simulator $(objects)
