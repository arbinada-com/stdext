#!/bin/bash

function check_last()
{
    if [ $1 -ne 0 ]; then
        popd > /dev/null
        echo "!!! Error(s) occurred !!!"
        exit $1
    fi
}

echo "Generating stdext library makefiles..."
pushd ../stdext > /dev/null
qmake stdext.pro -spec linux-clang CONFIG+=debug -nocache -nodepend
check_last $?
qmake stdext.pro -spec linux-clang -nocache -nodepend
check_last $?
echo "Done"

echo
echo "Generating stdext tests makefiles..."
cd ../stdext-test
qmake stdext-test.pro -spec linux-clang CONFIG+=debug -nocache -nodepend
check_last $?
qmake stdext-test.pro -spec linux-clang -nocache -nodepend
check_last $?
echo "Done"

echo "All makefiles done"

popd > /dev/null
exit 0
