#!/bin/bash

function check_last()
{
    if [ $1 -ne 0 ]; then
        popd > /dev/null
        echo "!!! Error(s) occurred !!!"
        exit $1
    fi
}

compiler_spec=linux-g++
for arg; do
    if [ "$arg" = "clang" ]; then
        compiler_spec=linux-clang
    fi
done
echo "Compiler spec: $compiler_spec"

echo "Generating stdext library makefiles..."
pushd ../stdext > /dev/null
qmake stdext.pro -spec $compiler_spec "CONFIG+=debug stdext_gen_makefile" -nocache -nodepend
check_last $?
qmake stdext.pro -spec $compiler_spec "CONFIG+=stdext_gen_makefile" -nocache -nodepend
check_last $?
echo "Done"

echo
echo "Generating stdext tests makefiles..."
cd ../stdext-test
qmake stdext-test.pro -spec $compiler_spec "CONFIG+=debug stdext_gen_makefile" -nocache -nodepend
check_last $?
qmake stdext-test.pro -spec $compiler_spec "CONFIG+=stdext_gen_makefile" -nocache -nodepend
check_last $?
echo "Done"

echo "All makefiles done"

popd > /dev/null
exit 0
