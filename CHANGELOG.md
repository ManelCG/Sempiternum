# Change Log

## [0.1.1] - UNRELEASED

This update is oriented towards the implementation of custom functions, and in general, an optimization and improvement when it comes to loading the OpenCL source code.

### Added

* The Custom Function mode is now added, which will look into the opencl/custom folder and load any .c or .cl files found there, appending them to the main OpenCL Code.

### Changed

* The ```draw_julia.c``` function now reads the contents of the ```opencl/draw_julia_headers/``` folder, then the main OpenCL file, then the custom files.

* The main OpenCL file is now separated into multiple header files.

* The ```draw_julia.c``` file now includes a ```draw_julia_load_opencl_src``` function, called by all of the other ```draw_julia*``` functions which loads the OpenCL source code. This makes the OpenCL loading process more centralized and easier to tweak.


## [0.1.0] - 27-Jul-2022

First official Beta release!!! Yay!!!

### Installation

Now Sempiternum can be installed with the ```sudo make install``` command in any Linux distribution.

For Arch-based distributions, Sempiternum is also available on the AUR as ```sempiternum```. Any AUR helper can install Sempiternum. If you want to use yay, just run ```yay -S sempiternum```

### Added

* Shows convergence point of the starting value pointed at by the cursor, whenever Display Orbits is toggled on.

* Added many different colorschemes which can be changed from the Preferences window.

### Changed

* Fixed some usability issues regarding parameter spaces. When changing from Dynamic Plane to Parameter Space, will always try to draw from critical point.
