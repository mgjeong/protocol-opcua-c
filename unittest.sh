#!/bin/sh
echo "Start edge opcua build"
PROJECT_ROOT=$(pwd)
echo $PROJECT_ROOT

cd $PROJECT_ROOT
./build.sh
cd ./unittests
./prep.sh
scons TEST=1

echo "End of edge opcua build"
