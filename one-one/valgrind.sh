#!/bin/zsh

if [ $# -eq 0 ]; then
    echo "Usage ./valgrind.sh <test-name>"
else
    valgrind --leak-check=full \
    --track-origins=yes \
    --show-leak-kinds=all \
    --verbose \
    --log-file=valgrind-out.txt \
    ./bin/$1 "${@:2}"
fi

#   --leak-check=full    : each individual leak will be shown in detail
#   --track-origins=yes  : tracks the origins of uninitialized values, 
#                          which could be very useful for memory errors
#   --show-leak-kinds=all: show all of "definite, indirect, possible, 
#                          reachable" leak kinds in the "full" report
#   --verbose            : tells you about unusual behavior of your program
#   --log-file           : write to a file