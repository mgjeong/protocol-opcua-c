###############################################################################
# Copyright 2017 Samsung Electronics All Rights Reserved.
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

#!/bin/bash -xe

set -e
#Colors
RED="\033[0;31m"
GREEN="\033[0;32m"
BLUE="\033[0;34m"
NO_COLOUR="\033[0m"

#Defaults
time_stamp=$(date -u +%Y-%b-%d)
module_name="OPC_UA"
report_format="html"
report_flags="--html --html-details";

#OPC_UA Flags
OPC_UA_BASE="${PWD}"
OPC_UA_TEST="unittests"
OPC_UA_TARGET_OS="linux"
OPC_UA_TARGET_ARCH="$(uname -m)"
USE_TIMESTAMP="yes"
UNITTEST_XML_REPORT="no"


usage() {
    echo "Usage: tools/generate_report.sh <options>"
    echo "Options:"
    echo "      -h / --help                                     :  Display help and exit"
    echo "      -c                                              :  Clean OPC_UA Repository; Should be used to clean existing repository"
    echo "      -f [html|gcov|xml] (default: html)              :  Report Format."
    echo "      --format=[html|gcov|xml] (default: html)        :  Report Format."
    echo "      --module=[OPC_UA] (default: OPC_UA) :  Module for which report needs to be generated."
    echo "      --timestamp=[yes|no] (default: yes)             :  Remove Time Stamp from the report output. If directory exits, overwrites the report to the same directory"
    echo "      --ut_report=[yes|no] (default: yes)             :  The unit test report will be generated in xml format (as gtest only supports xml)."
    echo "      --target_arch=[x86|x86_64] (default: x86)       :  Choose Target Architecture for running test executables."
    echo "Example:"
    echo "  $ cd path/to/opcua/device-opcua-c"
    echo "  $ build_linux.sh"
	echo "  $ cd path/to/opcua/device-opcua-c/unittests"
    echo "  $ prep.sh"
	echo "  $ scons"
    echo "  $ generate_report.sh --format=html --module=OPC_UA --timestamp=yes --ut_report=yes"
}

clean_opc_ua() {
    echo -e "Cleaning ${BLUE}${OPC_UA_BASE}${NO_COLOUR}"
    echo -e "Deleting  ${RED}${OPC_UA_BASE}/.sconsign.dblite${NO_COLOUR}"
    rm -r "${OPC_UA_BASE}/.sconsign.dblite"
    find "${OPC_UA_BASE}" -name "*.memcheck" -delete -o -name "*.gcno" -delete -o -name "*.gcda" -delete -o -name "*.os" -delete -o -name "*.o" -delete
    echo -e "Finished Cleaning ${BLUE}${IOTIVITY_BASE}${NO_COLOUR}"
}

process_cmd_args() {
    while [ "$#" -gt 0  ]; do
        case "$1" in
            -c)
                clean_opc_ua
                shift 1; exit 0
                ;;

            -f)
                report_format="$2";
                if [ "gcov" != ${report_format} -a "html" != ${report_format} -a "xml" != ${report_format} ]; then
                    usage; exit 1;
                fi
                case "$report_format" in
                    "html")
                        report_flags="--html --html-details";
                        ;;
                    "gcov")
                        report_flags="";
                        ;;
                    "xml")
                        report_flags="--xml";
                        ;;
                esac
                shift 2
                ;;

            --format=*)
                report_format="${1#*=}";
                if [ "gcov" != ${report_format} -a "html" != ${report_format} -a "xml" != ${report_format} ]; then
                    usage; exit 1;
                fi
                case "$report_format" in
                    "html")
                        report_flags="--html --html-details";
                        ;;
                    "gcov")
                        report_flags="";
                        ;;
                    "xml")
                        report_flags="--xml --xml-pretty";
                        ;;
                esac
                shift 1
                ;;

            --format)
                echo "$1 requires an argument [gcov|html|xml]" >&2;
                usage;
                exit 1
                ;;

            --module=*)
                module_name="${1#*=}";
                if [ "OPC_UA" != ${module_name}]; then
                    usage; exit 1;
                fi
                shift 1
                ;;

            --timestamp=*)
                USE_TIMESTAMP="${1#*=}";
                if [ "yes" != ${USE_TIMESTAMP} -a "no" != ${USE_TIMESTAMP} ]; then
                    usage; exit 1;
                fi
                shift 1
                ;;
            --timestamp)
                echo "$1 requires an argument [yes|no]" >&2;
                usage;
                exit 1
                ;;

            --ut_report=*)
                UNITTEST_XML_REPORT="${1#*=}";
                if [ "yes" != ${UNITTEST_XML_REPORT} -a "no" != ${UNITTEST_XML_REPORT} ]; then
                    usage; exit 1;
                fi
                shift 1
                ;;
            --ut_report)
                echo "$1 requires an argument [yes|no]" >&2;
                usage;
                exit 1
                ;;

            --target_arch=*)
                IOTIVITY_TARGET_ARCH="${1#*=}";
                if [ "x86" != ${OPC_UA_TARGET_ARCH} -a "x86_64" != ${OPC_UA_TARGET_ARCH} ]; then
                    usage; exit 1;
                fi
                shift 1
                ;;
            --target_arch)
                echo "$1 requires an argument" >&2;
                usage;
                exit 1
                ;;

            -h)
                usage;
                shift 1; exit 0
                ;;
            --help)
                usage;
                shift 1; exit 0
                ;;

            -*)
                echo "unknown option: $1" >&2;
                usage;
                exit 1
                ;;
        esac
    done
}

generate_report_OPC_UA()
{
    # Setting Parameters
    if [ "yes" = ${USE_TIMESTAMP} ]; then
        report_dir="${module_name}_${time_stamp}"
    else
        report_dir="${module_name}"
    fi

    report_file="report.${report_format}"

    test_report_dir="${OPC_UA_TEST}/TestReport/${report_format}/${report_dir}"
    test_report_file="${test_report_dir}/${report_file}"

    rm -rf "${test_report_dir}"
    mkdir -p "${test_report_dir}"

    LD_LIBRARY_PATH="${OPC_UA_BASE}/unittests/libs"

    #Setting Proper Location for UnitTest XML report generation
    unittest_report_dir="UnitTestReport/${report_dir}"
    if [ "yes" = ${UNITTEST_XML_REPORT} ]; then
        rm -rf "${unittest_report_dir}"
        mkdir -p "${unittest_report_dir}"
        UNITTEST_XML_REPORT_FLAG_PREFIX="--gtest_output=xml:${unittest_report_dir}"
    fi

    tests_list=(
                "${OPC_UA_BASE}/unittests/test"
               );

    for exe in ${tests_list[@]}; do
        filename=$(basename -- "${exe}")
        if [ -n "${UNITTEST_XML_REPORT_FLAG_PREFIX}" ]; then
            UNITTEST_XML_REPORT_FLAG="${UNITTEST_XML_REPORT_FLAG_PREFIX}/${filename}.xml"
        fi
        eval "${exe} ${UNITTEST_XML_REPORT_FLAG}"
    done

    unset tests_list

    sleep 1

    echo -e "Generating ${GREEN}${module_name}${NO_COLOUR} Reports"

    # Printing Unit Test Report Location
    if [  "yes" = ${UNITTEST_XML_REPORT} ]; then
        echo -e "${GREEN}${module_name}${NO_COLOUR} UnitTest Report Location: ${BLUE}${unittest_report_dir}${NO_COLOUR}"
    fi

    gcovr -r . \
        -e ".sconf_temp*" \
        -e "extlib.*" \
        ${report_flags} -o ${test_report_file}

    if [  $? -eq 0 ]; then
        echo -e "${GREEN}${module_name}${NO_COLOUR} Coverage Report Location: ${BLUE}${test_report_file}${NO_COLOUR}"
        echo -e "${GREEN}${module_name}${NO_COLOUR} Report Generated ${GREEN}Successfully!${NO_COLOUR}"
    else
        echo -e "${RED}${module_name}${NO_COLOUR} Report Generation ${RED}Failed!${NO_COLOUR}"
    fi
}

process_cmd_args "$@"
module_name="OPC_UA"
generate_report_OPC_UA
