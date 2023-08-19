\page page__getting_started Getting started

[TOC]

Dependencies
=========================

One needs a C++ compiler of ones choice that supports both the C and C++ *11*
standard, such as the *GNU*, *Clang*, and *MSVC* compilers. The entire project
is built with *CMake*.

Furthermore, to create the documentation files, you need the *Doxygen* tool.

To install all of the above then run the respective command below.

| Operating System | Shell command                       |
|------------------|-------------------------------------|
| Ubuntu 22+       | `apt install cmake g++     doxygen` |
| Fedora 36+       | `dnf install cmake gcc-c++ doxygen` |

Building with CMake
=========================

To get started with *CAL*, you need to place the repository somewhere on your
machine. The simplest way to do so is to add it as a submodule inside of your
Git repository.

```bash
git submodule add https://github.com/SSoelvsten/cal external/cal
git submodule update --init --recursive
```

Then include the following line in your project's *CMakeLists.txt*.

```cmake
add_subdirectory (external/cal cal)
```

Finally, every single executable target is linked to *CAL* in the
*CMakeLists.txt* file with the following lines.

```cmake
add_executable(<target> <source>)
target_link_libraries(<target> cal)
```

Usage
=========================

After having linked the C++ source file with *CAL* as described above, one can
either use CAL through a C and a C++ API.

C API
-------------------------

The original C API from the 90s can be found in the `<cal.h>` header file.

```c
#include <cal.h>

int main()
{
  Cal_BddManager bddManager  = Cal_BddManagerInit();

  // Do your stuff here...

  Cal_BddManagerQuit(bddManager);
}
```

See the \ref module__c module for all functions.

C++ API
-------------------------

The C++ header-only interface provides a zero-overhead wrapper for CAL in
`<calObj.hh>`. This is in many ways very similar to the C++ API of CUDD.

```cpp
#include <adiar/adiar.h>

int main()
{
  Cal();

  // do your stuff here...
}
```

See the \ref module__cpp module for all functions

\warning All `BDD` objects have to be destructed (i.e. go out of a scope) before
their associated `Cal` object is destructed itself.
