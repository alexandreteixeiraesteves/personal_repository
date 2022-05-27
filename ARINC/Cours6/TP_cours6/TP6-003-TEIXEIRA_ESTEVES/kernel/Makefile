# source files
SOURCES_ASM := $(wildcard *.S)
SOURCES_C_OPT   := \

SOURCES_C_NONOPT := \
	boot-aux.c \
	init-kernel.c \
	init-load-partitions.c \
	init-memory.c \
	init-mmu.c \
	interrupt_handler_irq.c \
	interrupt_handler_svc.c \
	interrupt_handlers.c \
	kernel-debug.c \
	kernel-stdlib.c \
	queuing.c \
	scheduler-print-configuration.c \
	scheduler.c \
	system-partition-console.c \
	system-partition.c \



# object files
OBJS        := $(patsubst %.S,%.o,$(SOURCES_ASM))
OBJS        += $(patsubst %.c,%.opt.o,$(SOURCES_C_OPT))
OBJS        += $(patsubst %.c,%.nonopt.o,$(SOURCES_C_NONOPT))

# build rules
all: kernel.o kernel_rpi.x

# this directive will include the dependency files, which 
# are built for each file by the compiler when called 
# using -MD -MP. I'm not sure the approach is really correct,
# given that the changes seem to apply only to the next call to 
# make...
include $(wildcard *.d)

kernel_rpi.x: kernel_rpi.cmd ../librpi/mmap.h
	$(GCC) -E -P -traditional -undef -x assembler-with-cpp \
	                 -nostdinc -I$(INCLUDEDIR) kernel_rpi.cmd > $@

kernel.o: $(OBJS)
	$(LD) --relocatable $(OBJS) -o $@

clean:
	$(RM) $(OBJS) kernel.o kernel_rpi.x

distclean: clean
	$(RM) *.d

# C optimized.
%.opt.o: %.c Makefile
	$(GCC) $(CFLAGS) -c $< -o $@

# C nonoptimized.
%.nonopt.o: %.c Makefile
	$(GCC) $(CFLAGS) -c $< -o $@

# AS.
%.o: %.S Makefile
	$(AS) $(ASFLAGS) -c $< -o $@
