#!/bin/bash
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

set -e

package="gtest"
packageRevision="1.8.0"
packagezip="release-${packageRevision}.tar.gz"
packageUrl='https://github.com/google/googletest/archive/${packagezip}'
packageArchive=$(basename -- "${packageUrl}")
packageDir="googletest-release-${packageRevision}"
packageLibDir="${packageDir}/googlemock/gtest"
packageSourceFile="${packageDir}/CMakeLists.txt"

do_()
{
    set +f
    printf "%s \n" "Executing: \"$@\""
    eval "$@"
}


main_()
{
    echo "# Creating gtest Directory "
    do_ "mkdir -p ${package} && cd ${package}"

    echo "# Checking for gtest presence:"
    if [ ! -e "${packageSourceFile}" ] ; then
        whereis -b wget 2>/dev/null
        whereis -b unzip 2>/dev/null
        do_ "wget -nc -O ${packageArchive} ${packageUrl} && tar xf ${packageArchive}"

		echo "# Removing ${packagezip} File"
		do_ "rm ${packagezip}"

		do_ "cd ${packageDir}"

		echo "Info: Building gtest library"
		do_ "cmake -DBUILD_SHARED_LIBS=ON . && make"
		
		do_ "cd .."
	
	else
		echo "WARNING: gtest Already exists, Not downloading now..."
    fi
}


main_ "$@"
