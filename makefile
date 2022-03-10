BDIR = build
SDIR = src

IDIR = include
CC = gcc
CFLAGS = -I$(IDIR)

ODIR=.obj
LDIR=lib

LIBS = -lm -lOpenCL

_DEPS = lodepng.h draw_julia.h opencl_funcs.h file_io.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o lodepng.o draw_julia.o opencl_funcs.o file_io.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	mkdir -p $(BDIR)
	$(CC) -o $(BDIR)/$@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~

.PHONY: all
all: main clean
