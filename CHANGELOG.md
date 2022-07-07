# Change Log

## [0.1.1] - UNRELEASED

This update is oriented towards the implementation of custom functions, and in general, an optimization and improvement when it comes to loading the OpenCL source code.

### Added

* The Custom Function mode is now added, which will look into the opencl/custom folder and load any .c or .cl files found there, appending them to the main OpenCL Code.



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
