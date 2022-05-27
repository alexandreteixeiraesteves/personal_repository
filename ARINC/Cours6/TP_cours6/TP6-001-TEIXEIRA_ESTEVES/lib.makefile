# source files
SOURCES_ASM := $(wildcard *.S)
SOURCES_C   := $(wildcard *.c)
 
# object files
OBJS        := $(patsubst %.S,%.o,$(SOURCES_ASM))
OBJS        += $(patsubst %.c,%.o,$(SOURCES_C))
 
# build rules
all: $(TARGET)
 
include $(wildcard *.d)
 
$(TARGET): $(OBJS)
	$(AR) rcs $@ $(OBJS)

clean:
	$(RM) $(OBJS) $(TARGET)
 
distclean: clean
	$(RM) *.d
 
# C.
%.o: %.c Makefile
	$(GCC) $(CFLAGS) $(EXTRAINCLUDES) -c $< -o $@
 
# AS.
%.o: %.S Makefile
	$(AS) $(ASFLAGS) -c $< -o $@
