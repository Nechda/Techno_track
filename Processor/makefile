TARGET = ProgramManager.out
SRC = main.cpp Tester.cpp  Logger.cpp CPU.cpp Asm.cpp Stack/Stack.cpp
BIN = main.o Tester.o  Logger.o CPU.o Asm.o Stack/Stack.o

.PHONY: all clean

all: $(TARGET)

clean:
	rm -rf *.o

*.o: *.cpp
	g++ -c -o $@ $<
$(TARGET): $(BIN)
	g++ -o $(TARGET) $(BIN)
