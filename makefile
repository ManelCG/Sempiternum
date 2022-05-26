BDIR = build
SDIR = src

IDIR = include
#CC = x86_64-w64-mingw32-gcc
CC = gcc
CFLAGS = -I$(IDIR) `pkg-config --cflags --libs gtk+-3.0` -Wall -O2

ODIR=.obj
LDIR=lib

LIBS = -lm -lOpenCL -lpthread

_DEPS = lodepng.h draw_julia.h opencl_funcs.h file_io.h ComplexPlane.h image_manipulation.h gui_gen_video.h complex_function.h gui_templates.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = lodepng.o draw_julia.o opencl_funcs.o file_io.o ComplexPlane.o image_manipulation.o complex_function.o gui_templates.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

_OBJ_cli = main.o
OBJ_cli = $(patsubst %,$(ODIR)/%,$(_OBJ_cli))

_OBJ_GUI = GUI.o gui_gen_video.o
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
