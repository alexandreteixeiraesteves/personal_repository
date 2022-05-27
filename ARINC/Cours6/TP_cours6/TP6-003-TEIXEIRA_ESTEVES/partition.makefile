# source files
SOURCES_ASM := $(wildcard *.S)
SOURCES_C   := $(wildcard *.c)
 
# object files
OBJS        := $(patsubst %.S,%.o,$(SOURCES_ASM))
OBJS        += $(patsubst %.c,%.o,$(SOURCES_C))

# libraries -- I need to change their paths
LOCAL_LIBS  := $(patsubst %,../%,$(PARTITION_LIBRARIES))

# build rules
all: $(TARGET)
 
include $(wildcard *.d)

rpi.x: ../libpartition/rpi.cmd	
	$(GCC) -E -P -traditional -undef -x assembler-with-cpp \
	                 -nostdinc -I$(INCLUDEDIR) ../libpartition/rpi.cmd >rpi.x

$(TARGET): $(OBJS) rpi.x $(LOCAL_LIBS)
	$(LD) $(OBJS) $(LOCAL_LIBS) $(LOCAL_LIBS) -Trpi.x -o $@

clean:
	$(RM) $(OBJS) $(TARGET) rpi.x
 
distclean: clean
	$(RM) *.d
 
# C.
%.o: %.c Makefile
	$(GCC) $(CFLAGS) -c $< -o $@
 
# AS.
%.o: %.S Makefile
	$(AS) $(ASFLAGS) -c $< -o $@
