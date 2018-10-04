OPC-UA Protocol Stack (C)
================================

This provides opcua protocol stack library

## Prerequisites ##

- Python
  - Version : 3.x
  - Install python-six with pip package manager (pip install six)

- SCons
  - Version : 2.5.1 or above
  - [How to install](https://scons.org/doc/1.2.0/HTML/scons-user/x166.html)

- cmake
  - Version : Upper 3.10
  - [Where to download](https://cmake.org/download/)

- Visual Studio 2015 or above
  - On launching the installer, select 'Custom' and then select 'VC++'

- Microsoft .NET (msbuild: v3.5)
  - Default path for msbuild tool is : C:\Windows\Microsoft.NET\Framework64\v3.5
  - Add above path to PATH

- wget for windows
  - Download and install from https://sourceforge.net/projects/gnuwin32/files/
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

