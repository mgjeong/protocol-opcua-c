#!/bin/sh
echo "Start edge opcua build"

scons -c
scons TARGET_ARCH=linux TEST=1

echo "End of edge opcua build"
