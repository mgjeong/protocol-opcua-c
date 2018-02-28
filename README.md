OPC-UA Protocol Stack (C)
================================

This provides opcua protocol stack library 

## Prerequisites ##

- You must install basic prerequisites for build
  ```shell
  $ sudo apt-get install build-essential
  ```
- cmake
  - Version : Upper 3.10
  - [Where to download](https://cmake.org/download/)
  - [How to install](https://cmake.org/install/)

## How to build  ##

#### 1. Command ####

```shell
$ ./build.sh
```

##### Binaries #####
- opcua protocol stack library : build/libopcua-adapter.so

##### Note #####
- Build script will download open62541 library automatically from github.
- After download, it will be built for 'single file distribution' mode which combines
all header files into a single header file(open62541.h) and all source files into a single source file(open62541.c).
- After building, all library files and folders except open62541.h and open62541.c will be deleted.
- Final location of open62541.h and open62541.c: extlibs/open62541/open62541/
- These two files will be directly included and built along with OPC-UA protocol stack.

## How to run server and client examples

1. Go to 'example/out' folder.

2. Export 'libopcua-adapter.so' library path for executables:

	Run command : `export LD_LIBRARY_PATH=../../build`
	
3. Run the server and client :

        1. server : `./server`

        2. client : `./client`
