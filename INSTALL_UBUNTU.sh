#!/bin/sh
# INSTALL_UBUNTU.sh -- Downloads, builds and installs HeeksCAD and related projects for ubuntu
DIR="$( cd "$( dirname "$0" )" && pwd )"  # find out the current directory
BUILDPATH=$DIR             # Location of HeeksCAD build dir
BUILDPREREQS="libwxbase2.8-dev cmake \
  build-essential libwxgtk2.8 libwxgtk2.8-dev ftgl-dev \
  libgtkglext1-dev python-dev cmake libboost-python-dev"

# Install build prerequisites
# The next two lines can be commented out after the initial run.
sudo apt-get update
sudo apt-get install -y $BUILDPREREQS

cd ${BUILDPATH}
if [ ! -d heekscad ]; then 
  git clone --recursive git://github.com/Heeks/heekscad.git
fi

cd ${BUILDPATH}/heekscad/heekscnc/oce
if [ -d build ]; then
  cd build
else
  mkdir build
  cd build
fi
cmake ..
#make -j8
make 
sudo make install

cd ${BUILDPATH}/heekscad/
cmake .

#make package -j8  #For faster build on multi-core machines, uncomment this line and comment out the next
make package 
sudo dpkg -i heekscad_*.deb

# Install HeeksCNC
cd ${BUILDPATH}/heekscad/heekscnc/
cmake .

#make package -j8 #For faster build on multi-core machines, uncomment this line and comment out the next
make package 
sudo dpkg -i heekscnc_*.deb

# Install libarea
# area.so is required for pocket operations.
#Get the libarea files from the SVN repository, build, and install
cd ${BUILDPATH}/heekscad/heekscnc/libarea/
make clean

#make -j8 #For faster build on multi-core machines, uncomment this line and comment out the next
make
sudo make install
sudo ln -s .libs/area.so ${BUILDPATH}/heekscad/heekscnc/area.so

# Install opencamlib
cd ${BUILDPATH}/heekscad/heekscnc/opencamlib/
if [ -d build]; then
cd build
else
mkdir build
 cd build
fi
cmake ../src

#make -j8 package #For faster build on multi-core machines, uncomment this line and comment out the next
make package
sudo dpkg -i opencamlib_*.deb

