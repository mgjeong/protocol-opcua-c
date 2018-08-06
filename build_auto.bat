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
		GOTO BUILD_PROTOCOL_OPCUA_C
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

		cmake .. -G "Visual Studio 14 2015" -DCMAKE_BUILD_TYPE=Release -DUA_ENABLE_AMALGAMATION=ON -DUA_ENABLE_ENCRYPTION=OFF

		MSBuild open62541.vcxproj

		mkdir %open62541_dir%\open62541_%versionName%
		copy open62541.h %open62541_dir%\open62541_%versionName%
		copy open62541.c %open62541_dir%\open62541_%versionName%
		
		:: remove the directory
		cd ..\..\
		rmdir /Q /S open62541_%versionName%
		cd %cwd%
		
		GOTO BUILD_PROTOCOL_OPCUA_C	
	)	
	
:BUILD_PROTOCOL_OPCUA_C	
	cd %cwd%
    call scons TARGET_ARCH=windows AUTO_DOWNLOAD_DEP_LIBS=1
    copy %cwd%\extlibs\pthread-win32\dll\x64\pthreadVC2.dll %cwd%\example\out\pthreadVC2.dll
    GOTO END

:ERROR_INVALID_PARAM
    ECHO Invalid parameter provided. Please re-run the batch file.
    ECHO e.g. windows.bat --with-dependencies=true
    GOTO END

:END
