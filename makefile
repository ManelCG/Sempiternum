SEMPITERNUM_VERSION = "0.1.1 Development"

SDIR = src

IDIR = include
#CCCMD = x86_64-w64-mingw32-gcc
CCCMD = gcc
CFLAGS = -I$(IDIR) `pkg-config --cflags --libs gtk+-3.0` -Wall -Wno-deprecated-declarations

debug: CC = $(CCCMD) -DDEBUG_GUI -DDEBUG_CP -DDEBUG_DRAW_JULIA_C -DDEBUG_IMAGE_MANIPULATION_C -DDEBUG_GUI_TEMPLATES -DDEBUG_OPENCL_FUNCS_C -DDEBUG_GUI_GEN_VIDEO_C -DSEMPITERNUM_VERSION=\"$(SEMPITERNUM_VERSION)\"
debug: BDIR = debug

release: CC = $(CCCMD) -O2 -DSEMPITERNUM_VERSION=\"$(SEMPITERNUM_VERSION)\"
release: BDIR = build

install: CC = $(CCCMD) -O2	-DMAKE_INSTALL -DSEMPITERNUM_VERSION=\"$(SEMPITERNUM_VERSION)\"
install: SEMPITERNUM_DIR = /usr/lib/sempiternum
install: BDIR = $(SEMPITERNUM_DIR)/bin

archlinux: CC = $(CCCMD) -O2 -DMAKE_INSTALL -DSEMPITERNUM_VERSION=\"$(SEMPITERNUM_VERSION)\"

ODIR=.obj
LDIR=lib

LIBS = -lm -lOpenCL -lpthread

_DEPS = lodepng.h draw_julia.h opencl_funcs.h file_io.h ComplexPlane.h image_manipulation.h gui_gen_video.h complex_function.h gui_templates.h complex_plane_colorschemes.h custom_function.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = lodepng.o draw_julia.o opencl_funcs.o file_io.o ComplexPlane.o image_manipulation.o complex_function.o gui_templates.o complex_plane_colorschemes.o custom_function.o
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
	ln -sf $(BDIR)/sempiternum /usr/bin/sempiternum
	cp assets/sempiternum.desktop /usr/share/applications/
	cp assets/app_icon/256.png /usr/share/pixmaps/sempiternum.png

archlinux: $(OBJ) $(OBJ_GUI)
	mkdir -p $(BDIR)/usr/lib/sempiternum
	mkdir -p $(BDIR)/usr/share/applications
	mkdir -p $(BDIR)/usr/share/pixmaps
	mkdir -p $(BDIR)/usr/bin/
	mkdir -p $(ODIR)
	$(CC) -o $(BDIR)/usr/bin/sempiternum $^ $(CFLAGS) $(LIBS)
	cp -r opencl/ $(BDIR)/usr/lib/sempiternum/
	cp -r assets/ $(BDIR)/usr/lib/sempiternum/
	cp assets/sempiternum.desktop $(BDIR)/usr/share/applications/
	cp assets/app_icon/256.png $(BDIR)/usr/share/pixmaps/sempiternum.png

.PHONY: clean
clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~

.PHONY: all
all: main release clean
