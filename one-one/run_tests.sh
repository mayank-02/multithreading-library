#!/bin/bash

echo "************************RUNNING ROBUST TEST*************************"
echo "./bin/robust_test"
./bin/robust_test
echo ""
echo ""
/echo "***********************RUNNING SPINLOCK TEST************************"
echo "./bin/spin_test"
./bin/spin_test
echo "5 seconds"
./bin/spin_test 5
echo ""
echo ""
/echo "***********************RUNNING MUTEX TEST************************"
echo "./bin/mutex_test"
./bin/mutex_test
echo "5 seconds"
./bin/mutex_test 5
echo ""
echo ""
/echo "***********************RUNNING CONDVAR TEST************************"
echo "./bin/condvar_test"
./bin/condvar_test
echo ""
echo ""
echo "***********************RUNNING SEMAPHORE TEST************************"
echo "./bin/semaphore_test"
./bin/semaphore_test
echo "1 10 10"
./bin/semaphore_test 1 10 10
echo ""
echo ""
echo "**********************RUNNING PERFORMANCE TEST**********************"
echo "./bin/performance_test"
./bin/performance_test
echo ""
echo ""
echo "**********************RUNNING PHILOSOPHERS TEST**********************"
echo "./bin/philosophers"
./bin/philosophers
echo "2000 steps"
./bin/philosophers 2000
echo ""
echo ""