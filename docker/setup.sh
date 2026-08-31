#!/bin/bash

apt-get update

ln -fs /usr/share/zoneinfo/UTC /etc/localtime
DEBIAN_FRONTEND=noninteractive apt install -y tzdata
dpkg-reconfigure --frontend noninteractive tzdata

apt-get install -y \
    cmake clang libasound2-dev \
    libxext-dev libglew-dev \
    libcurl4-openssl-dev build-essential \
    libsdl2-dev libsdl2-image-dev \
    python3 git libgtk2.0-dev

# update-alternatives --config c++
# update-alternatives --config cc
