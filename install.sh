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

USR_LIB_DIR=/usr/local/lib
USR_INC_DIR=/usr/local/include
KEYWORD_OPCUA_C=protocol-opcua-c

# COPY shared library to /usr/local/lib
sudo cp -f ./build/libopcua-adapter.so $USR_LIB_DIR

# COPY header file to /usr/local/include/opcua-c
sudo rm -rf $USR_INC_DIR/$KEYWORD_OPCUA_C
sudo mkdir $USR_INC_DIR/$KEYWORD_OPCUA_C
sudo cp -f ./include/* $USR_INC_DIR/$KEYWORD_OPCUA_C/
