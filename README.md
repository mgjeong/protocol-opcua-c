OPC-UA Protocol Stack (C)
================================

This provides opcua protocol stack library 

## Prerequisites ##

- You must install basic prerequisites for build
  ```shell
  $ sudo apt-get install build-essential
  ```

## How to build  ##

#### 1. Command ####

```shell
$ ./build.sh
```

##### Binaries #####
- opcua protocol stack library : build/libopcua-adapter.so

## How to run server and client examples

1. Go to 'example/out' folder.

2. Export 'libopcua-adapter.so' library path for executables:

	Run command : `export LD_LIBRARY_PATH=../../build`
	
3. Run the server and client :

        a. server : `./server`

        b. client : `./client`
