TARGET= offsched
OBJS  = offsched_main.o

CURRENT=`uname -r`
KDIR = /lib/modules/$(CURRENT)/build/
PWD = $(shell pwd) 

obj-m      := $(TARGET).o
$(TARGET)-objs   := $(OBJS)

default:
	make -C $(KDIR) SUBDIRS=$(PWD) modules

$(TARGET).o: $(OBJS)
	$(LD) $(LD_RFLAG) -r -o $@ $(OBJS)
clean:
	-rm -fr *.o .*.o.cmd .*.ko.cmd *.[ch]~ Modules.symvers *.order *.*.ko *.ko *.mod.c .*.c  Module.symvers .tmp_versions
indent:
	indent -kr -i8 *.[ch]
