@ECHO OFF
    set cwd=%cd%
	set versionName="0.2"
	set open62541_dir=%cwd%\extlibs\open62541

	IF "%~1"=="" GOTO BUILD_OPCUA_LIB
	GOTO ERROR_INVALID_PARAM

:WITH_NO_FLAG
	ECHO Dependency option not provided or set to false.
	GOTO BUILD_PROTOCOL_OPCUA_C

:BUILD_OPCUA_LIB
	cd %open62541_dir%
	IF EXIST "open62541_%versionName%" (
		ECHO open62541 library already exists
		GOTO PTHREAD_LIB
	) ELSE (
		cd %cwd%
		cd ..	
		:: clone 
		IF EXIST "open62541_%versionName%" (
			rmdir /Q /S open62541_%versionName%
		)
		git clone https://github.com/open62541/open62541.git open62541_%versionName%
		cd open62541_%versionName%
		git reset --hard 2be85c43813f7bb0c8fd420b93ab469427ccb4cf
		
		mkdir build
		cd build

		cmake .. -DCMAKE_BUILD_TYPE=Release -DUA_ENABLE_AMALGAMATION=ON -DUA_ENABLE_ENCRYPTION=OFF

		MSBuild /property:Configuration=Release open62541.vcxproj

		mkdir %open62541_dir%\open62541_%versionName%
		copy open62541.h %open62541_dir%\open62541_%versionName%
		copy open62541.c %open62541_dir%\open62541_%versionName%
		
		:: remove the directory
		cd ..\..\
		rmdir /Q /S open62541_%versionName%
		cd %cwd%		
		GOTO PTHREAD_LIB	
	)

:PTHREAD_LIB
	ECHO pthread library
	cd %cwd%\extlibs\
	IF EXIST "pthread-win32" (
		ECHO pthread-win32 already exists
		GOTO BUILD_PROTOCOL_OPCUA_C
	) ELSE (
		ECHO make directory
		mkdir pthread-win32
		wget --no-check-certificate -r -np -nH --cut-dirs=3 https://sourceware.org/pub/pthreads-win32/dll-latest/dll/ -P pthread-win32
		wget --no-check-certificate -r -np -nH --cut-dirs=3 https://sourceware.org/pub/pthreads-win32/dll-latest/include/ -P pthread-win32
		wget --no-check-certificate -r -np -nH --cut-dirs=3 https://sourceware.org/pub/pthreads-win32/dll-latest/lib/ -P pthread-win32		
		GOTO BUILD_PROTOCOL_OPCUA_C
	)
	
:BUILD_PROTOCOL_OPCUA_C	
	cd %cwd%
	
	IF EXIST "out" (
		rmdir /Q /S out
	)
	mkdir out
	
	cd %cwd%\example
	IF EXIST "out" (
		rmdir /Q /S out
	)
	mkdir out
	
	cd %cwd%
	
	:: build opcua-adapter
	mkdir build
	cd build
	cmake ..
	MSBuild /property:Configuration=Release opcua-adapter.vcxproj
	
	:: build samples
	cd example
	MSBuild /property:Configuration=Release server.vcxproj
	MSBuild /property:Configuration=Release client.vcxproj	
    
	GOTO END

:ERROR_INVALID_PARAM
	ECHO Invalid parameter provided. Please re-run the batch file.
	ECHO e.g. windows.bat --with-dependencies=true
	GOTO END

:END
	cd %cwd%
