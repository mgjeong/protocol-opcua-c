# OPC UA UNIT TESTS

## How to build and run OPC UA Unit Tests

1. Environment : Ubuntu

2. [ONE TIME ONLY]  Run ./prep.sh in 'device-opcua-c/unittests/' folder.
	(This command is run only once, as it downloads, and builds the gtest library.)

3. Run 'scons' command in 'device-opcua-c/unittests/' folder to build the test file.
	(Note : OPC_UA and gtest should have been already built['libopcua-adapter.so' and 'libgtest.so' generated], prior to running unit tests.)


4. Scons will automatically copy required libs, and copy to libs folder. 

5. Run 'export LD_LIBRARY_PATH=libs' in command line, to give path to libraries for executables.

6. Run './opcuaTest --gtest_output="xml:./opcuatestReport.xml"' command to execute test cases and get report in file 'opcuatestReport.xml'.

7. Run './opcuaTest' command to execute test cases in view report in shell itself.


