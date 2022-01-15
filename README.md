# CAL
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

## Usage

Run the *configure* script to create a *Makefile* with the desired
targets.

```bash
./configure
```

The *Makefile* provides the following targets for your to run

| Target       | Function                        |
|--------------|---------------------------------|
| `all`        | Build *libcal.a*                |
| `check`      | Run all unit tests              |
| `check-code` | Run a linter on the source code |

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
