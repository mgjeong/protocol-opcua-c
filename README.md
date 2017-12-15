# OPC UA C library

## How to build OPC UA library and examples

1. Environment : Ubuntu

2. a. Run 'scons' in command line.
   b. Run 'scons TEST=1' in command line to build with unittests.
   
3. You can find libopua-adpater.so in 'build' folder.

4. You can find server and client executable in 'example/out' folder.

## How to run server and client examples

1. Go to 'example/out' folder.

2. Export 'libopcua-adapter.so' library path for executables: 
	Run command : 'export LD_LIBRARY_PATH=../../build'
	
3. Run the server and client :
	a. server : './server'
	a. client : './client'
