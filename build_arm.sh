###############################################################################
# Copyright 2018 Samsung Electronics All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
###############################################################################

#!/bin/bash
echo "Start edge opcua build"

process_cmd_args() {
    while [ "$#" -gt 0  ]; do
            case "$1" in
            --build_mode=*)
                build_mode="${1#*=}";
                echo -e "Build mode = $build_mode"
                shift 1;
                ;;
            --enable_sub_queue=*)
                enable_sub_queue="${1#*=}";
                echo -e "Build mode = $enable_sub_queue"
                shift 1;
                ;;
            -*)
                echo "unknown option: $1" >&2;
                usage; exit 1
                ;;
            *)
                echo "unknown option: $1" >&2;
                usage; exit 1
                ;;
        esac
    done
}
process_cmd_args "$@"

if [ "$enable_sub_queue" == true]
    then
    sub_queue_option="SUB_QUEUE=1"
else
    sub_queue_option=""
fi

if [ "$build_mode" == debug -o "$build_mode" == DEBUG ]
    then
    echo "Build with DEBUG mode"
    scons -c
    scons TARGET_ARCH=arm TEST=1 AUTO_DOWNLOAD_DEP_LIBS=1 DEBUG=1 $sub_queue_option
else
    echo "Build with RELEASE mode"
    scons -c
    scons TARGET_ARCH=arm TEST=1 AUTO_DOWNLOAD_DEP_LIBS=1 $sub_queue_option
fi

echo "End of edge opcua build"
