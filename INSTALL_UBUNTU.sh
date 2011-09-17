#!/bin/sh
# heekscad-install.sh -- Downloads, builds and installs HeeksCAD from svn
DIR="$( cd "$( dirname "$0" )" && pwd )"  # find out the current directory
BUILDPATH=$DIR             # Location of HeeksCAD build dir
BUILDPREREQS="libwxbase2.8-dev cmake \
  build-essential libwxgtk2.8 libwxgtk2.8-dev ftgl-dev\
  libgtkglext1-dev python-dev cmake libboost-python-dev"

# Install build prerequisites
# The next two lines can be commented out after the initial run.
sudo apt-get update
sudo apt-get install -y $BUILDPREREQS
sudo git clone --recursive git://github.com/Heeks/heekscad.git

cd ${BUILDPATH}/heekscad/heekscnc/oce
if [ -d build]; then
  cd build
else
  mkdir build
  cd build
fi
cmake ..
make -j8
sudo make install

cd ${BUILDPATH}/heekscad/
cmake .
make -j8 package
sudo dpkg -i heekscad_*.deb

# Install HeeksCNC
cd ${BUILDPATH}/heekscad/heekscnc/
cmake .
make -j8 package
sudo dpkg -i heekscnc_*.deb

# Install libarea
# area.so is required for pocket operations.
#Get the libarea files from the SVN repository, build, and install
cd ${BUILDPATH}/heekscad/heekscnc/libarea/
make clean
make -j8
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
make -j8
# make doc        # Creates PDF file needed by make install
sudo make install
