#!/bin/bash

# get build depends
sudo apt-get install g++ scons libglew1.5-dev libpng12-dev libjpeg62-dev
#sudo apt-get install m4 libxxf86vm-dev libopenal-dev libasound2-dev g++-multilib gcc-multilib zlib1g-dev libxext-dev

# make sure we have a libpng to link to
sudo ln -s /lib32/libpng12.so.0 /lib32/libpng.so