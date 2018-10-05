OPC-UA Protocol Stack (C) for windows
================================

This provides opcua protocol stack library

## Prerequisites ##

- Python 3.x 
  - Version : 3.x 
  - Download from https://www.python.org
  - you can find **Windows x86_64 executable installer** version
  - install
  - Add pyhton 3.X to the System PATH
  - Also Install python-six with pip package manager (pip install six)
  ```shell
  C:\[my protocol-opcua-c path]\pip install six
  ```
  
- SCons
  - Version : 2.5.1 or above
  - Download and install from https://scons.org/pages/download.html or download page of https://scons.org
  ```shell
  e.g) Proir stable (3.0.0)
          . Windows installer scons-3.0.0-setup.exe
  ```
  - Add scons to the System PATH

- cmake
  - Version : Upper 3.10
  - Download https://cmake.org/download/ or download page of https://cmake.org
  - e.g) you can find **cmake-3.12.3-wind64-x64.msi** of Windows wind64-x64 installer platform
  - install
  - Add cmake to the System PATH

- Visual Studio 2015 Community Edition / Express edition (Community Edition is needed License)
  - On launching the Community edition installer, select 'Custom' and then select 'VC++'

- Microsoft .NET (msbuild: v3.5)
  - Default path for msbuild tool is : C:\Windows\Microsoft.NET\Framework64\v3.5
  - Add above path to the System PATH

- gnuwin/wget for windows
  - Download and install from https://sourceforge.net/projects/gnuwin32/files/
    1. GnuWin (e.g. sed-4.2.1-setup.exe) of Home path
    2. wget (e.g. wget-1.11.4-1-setup.exe) of Home/wget/1.11.4-1 path
  - Add wget to the System PATH
  
- git for windows
  - Download and install from https://git-scm.com/
  - Add git to the System PATH


## Building OPC UA library and sample : [Auto Build script] ##

### Build ###
- Open Command prompt in windows

- set vcvarall (turn cmd to visual studio terminal)
  1. In case of VS 2015 Community edition
    ```shell
    C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat amd64
    ```
  2. ~~(not support yet) In case of VS 2017 Community edition~~
    ```shell
    C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat x86_amd64
    ```
  3. ~~(not support yet) In case of VS 20157 Express edition~~
    ```shell
    C:\Program Files (x86)\Microsoft Visual Studio\2017\WDExpress\VC\Auxiliary\Build\vcvarsall.bat x86_amd64
    ```

- $ cd ~\protocol-opcua-c

- $ build_windows.bat

### Run Sample ###
- Move Sample path
   ```shell
   C:\~\protocol-opcua-c\example\out\
   ```
- Execute server
   ```shell
   C:\~\protocol-opcua-c\example\out\server.exe
   ```
- Execute client
   ```shell
   C:\~\protocol-opcua-c\example\out\client.exe
   ```
- Also you can find **opcua-adapter.dll** in **protocol-opcua-c\build**
