CC = gcc
TARGET = send1
OBJECTS = $(patsubst %.cpp,%.o, $(wildcard *.cpp))

$(TARGET): $(OBJECTS)
	@echo "** Linking Executable"
	mipsel-openwrt-linux-uclibc-g++  $(OBJECTS) -o $(TARGET)

clean:
	@rm -f *.o *~

veryclean: clean
	@rm -f $(TARGET)

%.o: %.cpp
	@echo "** Compiling C Source" 
	$(CC) -c $<
