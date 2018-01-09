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

def do__(self, arg):
	Execute(arg)
	print "\n"

buildDir= 'build/'
incPath= 'include/'
srcPath= 'src/'
extPath= 'extlib/'

AddMethod(env, do__)

######################################################################
# Build flags
######################################################################

env.AppendUnique(CCFLAGS=['-fPIC',  '-Wall', '-Werror', '-std=gnu99'])

test = ARGUMENTS.get('TEST')
if ARGUMENTS.get('TEST', False) in [
            'y', 'yes', 'true', 't', '1', 'on', 'all', True
    ]:
    print "TEST IS ENABLED"
    env.AppendUnique(CCFLAGS=['-fvisibility=default', '-fprofile-arcs', '-ftest-coverage'])
    env.AppendUnique(LINKFLAGS=['-lgcov', '--coverage'])
else:
    print "TEST IS DISABLED"
    env.AppendUnique(CCFLAGS=['-fvisibility=hidden'])

#delBuildDir = 'rm -rf ' + buildDir
#env.do__(delBuildDir)

createBuildDir = 'mkdir -p ' + buildDir
env.do__(createBuildDir )

env.AppendUnique(CPPPATH= [
		incPath,
		extPath,
		srcPath + '/command',
		srcPath + '/node',
		#srcPath + '/queue',
		srcPath + '/session',
		srcPath + '/utils'
])

env.AppendUnique(LIBS= ['pthread', 'rt'])

######################################################################
# Source files and Targets
######################################################################

src = [
		buildDir + srcPath + '/api/opcua_manager.c',
		buildDir + extPath + 'open62541.c',
		buildDir + srcPath + '/command/browse.c',
		buildDir + srcPath + '/command/read.c',
		buildDir + srcPath + '/command/write.c',
		buildDir + srcPath + '/command/method.c',
		buildDir + srcPath + '/command/subscription.c',
		buildDir + srcPath + '/node/edge_node.c',
		#buildDir + srcPath + '/queue/message_dispatcher.c',
		#buildDir + srcPath + '/queue/queue.c',
		buildDir + srcPath + '/session/edge_opcua_client.c',
		buildDir + srcPath + '/session/edge_opcua_server.c',
		buildDir + srcPath + '/utils/edge_utils.c',	
	]

env.VariantDir(variant_dir = (buildDir + '/' + srcPath), src_dir = 'src', duplicate = 0)
env.VariantDir(variant_dir = (buildDir + '/' + extPath), src_dir = 'extlib', duplicate = 0)

env.SharedLibrary(target = (buildDir + '/' + 'opcua-adapter'), source = src)

Export('env')

Return('env')
