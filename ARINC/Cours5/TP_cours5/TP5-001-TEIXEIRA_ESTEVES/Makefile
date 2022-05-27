#===============================================================
# APPNAME
#===============================================================
# Heptagon part of the application
CSTNAME     = constants
CMPNAME     = complex
FFTNAME     = fft
APPNAME     = pitch
# Needed because I don't know how  to capitalize
CSTNAME_CAP = Constants
CMPNAME_CAP = Complex
FFTNAME_CAP = Fft
APPNAME_CAP = Pitch
# C part of the application
SNDIO       = sndio
FLOAT       = float
FFTC        = fftc


#===============================================================
# FOLDERS
#===============================================================
# Heptagon library C files location
LIBDIR    = ./
# Where the generated C code goes
CSTDIR    = ./$(CSTNAME)_c
CMPDIR    = ./$(CMPNAME)_c
FFTDIR    = ./$(FFTNAME)_c
APPDIR    = ./$(APPNAME)_c
# Where the external C code is expected to be
SNDIODIR  = ./$(SNDIO)_c
FLOATDIR  = ./$(FLOAT)_c
FFTCDIR   = ./$(FFTC)_c

#===============================================================
# BUILD ENVIRONMENT
#===============================================================
# Headers can be everywhere
GCC     = gcc -I$(LIBDIR) -I$(CSTDIR) -I$(CMPDIR) -I$(FFTDIR) -I$(APPDIR) -I$(SNDIODIR) -I$(FLOATDIR) -I$(FFTCDIR)
# Only for compiling main.c, I need some special 
# definitions to avoid rewriting the file every time.
MAIN_CFLAGS = -DHEADER="<"$(APPNAME)".h>" \
		-DTHEADER="<"$(APPNAME)"_types.h>" \
		-DMODNAME=$(APPNAME_CAP)
# gcc is better at loading than ld itself (it knows
# where to find the crt0 files, a.s.o.)
LD      = gcc -lm
HEPTC	= heptc

#===============================================================
# FILES
#===============================================================
CSTSRC      = ./$(CSTNAME).ept
CMPSRC      = ./$(CMPNAME).ept
FFTSRC      = ./$(FFTNAME).ept
APPSRC      = ./$(APPNAME).ept
CST_C       = $(CSTDIR)/$(CSTNAME).c
CST_TYPES_C = $(CSTDIR)/$(CSTNAME)_types.c
CST_H       = $(CSTDIR)/$(CSTNAME).h
CST_TYPES_H = $(CSTDIR)/$(CSTNAME)_types.h
CMP_C       = $(CMPDIR)/$(CMPNAME).c
CMP_TYPES_C = $(CMPDIR)/$(CMPNAME)_types.c
CMP_H       = $(CMPDIR)/$(CMPNAME).h
CMP_TYPES_H = $(CMPDIR)/$(CMPNAME)_types.h
FFT_C       = $(FFTDIR)/$(FFTNAME).c
FFT_TYPES_C = $(FFTDIR)/$(FFTNAME)_types.c
FFT_H       = $(FFTDIR)/$(FFTNAME).h
FFT_TYPES_H = $(FFTDIR)/$(FFTNAME)_types.h
APP_C       = $(APPDIR)/$(APPNAME).c
APP_TYPES_C = $(APPDIR)/$(APPNAME)_types.c
APP_H       = $(APPDIR)/$(APPNAME).h
APP_TYPES_H = $(APPDIR)/$(APPNAME)_types.h
SNDIO_H     = $(SNDIODIR)/$(SNDIO).h
SNDIO_C     = $(SNDIODIR)/$(SNDIO).c
FLOAT_H     = $(FLOATDIR)/$(FLOAT).h
FLOAT_C     = $(FLOATDIR)/$(FLOAT).c
FFTC_H      = $(FFTCDIR)/$(FFTC).h
FFTC_C      = $(FFTCDIR)/$(FFTC).c
PERVASIVE_H = ./pervasives.h
CMAIN       = ./main.c

CHEADERS    = $(PERVASIVE_H) $(SNDIO_H) $(FLOAT_H) $(FFTC_H) \
	      $(CST_H) $(CST_TYPES_H) \
	      $(CMP_H) $(CMP_TYPES_H) \
	      $(FFT_H) $(FFT_TYPES_H) \
	      $(APP_H) $(APP_TYPES_H)
CFILES      = $(CMAIN) $(SNDIO_C) $(FLOAT_C) $(FFTC_C)\
	      $(FFT_C) $(FFT_TYPES_C) \
	      $(CST_C) $(CST_TYPES_C) \
	      $(CMP_C) $(CMP_TYPES_C) \
	      $(APP_C) $(APP_TYPES_C)
OBJECTS     = $(CFILES:.c=.o) $(FLOAT_C:.c=.o)

#===============================================================
# LIBRARIES, which have a general compilation script
#===============================================================


all: $(APPNAME)

echo:
	echo $(CSTNAME) $(CMPPNAME) $(FFTNAME) $(APPNAME) $(SNDIO) $(FLOAT) $(FFTC)
	echo $(LIBDIR) $(CSTDIR) $(CMPDIR) $(FFTDIR) $(APPDIR) $(SNDIODIR) $(FLOATDIR) $(FFTCDIR)
	echo $(FFT_C) $(FFT_TYPES_C) $(FFT_H) $(FFT_TYPES_H) $(FFTNAME).epci $(FFTNAME).ept $(CSTNAME).epci $(CMPNAME).epci $(FFTC).epci



$(APPNAME): $(OBJECTS)
	$(LD) $^ -lm -o $@

# Rules for Heptagon output compilation
$(CSTDIR)/%.o : $(CSTDIR)/%.c $(LIB_H)
	$(GCC) -c $< -o $@

$(CMPDIR)/%.o : $(CMPDIR)/%.c $(LIB_H)
	$(GCC) -c $< -o $@

$(FFTDIR)/%.o : $(FFTDIR)/%.c $(LIB_H) $(CST_H) $(CMP_H) $(FFTC_H)
	$(GCC) -c $< -o $@

$(APPDIR)/%.o : $(APPDIR)/%.c $(CHEADERS)
	$(GCC) -c $< -o $@

# Rule for external files, which do not depend upon some headers
$(FLOATDIR)/%.o : $(FLOATDIR)/%.c $(LIB_H) $(FLOAT_H)
	$(GCC) -c $< -o $@

$(FFTCDIR)/%.o : $(FFTCDIR)/%.c $(LIB_H) $(FFTC_H) $(CST_H) $(CMP_H) 
	$(GCC) -c $< -o $@

$(SNDIODIR)/%.o : $(SNDIODIR)/%.c $(LIB_H) $(CMP_H) $(CST_H) $(SNDIO_H)
	$(GCC) -c $< -o $@

# Rule for main.c
main.o: main.c $(CHEADERS)
	$(GCC) $(MAIN_CFLAGS) -c $< -o $@

# Heptagon compilation rules
$(CST_C) $(CST_TYPES_C) $(CST_H) $(CST_TYPES_H) $(CSTNAME).epci : $(CSTSRC)
	$(HEPTC) -target c $<

$(CMP_C) $(CMP_TYPES_C) $(CMP_H) $(CMP_TYPES_H) $(CMPNAME).epci $(CMPNAME).epo : $(CMPSRC) $(FLOAT).epci
	$(HEPTC) -target c $<

$(SNDIO).epci: $(SNDIO).epi $(CSTNAME).epci
	$(HEPTC) -c $<

$(FLOAT).epci: $(FLOAT).epi
	$(HEPTC) -c $<

$(FFTC).epci: $(FFTC).epi $(CMPNAME).epci $(FLOAT).epci
	$(HEPTC) -c $<

$(FFT_C) $(FFT_TYPES_C) $(FFT_H) $(FFT_TYPES_H) $(FFTNAME).epci : $(FFTNAME).ept $(CMPNAME).epci $(FFTC).epci
	$(HEPTC) -target c $<

$(APP_C) $(APP_TYPES_C) $(APP_H) $(APP_TYPES_H) : $(APPNAME).ept $(CMPNAME).epci $(FFTC).epci $(SNDIO).epci $(FFTNAME).epci 
	$(HEPTC) -target c $<

clean:
	echo $(OBJECTS)
	rm -rf $(OBJECTS) $(APPNAME) $(APPDIR) $(CMPDIR) $(CSTDIR) $(FFTDIR) *.epci *.mls *~ */*~ *.epo *.obc

run: $(APPNAME)
	rec -t raw -r 44100 -e signed -b 16 -c 2 - | \
	./$< | \
        play -t raw -r 44100 -e signed -b 16 -c 2 -
