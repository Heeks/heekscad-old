#!/bin/sh
# heekscad-install.sh -- Downloads, builds and installs HeeksCAD from svn

BUILDPATH=~             # Location of HeeksCAD build dir
BUILDPREREQS="libwxbase2.8-dev cmake \
  build-essential libwxgtk2.8 libwxgtk2.8-dev ftgl-dev\
  libgtkglext1-dev python-dev cmake libboost-python-dev"

# Install build prerequisites
# The next two lines can be commented out after the initial run.
sudo apt-get update
sudo apt-get install -y $BUILDPREREQS

cd ${BUILDPATH}/heekscad/heekscnc/oce
if [ -d build]; then
  cd build
else
  mkdir build
  cd build
fi
cmake ..
make 
sudo make install

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

# Install libactp
#cd ${BUILDPATH}/heekscad/heekscnc/libactp/PythonLib
#make clean
#make
#sudo make install
#sudo ln -s .libs/actp.so ${BUILDPATH}/heekscad/heekscnc/actp.so

# Install opencamlib
cd ${BUILDPATH}/heekscad/heekscnc/opencamlib/src
cmake .
make
make doc        # Creates PDF file needed by make install
sudo make install
