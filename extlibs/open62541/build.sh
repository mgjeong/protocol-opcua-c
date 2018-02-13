#!/bin/sh
# open62541 build script

# Get into the open62541 library
cd open62541

# If build directory for libwebsockets library exists, then remove it.
build_dir="$PWD/build"
if [ -d "$build_dir" ]; then
    rm -rf build
fi

# Create build directory
mkdir build

# Get into the build directory
cd build

cmake .. -DCMAKE_BUILD_TYPE=Release -DUA_ENABLE_AMALGAMATION=ON -DUA_ENABLE_ENCRYPTION=OFF

make

# Delete all files and folders except open62541.c and open62541.h.
cp open62541.h ../../
cp open62541.c ../../

cd ../../

rm -rf open62541
mkdir open62541

mv open62541.h open62541.c open62541
