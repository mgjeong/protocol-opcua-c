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

- SCons
  - Version : 2.3.0 or above
  - [How to install](http://scons.org/doc/2.3.0/HTML/scons-user/c95.html)

- Python
  - Version : 2.7.X

- pip
  - [How to install](https://pip.pypa.io/en/latest/installing/)
   ```shell
   $ curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py
   $ python get-pip.py
   ``` 

## How to build  ##

```shell
$ ./build.sh
$ ./build_arm.sh  : for arm architecture
```

##### Build Options #####
- --build_mode = [release/debug] 

  If you want to build to debug mode, then please make this option [debug/DEBUG]. Default value is [release].

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

#### How to run browseNext in client example ####

1. 'start' : Connection establishment.

2. 'set_max_ref' : Set the maximum references per node to be returned. [ex 10]

3. 'browse' : Performs browse with '10' as maximum references per node.

`...`
`<browse callback may come with 1 or more continuation points. Callback in sample stores all of them in a list>`
`...`

4. 'browse_next' : Perform browse next with stroed continuation points

`...`
`<browse callback may come with 1 or more continuation points again. Callback in sample stores all of them in a list>`
`...`

5. go to Step 4.
   Step 4 can be continued until continuation point list becomes empty.

#### OPC-UA protocol stack library for Windows ####
  - [How to build OPC-UA protocol stack library](https://github.sec.samsung.net/RS7-EdgeComputing/protocol-opcua-c/edit/master/README_windows.md)
