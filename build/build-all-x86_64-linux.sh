#!/bin/bash

function check_last()
{
    if [ $1 -ne 0 ]; then
        popd > /dev/null
        echo "!!! Error(s) occurred !!!"
        exit $1
    fi
}

#./gen-makefiles-x86_64-linux.sh
#check_last $?

mkargs=$1
echo "Additional make parameters: $mkargs"

echo "Building stdext library..."
pushd ../stdext > /dev/null
make $mkargs -f Makefile-x86_64-linux.Debug
check_last $?
make $mkargs -f Makefile-x86_64-linux.Release
check_last $?

echo
echo "Building stdext library..."
pushd ../stdext-test > /dev/null
make $mkargs -f Makefile-x86_64-linux.Debug
check_last $?
make $mkargs -f Makefile-x86_64-linux.Release
check_last $?

popd > /dev/null
echo "All builds done"

