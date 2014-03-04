#TOOLCHAIN=/usr/local/arm-linux/bin/
#CROSS_COMPILE=arm-linux-
#CC=$(TOOLCHAIN)$(CROSS_COMPILE)gcc
CC = gcc
#LIB=/usr/lib
CCFLAGS=-v -O3  \
	-DUNIX -D__UNIX__ \
#	-I/usr/local/arm-linux/arm-linux/include
LINKFLAGS=

TARGET=cs_demo

OBJ = udp.o debug.o dio.o dproc.o

all:	$(TARGET).o $(OBJ)
	$(CC) $(LINKFLAGS) $(TARGET).o $(OBJ) -o $(TARGET)

$(TARGET).o:$(TARGET).c
	$(CC) $(CCFLAGS) $(TARGET).c -c -o $(TARGET).o

udp.o: udp.c
	$(CC) $(CCFLAGS) udp.c -c -o udp.o

debug.o: debug.c
	$(CC) $(CCFLAGS) debug.c -c -o debug.o

dio.o: dio.c
	$(CC) $(CCFLAGS) dio.c -c -o dio.o

dproc.o: dproc.c
	$(CC) $(CCFLAGS) dproc.c -c -o dproc.o

clean:
	\rm -f *.o $(TARGET) *~

