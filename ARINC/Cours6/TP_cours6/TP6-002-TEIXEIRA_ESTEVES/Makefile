#===============================================================
# FOLDERS
#===============================================================
# Base directory for building. It is used as basis for
# include files search. It's the current directory where 
# make is called.
INCLUDEDIR = $(shell pwd)
export INCLUDEDIR
# All output files (kernel.img and partitions ELF files)
# are placed in this folder. Along with the configuration of
# the OS, they provide all that's needed to the RPi to
# execute.
OUTPUTDIR = $(INCLUDEDIR)/output
export OUTPUTDIR
# The kernel sources are placed in this directory
KERNELDIR = $(INCLUDEDIR)/kernel

#===============================================================
# BUILD ENVIRONMENT
#===============================================================
ARMGNU  = arm-none-eabi
# build tools
GCC     = $(ARMGNU)-gcc
AS      = $(ARMGNU)-gcc
LD      = $(ARMGNU)-ld
AR      = $(ARMGNU)-ar
OBJCOPY = $(ARMGNU)-objcopy
RM      = rm -f
ECHO    = echo
export GCC AS LD AR OBJCOPY RM ECHO

# Build flags
DEPENDFLAGS := -MD -MP
INCLUDES    := -nostdinc -I$(INCLUDEDIR) $(EXTRAINCLUDES)
BASEFLAGS   := -O0 -fpic -nostdlib -nostartfiles
#BASEFLAGS   := -pedantic -pedantic-errors 
BASEFLAGS   += -ffreestanding -fomit-frame-pointer 
BASEFLAGS   += -mcpu=arm1176jzf-s
WARNFLAGS   := -Wall -Wextra -Wshadow
#WARNFLAGS   := -Werror
WARNFLAGS   += -Wwrite-strings
WARNFLAGS   += -Wredundant-decls -Winline
WARNFLAGS   += -Wno-attributes -Wno-deprecated-declarations
WARNFLAGS   += -Wno-div-by-zero -Wno-endif-labels -Wfloat-equal
WARNFLAGS   += -Wformat=2 -Wno-format-extra-args -Winit-self
WARNFLAGS   += -Winvalid-pch -Wmissing-format-attribute
WARNFLAGS   += -Wmissing-include-dirs -Wno-multichar
WARNFLAGS   += -Wredundant-decls -Wshadow
WARNFLAGS   += -Wno-sign-compare -Wswitch -Wsystem-headers -Wundef
WARNFLAGS   += -Wno-pragmas -Wno-unused-but-set-parameter
WARNFLAGS   += -Wno-unused-but-set-variable -Wno-unused-result
WARNFLAGS   += -Wwrite-strings -Wdisabled-optimization -Wpointer-arith
# The following line is commented because I do "unsafe" conversions
# in function bzero.
#WARNFLAGS   += -Wcast-align
# It's not possible with the following line, because I can no longer
# perform casts from char* to int*
#WARNFLAGS   += -Werror 
ASFLAGS     := $(INCLUDES) $(DEPENDFLAGS) $(BASEFLAGS) -D__ASSEMBLY__
CFLAGS      := $(INCLUDES) $(DEPENDFLAGS) $(BASEFLAGS) $(WARNFLAGS)
CFLAGS      += -std=gnu99
export ASFLAGS CFLAGS

#===============================================================
# LIBRARIES, which have a general compilation script
#===============================================================
KERNEL_LIBRARIES   := \
	libsdfs/libsdfs.a \
	librpi/librpi.a \
	libc/libc.a \

PARTITION_LIBRARIES := \
	librpi/librpi.a \
	arinc653/lib653.a \
	libpartition/libpartition.a \
	libc/libc.a \
	librpi/librpi.a \


# I also need to export this for the compilation of the
# partitions
export PARTITION_LIBRARIES

#===============================================================
# KERNEL-SPECIFIC STUFF
#===============================================================
# Kernel sub-folder compilation produces a single object file
KERNELOBJ       := $(KERNELDIR)/kernel.o
# Once kernel.o and libraries are compiled, the are linked using
# this LD script.
KERNEL_LDSCRIPT := $(KERNELDIR)/kernel_rpi.x


#===============================================================
# BUILD RULES
#===============================================================
all: kernel

kernel: $(OUTPUTDIR)/kernel.img \
	$(OUTPUTDIR)/mypart.elf \


$(OUTPUTDIR)/kernel.img: $(KERNELDIR)/kernel.elf
	$(OBJCOPY) $(KERNELDIR)/kernel.elf -O binary $(OUTPUTDIR)/kernel.img

$(KERNELDIR)/kernel.elf: $(KERNELOBJ) $(KERNEL_LDSCRIPT) $(KERNEL_LIBRARIES)
	$(LD) $(KERNELOBJ) $(KERNEL_LIBRARIES) -T$(KERNEL_LDSCRIPT) -o $@

$(KERNELOBJ) $(KERNEL_LDSCRIPT): force_look
	$(ECHO) looking into kernel
	cd kernel ; make

$(OUTPUTDIR)/mypart.elf: force_look $(PARTITION_LIBRARIES)
	$(ECHO) looking into partition folder mypart
	cd mypart ; make

librpi/librpi.a: force_look
	$(ECHO) looking into librpi
	cd librpi ; make

libc/libc.a: force_look
	$(ECHO) looking into libc
	cd libc ; make

libsdfs/libsdfs.a: force_look
	$(ECHO) looking into libsdfs
	cd libsdfs ; make

arinc653/lib653.a: force_look
	$(ECHO) looking into arinc653
	cd arinc653 ; make

libpartition/libpartition.a: force_look
	$(ECHO) looking into libpartition
	cd libpartition ; make

clean:
	$(RM) $(KERNELDIR)/kernel.elf $(OUTPUTDIR)/*.img $(OUTPUTDIR)/*.elf
	cd kernel ; make clean
	cd mypart ; make clean
	cd librpi ; make clean 
	cd libc ; make clean 
	cd libsdfs ; make clean
	cd arinc653 ; make clean
	cd libpartition ; make clean

distclean: clean
	$(RM) *.d
	cd kernel ; make distclean
	cd mypart ; make distclean
	cd librpi ; make distclean 
	cd libc ; make distclean 
	cd libsdfs ; make distclean 
	cd arinc653 ; make distclean
	cd libpartition ; make distclean

force_look: 
	true
