BDIR = build
SDIR = src

IDIR = include
CC = gcc
CFLAGS = -I$(IDIR) `pkg-config --cflags --libs gtk+-3.0`

ODIR=.obj
LDIR=lib

LIBS = -lm -lOpenCL

_DEPS = lodepng.h draw_julia.h opencl_funcs.h file_io.h ComplexPlane.h image_manipulation.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = lodepng.o draw_julia.o opencl_funcs.o file_io.o ComplexPlane.o image_manipulation.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

_OBJ_cli = main.o
OBJ_cli = $(patsubst %,$(ODIR)/%,$(_OBJ_cli))

_OBJ_GUI = GUI.o
OBJ_GUI = $(patsubst %,$(ODIR)/%,$(_OBJ_GUI))


$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJ) $(OBJ_cli)
	mkdir -p $(BDIR)
	$(CC) -o $(BDIR)/$@ $^ $(CFLAGS) $(LIBS)

JuliaViewerGUI: $(OBJ) $(OBJ_GUI)
	mkdir -p $(BDIR)
	$(CC) -o $(BDIR)/$@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~

.PHONY: all
all: main JuliaViewerGUI clean
