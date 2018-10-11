#******************************************************************
#
# Copyright 2017 Samsung Electronics All Rights Reserved.
#
#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

import os

env = Environment()
architecture = ARGUMENTS.get('TARGET_ARCH')
if architecture == 'linux' :
    env_options = {
        "CC"    : "gcc",
        "CXX"   : "g++",
        "LD"    : "ld",
        "AR"    : "ar",
        "STRIP" : "strip"
    }
    env = Environment(**env_options)
if architecture == 'arm' :
    env_options = {
        "CC"    : "arm-linux-gnueabihf-gcc",
        "CXX"   : "arm-linux-gnueabihf-g++",
        "LD"    : "arm-linux-gnueabihf-ld",
        "AR"    : "arm-linux-gnueabihf-ar",
        "STRIP" : "arm-linux-gnueabihf-strip"
    }
    env = Environment(**env_options)

def do__(self, arg):
	Execute(arg)
	print ("\n")

######################################################################
# Build Open62541 library
######################################################################
SConscript('extlibs/open62541/SConscript')

buildDir= 'build/'
incPath= 'include/'
srcPath= 'src/'
extPath= 'extlibs/'

AddMethod(env, do__)

######################################################################
# Build flags
######################################################################

env.AppendUnique(CCFLAGS=['-fPIC',  '-Wall', '-Werror', '-std=gnu99'])

test = ARGUMENTS.get('TEST')
if ARGUMENTS.get('TEST', False) in [
            'y', 'yes', 'true', 't', '1', 'on', 'all', True
    ]:
    print ("TEST IS ENABLED")
    env.AppendUnique(CCFLAGS=['-fvisibility=default', '-fprofile-arcs', '-ftest-coverage'])
    env.AppendUnique(LINKFLAGS=['-lgcov', '--coverage'])
else:
    print ("TEST IS DISABLED")
    env.AppendUnique(CCFLAGS=['-fvisibility=hidden'])

#delBuildDir = 'rm -rf ' + buildDir
#env.do__(delBuildDir)

createBuildDir = 'mkdir -p ' + buildDir
env.do__(createBuildDir )

open62541LibVersion = '_0.2'

env.AppendUnique(CPPPATH= [
		incPath,
		extPath + '/open62541/open62541' + open62541LibVersion,
		srcPath + '/command',
		srcPath + '/command/browse',
		srcPath + '/node',
		srcPath + '/queue',
		srcPath + '/session',
		srcPath + '/session/discovery',
		srcPath + '/utils'
])

env.AppendUnique(LIBS= ['pthread', 'rt'])

ctt = ARGUMENTS.get('CTT')
if ARGUMENTS.get('CTT', False) in [
            'y', 'yes', 'true', 't', '1', 'on', True
    ]:
	env.AppendUnique(CCFLAGS= ['-DCTT_ENABLED'])

debug = ARGUMENTS.get('DEBUG')
if ARGUMENTS.get('DEBUG', False) in [
            'y', 'yes', 'true', 't', '1', 'on', 'all', True
    ]:
    env.AppendUnique(CPPDEFINES= ['DEBUG'])
    env.AppendUnique(CCFLAGS= ['-g'])

subQueue = ARGUMENTS.get('SUB_QUEUE')
if ARGUMENTS.get('SUB_QUEUE', False) in [
            'y', 'yes', 'true', 't', '1', 'on', 'all', True
    ]:
    env.AppendUnique(CCFLAGS= ['-DENABLE_SUB_QUEUE'])

######################################################################
# Source files and Targets
######################################################################

src = [
		buildDir + srcPath + '/api/opcua_manager.c',
		buildDir + extPath + '/open62541/open62541' + open62541LibVersion + '/open62541.c',
		buildDir + srcPath + '/command/browse/browse.c',
		buildDir + srcPath + '/command/browse/browse_common.c',
		buildDir + srcPath + '/command/browse/browse_view.c',
		buildDir + srcPath + '/command/read.c',
		buildDir + srcPath + '/command/write.c',
		buildDir + srcPath + '/command/method.c',
		buildDir + srcPath + '/command/subscription.c',
		buildDir + srcPath + '/command/cmd_util.c',
		buildDir + srcPath + '/node/edge_node.c',
		buildDir + srcPath + '/queue/caqueueingthread.c',
		buildDir + srcPath + '/queue/cathreadpool_pthreads.c',
		buildDir + srcPath + '/queue/octhread.c',
		buildDir + srcPath + '/queue/uarraylist.c',
		buildDir + srcPath + '/queue/uqueue.c',
		buildDir + srcPath + '/queue/message_dispatcher.c',
		buildDir + srcPath + '/session/edge_opcua_client.c',
		buildDir + srcPath + '/session/edge_opcua_server.c',
		buildDir + srcPath + '/session/discovery/edge_discovery_common.c',
		buildDir + srcPath + '/session/discovery/edge_find_servers.c',
		buildDir + srcPath + '/session/discovery/edge_get_endpoints.c',
		buildDir + srcPath + '/utils/edge_malloc.c',
		buildDir + srcPath + '/utils/edge_utils.c',
		buildDir + srcPath + '/utils/edge_random.c',
		buildDir + srcPath + '/utils/edge_map.c',
		buildDir + srcPath + '/utils/edge_list.c',
		buildDir + srcPath + '/utils/edge_open62541.c'
	]

env.VariantDir(variant_dir = (buildDir + '/' + srcPath), src_dir = 'src', duplicate = 0)
env.VariantDir(variant_dir = (buildDir + '/' + extPath), src_dir = 'extlibs', duplicate = 0)

env.SharedLibrary(target = (buildDir + '/' + 'opcua-adapter'), source = src)
env.StaticLibrary(target = (buildDir + '/' + 'opcua-adapter'), source = src)
Export('env')

Return('env')
