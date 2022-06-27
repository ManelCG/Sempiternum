SDIR = src

IDIR = include
#CCCMD = x86_64-w64-mingw32-gcc
CCCMD = gcc
CFLAGS = -I$(IDIR) `pkg-config --cflags --libs gtk+-3.0` -Wall -Wno-deprecated-declarations

debug: CC = $(CCCMD) -DDEBUG_GUI -DDEBUG_CP -DDEBUG_DRAW_JULIA_C -DDEBUG_IMAGE_MANIPULATION_C -DDEBUG_GUI_TEMPLATES -DDEBUG_OPENCL_FUNCS_C -DDEBUG_GUI_GEN_VIDEO_C
debug: BDIR = debug

release: CC = $(CCCMD) -O2
release: BDIR = build

install: CC = $(CCCMD) -O2	-DMAKE_INSTALL
install: SEMPITERNUM_DIR = /usr/share/sempiternum
install: BDIR = $(SEMPITERNUM_DIR)/install

archlinux: CC = $(CCCMD) -O2 -DMAKE_INSTALL

ODIR=.obj
LDIR=lib

LIBS = -lm -lOpenCL -lpthread

_DEPS = lodepng.h draw_julia.h opencl_funcs.h file_io.h ComplexPlane.h image_manipulation.h gui_gen_video.h complex_function.h gui_templates.h complex_plane_colorschemes.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = lodepng.o draw_julia.o opencl_funcs.o file_io.o ComplexPlane.o image_manipulation.o complex_function.o gui_templates.o complex_plane_colorschemes.o
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
	mkdir -p $(ODIR)
	$(CC) -o $(BDIR)/$@ $^ $(CFLAGS) $(LIBS)

release: $(OBJ) $(OBJ_GUI)
	mkdir -p $(BDIR)
	mkdir -p $(ODIR)
	$(CC) -o $(BDIR)/sempiternum $^ $(CFLAGS) $(LIBS)

debug: $(OBJ) $(OBJ_GUI)
	mkdir -p $(BDIR)
	mkdir -p $(ODIR)
	$(CC) -o $(BDIR)/sempiternum $^ $(CFLAGS) $(LIBS)

install: $(OBJ) $(OBJ_GUI)
	mkdir -p $(SEMPITERNUM_DIR)
	mkdir -p $(BDIR)
	mkdir -p $(ODIR)
	$(CC) -o $(BDIR)/sempiternum $^ $(CFLAGS) $(LIBS)
	cp -r opencl/ $(SEMPITERNUM_DIR)
	cp -r assets/ $(SEMPITERNUM_DIR)
	ln -sf $(BDIR)/sempiternum /bin/sempiternum
	cp assets/sempiternum.desktop /usr/share/applications/

archlinux: $(OBJ) $(OBJ_GUI)
	mkdir -p $(BDIR)/usr/share/sempiternum
	mkdir -p $(BDIR)/usr/share/applications
	mkdir -p $(BDIR)/usr/bin/
	mkdir -p $(ODIR)
	$(CC) -o $(BDIR)/usr/bin/sempiternum $^ $(CFLAGS) $(LIBS)
	cp -r opencl/ $(BDIR)/usr/share/sempiternum/
	cp -r assets/ $(BDIR)/usr/share/sempiternum/
	cp assets/sempiternum.desktop $(BDIR)/usr/share/applications/

.PHONY: clean
clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~

.PHONY: all
all: main release clean
