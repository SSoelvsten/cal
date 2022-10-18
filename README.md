# CAL

[![Release](https://img.shields.io/github/v/release/ssoelvsten/cal)](https://github.com/SSoelvsten/cal/releases)
&nbsp;
[![Documentation](https://img.shields.io/website?down_message=not%20available&label=docs&up_message=available&url=https%3A%2F%2Fssoelvsten.github.io%2Fcal)](https://ssoelvsten.github.io/cal)
&nbsp;
[![test](https://img.shields.io/github/workflow/status/ssoelvsten/cal/test/main?label=test&logo=github&logoColor=white)](https://github.com/SSoelvsten/cal/actions/workflows/test.yml)
&nbsp;

CAL is a BDD package [[Bryant86](#references)] that uses breadt-first
algorithms [[Ochi93](#references), [Ashar94](#references),
[Sanghavi96](#references)] to exploit a locality of BDD nodes on disk.
This allows one to obtain a high performance when manipulating Binary
Decision Diagrams, even when they outgrow he memory limit of the given
machine.

This project was developed in the late 90's at
[EECS at UC Berkeley](https://eecs.berkeley.edu/).

**Table of Contents**

- [Documentation](#documentation)
- [Usage](#usage)
- [License](#license)
- [References](#references)

## Documentation

An overview of all functions can be found as simple text file in
[*calDoc.txt*](calDoc) or as an easier to navigate HTML page in
[*docs/index.html*](docs/index.html).

## Build

CAL can be built with CMake with the following set of commands.

```bash
mkdir build && cd build
cmake ..
make cal
```

### Page Size

By default CAL is set to use a page size of *4096*. To get the page size of your
machine, please compile and run the following small C program and set the CMake
option `CAL_PAGE_SIZE` to the value printed. One should also set
`CAL_LG_PAGE_SIZE` to the base-2 logarithm of `CAL_PAGE_SIZE`.

```c
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

main() {
  FILE * f = fopen( "conftestval", "w" );
  if ( f == NULL ) exit(1);
  fprintf( f, "%d\n", (int) getpagesize() );
  exit(0);
}
```

## Tests

The unit tests in *tests/* can are also built with CMake

```bash
mkdir build && cd build
cmake ..
make test_performance test_reorder test_unit
```

Then run the *build/test/test_performance*, *build/test/test_reorder*, and
*build/test/test_unit* executables.

## License

This software is provided as-is and with permission to *copy*,
*modify*, and *distribute*. See [LICENSE](LICENSE) for more details.

## References

- [[Ashar94](https://ieeexplore.ieee.org/document/629886)]
  Pranav Ashar and Matthew Cheong. “_Efficient breadth-first manipulation of
  binary decision diagrams_”. In: _Proceedings of the
  1994 IEEE/ACM International Conference on Computer-Aided Design_. (1994)

- [[Bryant86](https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=1676819)]
  Randal E. Bryant. “_Graph-Based Algorithms for Boolean Function Manipulation_”.
  In: _IEEE Transactions on Computers_. (1986)

- [[Ochi93](https://www.computer.org/csdl/proceedings-article/iccad/1993/00580030/12OmNAXglQz)]
  Hiroyuki Ochi, Koichi Yasuoka, and Shuzo Yajima. “_Breadth-first manipulation
  of very large binary-decision diagrams_”. In: _Proceedings of 1993
  International Conference on Computer Aided Design (ICCAD),_ (1993)

- [[Sanghavi96](https://link.springer.com/article/10.1007/s002360050083)
  Jagesh V. Sanghavi, Rajeev K. Ranjan, Robert K. Brayton, and Alberto
  Sangiovanni-Vincentelli. “_High performance BDD package by exploiting
  memory hierarchy_”. In: _Proceedings of the 33rd Annual Design
  Automation Conference_ (1996)

- [[Ranjan97](https://ieeexplore.ieee.org/abstract/document/628893)]
  Rajeev K. Ranjan, Wilsin Gosti, Robert K. Brayton, and Alberto
  Sangiovanni-Vincenteili. “_Dynamic reordering in a breadth-first
  manipulation based BDD package: challenges and solutions_”. In:
  _Proceedings International Conference on Computer Design VLSI in
  Computers and Processors_ (1997)
