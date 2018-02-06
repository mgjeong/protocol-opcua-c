#!/bin/sh
echo "Start edge opcua build"

scons -c
scons TARGET_ARCH=arm TEST=1
cp /usr/bin/qemu-arm-static .

echo "End of edge opcua build"
