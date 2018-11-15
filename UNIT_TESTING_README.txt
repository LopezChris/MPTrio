CGREEN UNIT TESTING INTRODUCTION
------------

This readme explains how to get your environment set up to start running and writing your own unit tests.
Unfortunately, the Cgreen Unit Testing framework at the time this was worked on has only tested on MAC, Linux, and Cygwin, but not Windows (without Cygwin). The following list of instructions will complete your setup to perform unit tests via Cygwin (on Windows).


WHY CGREEN?
------------

Cgreen is a modern unit test and mocking framework for C and C++. It's fast to build, the code is clean, and the API is fluent and expressive with the same modern syntax across C and C++. Each test is isolated, which is great because it prevents intermittent failures and cross-test dependencies. The Cgreen frame provides built-in mocking for C and is compatible with other C++ mocking libraries.


REQUIREMENTS
------------

The Cgreen Unit Test requires the following: (search the exact name and include the install option)
  * Cygwin x86 (https://cygwin.com/setup-x86.exe)
    Install the following packages in Cygwin:
      * 5.4.0-1 gcc-core: GNU Compiler Collection
      * 5.4.0-1 gcc-g++: GNU Compiler Collection
      * 1.0.3-1 gccmakedep
      * 5.4.0-1 libgcc1: GCC C runtime library
      * 5.4.0-1 mingw64-i686-gcc-core
      * 5.4.0-1 mingw64-i686-gcc-g++
      * 5.4.0-1 mingw64-x86_64-gcc-core
      * 4.2.1-1 make: The GNU version of the 'make' utility
      * 3.6.2-1 cmake: Cross-platform makefile generation system


Run the unit-test:
------------
Open the Cygwin command prompt and add CGreen path:
export PATH=$PATH:/cygdrive/c/SJSU_Dev/cgreen

After your path is setup, you can browse to the example unit-test and run it:
cd /cygdrive/c/SJSU_Dev/projects/cgreen_test
make           	:  Builds the unit test project
make clean      :  Cleans the project folder and removes all *.exe and *.o files
./test_all.exe 	:  Executes all of the built unit tests 


SAMPLE PASS AND FAIL TESTS
------------

Navigate and open projects/cgreen_test/code/test_main.cpp.
Change the #define SHOW_SAMPLE_PASS_AND_FAIL_TESTS to 1, then build and execute to see the output of all of the sample tests. There are many different examples of all types of asserts you can use to test your own code. Once you're finished then revert the #define back to 0.


BUILDING YOUR OWN UNIT TESTS
------------

The project is set up so that if you want to create a new unit test file then all you have to do is:
  * Include the cgreen.h header file
  * Add "using namespace cgreen;" if you're testing C++ code.
  * Create a TestSuite function pointer, which you will add to the main TestSuite at main() in test_main.cpp.
  * Create the .o object file in the Makefile.
Of course, it may be easier to just copy and paste what exists and change accordingly to your new test names.


MOCKING FUNCTIONS
------------

Take a look here for a great explanation and walk-through on mocking functions in Cgreen, as well as other Cgreen unit test info:
https://cgreen-devs.github.io


SOURCES & EXTRA REFERENCES:
------------
  * http://cgreen.sourceforge.net/
  * https://github.com/cgreen-devs/cgreen