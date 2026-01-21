# kdevcvs (KDevelop CVS Plugin)

<p align="center">
  <img src="kdevcvs.png" alt="KDevelop CVS Plugin"/>
</p>

This is a fork of the kdevelop discontinued cvs plugin.

I forked this because I work at a company that uses this plugin and I commited this plugin to my github in hopes that it will be useful to other developers.

## Compilation

### Clone the repository

`$ git clone https://github.com/Bacagine/kdevcvs.git`

`$ cd kdevcvs`

### Symbolic link for KDevelop 5

`$ ln -s CMakeLists.txt.qt5 CMakeLists.txt`

### Symbolic link for KDevelop 6

`$ ln -s CMakeLists.txt.qt6 CMakeLists.txt`

### Build and make

`$ mkdir build`

`$ cd build`

`$ cmake ..`

`$ make`

## Installation

After compile, you need copy the kdevcvs.so to the KDevelop plugins directory.

### For example:

#### In Slackware 15:

`# cp kdevcvs.so /usr/lib64/qt5/plugins/kdevplatform/35/`

#### In Debian/PureOS:

`# cp kdevcvs.so /usr/lib/x86_64-linux-gnu/qt5/plugins/kdevplatform/34/`

### In Arch Linux

`# cp kdevcvs.so /usr/lib64/qt6/plugins/kdevplatform/64/`

