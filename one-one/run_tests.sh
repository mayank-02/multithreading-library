#!/bin/bash

echo -e "\033[34m************************RUNNING ROBUST TEST*************************\033[0m"
echo "./bin/robust_test"
./bin/robust_test
echo ""
echo ""
echo -e "\033[34m***********************RUNNING SPINLOCK TEST************************\033[0m"
echo "./bin/spin_test"
./bin/spin_test
echo "5 seconds"
./bin/spin_test 5
echo ""
echo ""
echo -e "\033[34m***********************RUNNING MUTEX TEST************************\033[0m"
echo "./bin/mutex_test"
./bin/mutex_test
echo "5 seconds"
./bin/mutex_test 5
echo ""
echo ""
echo -e "\033[34m***********************RUNNING CONDVAR TEST************************\033[0m"
echo "./bin/condvar_test"
./bin/condvar_test
echo ""
echo ""
echo -e "\033[34m***********************RUNNING SEMAPHORE TEST************************\033[0m"
echo "./bin/semaphore_test"
./bin/semaphore_test
echo "1 10 10"
./bin/semaphore_test 1 10 10
echo ""
echo ""
echo -e "\033[34m**********************RUNNING PERFORMANCE TEST**********************\033[0m"
echo "./bin/performance_test"
./bin/performance_test
echo ""
echo ""
echo -e "\033[34m**********************RUNNING PHILOSOPHERS TEST**********************\033[0m"
echo "./bin/philosophers"
./bin/philosophers
echo "2000 steps"
./bin/philosophers 2000
echo ""
echo ""