#!/bin/sh
echo "Start edge opcua build"

scons TEST=1
cp /usr/bin/qemu-aarch64-static .

echo "End of edge opcua build"

