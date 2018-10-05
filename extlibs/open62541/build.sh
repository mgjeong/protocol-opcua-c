###############################################################################
# Copyright 2017 Samsung Electronics All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
###############################################################################

#!/bin/bash
# open62541 build script

# Get into the open62541 library
versionName="0.2"

if [ -d "open62541" ]; then
    rm -rf open62541
fi

cd "open62541_$versionName"

# If build directory for libwebsockets library exists, then remove it.
build_dir="$PWD/build"
if [ -d "$build_dir" ]; then
    rm -rf build
fi

# Create build directory
mkdir -p build

# Get into the build directory
cd build

cmake .. -DCMAKE_BUILD_TYPE=Release -DUA_ENABLE_AMALGAMATION=ON -DUA_ENABLE_ENCRYPTION=OFF

make

# Delete all files and folders except open62541.c and open62541.h.
cp open62541.h ../../
cp open62541.c ../../

cd ../../

rm -rf "open62541_$versionName"
mkdir "open62541_$versionName"

mv open62541.h open62541.c "open62541_$versionName"
