OPC-UA Protocol Stack (C)
================================

This provides opcua protocol stack library

## Prerequisites ##

- Python
  - Version : 3.x
  - Install python-six with pip package manager (pip install six)
  ```shell
  C:\[My Project]\pip install six
  ```
    
- SCons
  - Version : 2.5.1 or above
  - [How to install](https://scons.org/doc/1.2.0/HTML/scons-user/x166.html)

- cmake
  - Version : Upper 3.10
  - Download and install https://cmake.org/download/
  - Add wget to PATH

- Visual Studio 2015 Community Edition
  - On launching the installer, select 'Custom' and then select 'VC++'

- Microsoft .NET (msbuild: v3.5)
  - Default path for msbuild tool is : C:\Windows\Microsoft.NET\Framework64\v3.5
  - Add above path to PATH

- gnuwin/wget for windows
  - Download and install from https://sourceforge.net/projects/gnuwin32/files/
    1. GnuWin
    2. wget
  - Add wget to PATH
  
- git for windows
  - Download and install from https://git-scm.com/
  - Add wget to PATH

## Building OPC UA library and sample : [Auto Build script] ##

- Open windows Command prompt

- In case of VS 2015 community edition, 
	- $ call C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat amd64

  In case of VS 2017 community edition, 
	- $ call C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat x86_amd64

- The above command will turn cmd to visual studio terminal.

- $ cd ~\protocol-opcua-c

- $ build_windows.bat

