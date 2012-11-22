TARGET= ecslave
OBJS  = \
../stack/ec_cmd_apwr.o \
../stack/ec_cmd_aprd.o \
../stack/ec_cmd_aprw.o \
../stack/ec_cmd_armw.o \
../stack/ec_cmd_brd.o \
../stack/ec_cmd_brw.o \
../stack/ec_cmd_bwr.o \
../stack/ec_cmd_fprd.o \
../stack/ec_cmd_fprw.o \
../stack/ec_cmd_fpwr.o \
../stack/ec_cmd_frmw.o \
../stack/ec_cmd_lrw.o \
../stack/ec_cmd_lrd.o \
../stack/ec_cmd_lwr.o \
../stack/ec_sii.o \
../stack/ec_regs.o \
../stack/ec_coe.o \
../stack/ec_mbox.o \
../stack/ec_process_data.o

CURRENT=`uname -r`
KDIR = /lib/modules/$(CURRENT)/build/
PWD = $(shell pwd) 

obj-m      := $(TARGET).o
$(TARGET)-objs   := $(OBJS)

default:
	make -C $(KDIR) -I../include SUBDIRS=$(PWD) modules

$(TARGET).o: $(OBJS)
	$(LD) $(LD_RFLAG) -r -o $@ $(OBJS)
clean:
	-rm -f *.o .*.o.cmd .*.ko.cmd *.[ch]~ Modules.symvers *.order *.*.ko *.ko *.mod.c .*.c  Module.symvers
indent:
	indent -kr -i8 *.[ch]
