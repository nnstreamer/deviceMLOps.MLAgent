#!/bin/sh
##
## @file run-unittest.sh
## @author Gichan Jang <gichan2.jang@samsung.com>
## @date 08 Dec 2022
## @brief Run unit test on automation tool.
##
setup() {
    echo "setup start"
    export MLAGENT_SOURCE_ROOT_PATH=/usr/bin/ml-test/unittests
    pushd /usr/bin/ml-test/unittests
}

test_main() {
    echo "test_main start"
	testlist=$(find /usr/bin/ml-test/unittests -type f -executable -name "unittest_*")
    for test in ${testlist}; do
	  ${test}
    done
}

teardown() {
    echo "teardown start"
    popd
}

main() {
    setup
    test_main
    teardown
}

main "\$*"
