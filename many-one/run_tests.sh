#!/bin/bash

echo "************************RUNNING ROBUST TEST*************************"
echo "./bin/robust_test"
./bin/robust_test
echo ""
echo ""
echo "*********************RUNNING DETACH STATE TEST**********************"
echo "./bin/detach_state_test"
./bin/detach_state_test
./bin/detach_state_test n
echo ""
echo ""
./bin/detach_state_test
./bin/detach_state_test j
echo ""
echo ""
./bin/detach_state_test
./bin/detach_state_test d
echo ""
echo ""
./bin/detach_state_test
./bin/detach_state_test x
echo ""
echo ""
echo "************************RUNNING MATRIX TEST*************************"
echo "./bin/matrix_test <./data/2.txt"
./bin/matrix_test <./data/2.txt
echo ""
echo ""
echo "***********************RUNNING SPINLOCK TEST************************"
echo "./bin/spin_test 2> /dev/null"
./bin/spin_test 2> /dev/null

echo "For shorter output, make DEBUG = 0 in Makefile"