TOOLCHAIN=/usr/local/arm-linux/bin/
CROSS_COMPILE=arm-linux-
#CC=$(TOOLCHAIN)$(CROSS_COMPILE)gcc
CC = g++
#LIB=/usr/lib
CCFLAGS=-v -O3  \
	-DUNIX -D__UNIX__ \
	-DDEBUG \
#	-I/usr/local/arm-linux/arm-linux/include \
LINKFLAGS=

TARGET=mn

OBJ = 

all:	$(TARGET).o $(OBJ)
	$(CC) $(LINKFLAGS) $(TARGET).o $(OBJ) -o $(TARGET)

$(TARGET).o:$(TARGET).cpp
	$(CC) $(CCFLAGS) $(TARGET).cpp -c -o $(TARGET).o

clean:
	\rm -f *.o $(TARGET) *~

