CC = gcc
BC = mipsel-openwrt-linux-uclibc-g++
TARGET = send
OBJECTS = $(patsubst %.cpp,%.o, $(wildcard *.cpp))

$(TARGET): $(OBJECTS)
	@echo "** Linking Executable"
	$(BC) $(OBJECTS) -o $(TARGET) 

clean:
	@rm -f *.o *~

veryclean: clean
	@rm -f $(TARGET)

%.o: %.cpp
	@echo "** Compiling C Source" 
	$(CC) -c $<
