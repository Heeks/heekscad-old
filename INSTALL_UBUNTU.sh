#!/bin/sh
# heekscad-install.sh -- Downloads, builds and installs HeeksCAD from svn

BUILDPATH=~             # Location of HeeksCAD build dir
BUILDPREREQS="libwxbase2.8-dev cmake \
  build-essential libwxgtk2.8 libwxgtk2.8-dev ftgl-dev\
  libgtkglext1-dev python-dev libboost-python-dev libopencascade-dev python-wxgtk2.8 libboost-python1.49.0"

# Install build prerequisites
# The next two lines can be commented out after the initial run.
sudo apt-get update
sudo apt-get install -y $BUILDPREREQS

cd ${BUILDPATH}/heekscad/
cmake .
make package
sudo dpkg -i heekscad_*.deb

# Install HeeksCNC
cd ${BUILDPATH}/heekscad/heekscnc/
cmake .
make package
sudo dpkg -i heekscnc_*.deb

# Install libarea
# area.so is required for pocket operations.
#Get the libarea files from the SVN repository, build, and install
cd ${BUILDPATH}/heekscad/heekscnc/libarea/
make clean
make
sudo make install
sudo ln -s .libs/area.so ${BUILDPATH}/heekscad/heekscnc/area.so

