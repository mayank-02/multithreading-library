#!/bin/bash

echo -e "\033[34m************************RUNNING ROBUST TEST*************************\033[0m"
echo "./bin/robust_test"
./bin/robust_test
echo ""
echo ""
echo -e "\033[34m*********************RUNNING DETACH STATE TEST**********************\033[0m"
echo "./bin/detach_state_test"
./bin/detach_state_test
echo -e "\033[33mCASE: JOINABLE threads, but don't wait\033[0m"
./bin/detach_state_test n
echo ""
echo ""
echo -e "\033[33mCASE: JOINABLE threads, with mthread_join()\033[0m"
./bin/detach_state_test j
echo ""
echo ""
echo -e "\033[33mCASE: DETACHED threads, and don't wait\033[0m"
./bin/detach_state_test d
echo ""
echo ""
echo -e "\033[33mCASE: DETACHED threads, and wait 3 seconds\033[0m"
./bin/detach_state_test x
echo ""
echo ""
echo -e "\033[34m************************RUNNING MATRIX TEST*************************\033[0m"
echo "./bin/matrix_test <./data/2.txt"
./bin/matrix_test <./data/2.txt
echo ""
echo ""
echo -e "\033[34m***********************RUNNING SPINLOCK TEST************************\033[0m"
echo "./bin/spin_test 2> /dev/null"
./bin/spin_test 2> /dev/null
echo ""
echo -e "\033[31mTo disable debug statements, set DEBUG = 0 in Makefile"