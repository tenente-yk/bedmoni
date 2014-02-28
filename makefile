include makefile.rules

CC := gcc
CFLAGS := -O3 -O -Wall
SFLAGS :=
LFLAGS :=
DEFS := -DUNIX \

PROJDIR = /home/tenente/my_projects/bedmoni
ROOTFSDIR = /home/tenente/pxa270/rootfs
OBJDIR = obj
TARGET = bedmoni
XMLDIR = xml
LANGDIR = lng
MNDIR = menus/res
DATADIR=bedmoni_data

LINKFLAGS := -lm -lpthread -Wl,-Map,$(TARGET).map
POSTFIX_LINKFLAGS :=

ifeq ($(OS), UBUNTU)
LINKFLAGS += -pthread
POSTFIX_LINKFLAGS += -lm
endif

ifneq ($(ARCH), ARM)
CFLAGS += -g
LINKFLAGS += -g
endif

source_dirs := modules
source_dirs += .
source_dirs += tasks
source_dirs += menus/res

ifeq ($(ARCH), ARM)
TOOLCHAIN_PATH=/usr/local/arm-linux/bin
CC := $(TOOLCHAIN_PATH)/arm-linux-gcc
#TOOLCHAIN_PATH=/opt/OSELAS.Toolchain-1.99.3/arm-xscale-linux-gnueabi/gcc-4.3.2-glibc-2.8-binutils-2.18-kernel-2.6.27-sanitized/bin
#CC := $(TOOLCHAIN_PATH)/arm-xscale-linux-gnueabi-gcc
DEFS += -DARM
CFLAGS += -O3 -mstructure-size-boundary=8
endif
source_dirs += menus
source_dirs += res

#search_wildcard  s := $(addsuffix /*.c,$(source_dirs))

ifeq ($(ARCH), ARM)
all: build_arm install
else
all: build
endif

# Generate documentation by using Doxygen
doc:
	doxygen Doxyfile

.PHONY: doc

xmlout:
#	$(XMLDIR)/xml2h -f $(XMLDIR)/csio_typedefs.xml -o ./modules/csio_typedefs.h > /dev/null

.PHONY: xmlout

menus:
	$(MNDIR)/mn `find . -name '*.mn' -print` > /dev/null

.PHONY: menus

lang:
	$(LANGDIR)/lng $(LANGDIR)/english.lng > /dev/null
	$(LANGDIR)/lng $(LANGDIR)/russian.lng > /dev/null
	$(LANGDIR)/lng $(LANGDIR)/french.lng > /dev/null
	$(LANGDIR)/lng $(LANGDIR)/german.lng > /dev/null
	$(LANGDIR)/lng $(LANGDIR)/spanish.lng > /dev/null
	$(LANGDIR)/lng $(LANGDIR)/ukrainian.lng > /dev/null
	mv $(LANGDIR)/*.lang $(DATADIR)

.PHONY: lang

install:
ifeq ($(OS), UBUNTU)
	chmod a+x ./bedmoni
endif
	sudo cp ./bedmoni $(ROOTFSDIR)/rootfs_nfs/usr/local/bin/ -v
	sudo cp ./bedmoni $(ROOTFSDIR)/rootfs_ext2/usr/local/bin/ -v
	sudo cp ./bedmoni_data/*.lang $(ROOTFSDIR)/rootfs_nfs/usr/local/bedmoni_data/ -v
	sudo cp ./bedmoni_data/*.lang $(ROOTFSDIR)/rootfs_ext2/usr/local/bedmoni_data/ -v
	sudo mkdir $(ROOTFSDIR)/rootfs_ext2/usr/local/fonts -p
	sudo cp ./fonts/* $(ROOTFSDIR)/rootfs_ext2/usr/local/fonts -r
	sudo mkdir $(ROOTFSDIR)/rootfs_nfs/usr/local/fonts -p
	sudo cp ./fonts/* $(ROOTFSDIR)/rootfs_nfs/usr/local/fonts -r
	mkdir /home/tenente/my_projects/bmoni_updater/bedmoni_data -p
	cp ./bedmoni_data/*.lang /home/tenente/my_projects/bmoni_updater/bedmoni_data -v
	mkdir /home/tenente/my_projects/bmoni_updater/bin -p
	cp ./bedmoni /home/tenente/my_projects/bmoni_updater/bin -v
	cd /home/tenente/my_projects/bmoni_updater
#	./make_arc.sh


build: xmlout menus lang $(TARGET)

build_arm: xmlout menus lang $(TARGET)

objects := $(patsubst %.c,%.o,$(wildcard *.c))
objects += $(patsubst %.c,%.o,$(wildcard modules/*.c))
objects += $(patsubst %.c,%.o,$(wildcard menus/*.c))
objects += $(patsubst %.c,%.o,$(wildcard tasks/*.c))
objects += menulst.o

#$(TARGET): $(notdir $(patsubst %.c,%.o,$(wildcard  $(search_wildcard  s)))) menulst.o
$(TARGET): $(notdir $(objects))
	$(CC) $(LINKFLAGS) $^ -o $@ $(POSTFIX_LINKFLAGS)
	mkdir -p $(OBJDIR)
	mv -f *.o $(OBJDIR)

VPATH := $(source_dirs)

%.o: %.c
	$(CC) $(DEFS) $(CFLAGS) -c $(addprefix -I ,$(source_dirs)) $<

%.o: %.s
	$(CC) $(DEFS) $(SFLAGS) -c $(addprefix -I ,$(source_dirs)) $<


clean:
	find . -type f \
		\( -name '$(TARGET)' -o -name '*.bak' -o -name '*~' \
		-o -name '*.map' -o -name '*.men' \
		-o -name '*.o'  -o -name '*.a' -o -name '*.d' \) -print \
		| xargs rm -f
	rm -R $(OBJDIR)
	rm $(MNDIR)/*.men -f
	rm $(MNDIR)/*~ -f
	rm $(DATADIR)/*.lang

run:
	./$(TARGET)

#include $(wildcard  *.d)