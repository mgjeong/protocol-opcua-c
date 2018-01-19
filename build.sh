#!/bin/sh
echo "Start edge opcua build"

scons -c
scons TEST=1

echo "End of edge opcua build"
