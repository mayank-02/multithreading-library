mthread
=====

A lightweight and fast C library based on one-one and many-one model for threading.

## Contents
- [Building](#building)
- [Using mthread in your project](#using-mthread-in-your-project)
- [Usage and examples](#usage-and-examples)
- [API documentation](#api-documentation)
- [Running tests](#running-tests)
- [Time and space complexity](#time-and-space-complexity)
- [Development and contributing](#development-and-contributing)
- [Acknowledgements](#acknowledgements)


## Building

mthread uses make to build libraries (static and shared) and binaries (for tests).
Execute following commands to build mthread using make:

`make`

This will create binaries in `bin/` directory and libraries (static and shared) in current directory.

Optionally, you can run `sudo make install` to install mthread library on your machine (on Linux, this will usually install it to `usr/local/lib` and `usr/local/include`).

## Using mthread in your project

You can use mthread in you project by either directly copying header and source files from [one-one/](one-one/), or by linking mthread library (see [Building](#building) for instructions how to build mthread libraries).
In any case, only thing that you have to do in your source files is to include `mthread.h`.

To get you started quickly, let's take a look at a few ways to get simple Hello World project working.

Our Hello World project has just one source file, `example.c` file, and it looks like this:
```c
#include <stdio.h>
#include "mthread.h"

void worker(void) {
    printf("Hello World!\n");
    mthread_exit(NULL);
}

int main() {
    mthread_t tid;
	mthread_init();
	mthread_create(&tid, NULL, worker, NULL);
	mthread_join(tid, NULL);
	return 0;
}
```

### Approach #1: Copying mthread header file and static library

Instead of copying mthread source files, you could copy static library or a shared object (check [Building](#building) on how to create static library / shared object). We also need to copy mthread header files. We get following project structure:
In case of a static library:
```
example.c       -> your program
mthread.h       -> copied from mthread
libmthread.a    -> copied from mthread
```

In case of shared object:
```
example.c       -> your program
mthread.h       -> copied from mthread
libmthread.so   -> copied from mthread
```

Now you can compile it with `gcc example.c -o example -L. -llibmthread`.

### Approach #2: Install mthread library on machine (TODO)

Alternatively, you could avoid copying any mthread files and instead install libraries by running `sudo make install` (check [Building](#building)). Now, all you have to do to compile your project is `gcc example.c -o example -llibmthread`.
If you get error message like `cannot open shared object file: No such file or directory`, make sure that your linker includes path where mthread was installed.

## Usage and examples
To know more about how to use mthread library, check out the various tests written in the `test` directory of each model.

## API documentation

+ Types are named **mthread\_[type]\_t** (examples: mthread_t, mthread_cond_t, etc.)
+ Functions are called **mthread\_[type]\_[action]** with a few exceptions that are mthread_[action] and pertain to the API in whole and not a specific type.
+ Constants are named **MTHREAD\_[NAME]**

The mthreads API is inherently simple. Not in the sense that it makes multi-threaded (MT) programming a breeze (I doubt this is possible), but in the sense that it provides everything that's needed to write MT programs, and only that.

To generate the latest API documentation yourself from the source, you need to have [doxygen](www.doxygen.org) installed.
Position yourself in the root directory of the model you are interested in.
Then run `make docs`. This will output a `/docs/html` and `/docs/latex` folder.
Then open `docs/html/index.html` file with your favorite browser.

Alternatively, you can directly check [mthread.h](one-one/include/mthread.h) for one-one and [mthread.h](many-one/include/mthread.h) for many-one

## Running tests

Check [Building](#building) to see how to build binaries.
To run each test, just run `./run_tests` from the root directory of both models. 
Default values are used for tests requiring command line arguments.

## Time and space complexity

The library maintains a queue for internal book keeping. Thus, the functions have different time complexities. They have a best case time complexity of `O(1)` and worst case time complexity of `O(N)`, N being the number of threads spawned. Space complexity is `O(N)`.

## Implementation Details

+ To know the implementation details of one-one threading model, please check out it's [README](one-one/README.md)

+ To know the implementation details of many-one threading model, please check out it's [README](many-one/README.md)

## Development and contributing

Feel free to send pull requests and raise issues.

## Acknowledgements

Abhijit (@abhijit13) - Mentoring and guidance throughout the project.