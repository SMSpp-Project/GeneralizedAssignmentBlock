# GeneralizedAssignmentBlock

[![Pipeline](https://gitlab.com/smspp/generalizedassignmentblock/badges/develop/pipeline.svg)](https://gitlab.com/smspp/generalizedassignmentblock/-/pipelines?ref=develop)
[![Release](https://gitlab.com/smspp/generalizedassignmentblock/-/badges/release.svg)](https://gitlab.com/smspp/generalizedassignmentblock/-/releases)
[![Documentation](https://img.shields.io/badge/docs-Doxygen-blue)](https://smspp.gitlab.io/smspp-project/)
[![C++ Standard](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![License: LGPL v3](https://img.shields.io/badge/License-LGPL_v3-blue.svg)](https://www.gnu.org/licenses/lgpl-3.0)

`GeneralizedAssignmentBlock` is a SMS++ `:Block` for the Generalized
Assignment Problem (GAP), in which N jobs are to be assigned to M agents:
assigning job j to agent i costs C[ i ][ j ] and uses A[ i ][ j ] of the
resource of agent i, whose capacity is B[ i ], each job goes to exactly one
agent and the total cost is minimized (see, e.g., D.G. Cattrysse, L.N. Van
Wassenhove, "A survey of algorithms for the generalized assignment
problem", European Journal of Operational Research 60(3), 260-272, 1992).

The problem is represented as M sub-`Block`, one minimization
`BinaryKnapsackBlock` for each agent, whose items are the jobs; the
`GeneralizedAssignmentBlock` itself only holds the constraints that link
them, i.e., each job to exactly one agent. Hence, relaxing these
constraints in a Lagrangian way leaves M independent 0-1 knapsack
problems, which any `Solver` of `BinaryKnapsackBlock` can solve, while a
`:MILPSolver` solves the whole problem out of its abstract representation.

The data can be read from netCDF (`deserialize()`), from a text file in the
format of one instance of the OR-Library (the numbers of agents and jobs,
the costs, the resources and the capacities) or from memory (`load()`).


## Getting started

These instructions will let you build the `GeneralizedAssignmentBlock` module on
your system.

The channels that distribute SMS++ are listed in its [installation
guide](https://gitlab.com/smspp/smspp-project/-/wikis/Installing-SMS++); what
follows is about building the module yourself.

### Requirements

- The [SMS++ core library](https://gitlab.com/smspp/smspp) and its
  requirements.

- [BinaryKnapsackBlock](https://gitlab.com/smspp/binaryknapsackblock).

### Build and install with CMake

Configure and build the library with:

```sh
mkdir build
cd build
cmake ..
cmake --build .
```

The library has the same configuration options of
[SMS++](https://gitlab.com/smspp/smspp-project/-/wikis/Customize-the-configuration).

Optionally, install the library in the system with:

```sh
cmake --install .
```

### Usage with CMake

After the library is built, you can use it in your CMake project with:

```cmake
find_package(GeneralizedAssignmentBlock)
target_link_libraries(<my_target> SMS++::GeneralizedAssignmentBlock)
```

### Build and install with makefiles

Carefully hand-crafted makefiles have also been developed for those unwilling
to use CMake. Makefiles build the executable in-source (in the same directory
tree where the code is) as opposed to out-of-source (in the copy of the
directory tree constructed in the build/ folder) and therefore it is more
convenient when having to recompile often, such as when developing/debugging
a new module, as opposed to the compile-and-forget usage envisioned by CMake.

Each executable using `GeneralizedAssignmentBlock` has to include a "main makefile" of
the module, which typically is either [makefile-c](makefile-c) including all
necessary libraries comprised the "core SMS++" one, or
[makefile-s](makefile-s) including all necessary libraries but not the "core
SMS++" one (for the common case in which this is used together with other
modules that already include them). These in turn recursively include all the
required other makefiles, hence one should only need to edit the "main
makefile" for compilation type (C++ compiler and its options) and it all
should be good to go. In case some of the external libraries are not at their
default location, it should only be necessary to create the
`../extlib/makefile-paths` out of the `extlib/makefile-default-paths-*` for
your OS `*` and edit the relevant bits (commenting out all the rest).

Check the [SMS++ installation wiki](https://gitlab.com/smspp/smspp-project/-/wikis/Customize-the-configuration#location-of-required-libraries)
for further details.


## Data

We provide a set of instances that is used by the testers of the [tests
repo](https://gitlab.com/smspp/tests). It is not included in the repo: it is
automatically downloaded by CMake when the tests need it, but if you are not
using CMake you need to do it by hand, via

```sh
cd data
wget https://gitlab.com/api/v4/projects/89369533/packages/generic/txt/2026-11-24/txt.tgz
tar xzvf txt.tgz
```

This builds the folder `data/txt`, with 48 instances in the format of the
OR-Library: those of types A, B, C and D of the OR-Library, with 5, 10 and
20 agents and 100 and 200 jobs, and those of types D and E of Yagiura, with
up to 80 agents and 1600 jobs.


## Getting help

If you need support, you want to submit bugs or propose a new feature, you
can [open a new issue](https://gitlab.com/smspp/generalizedassignmentblock/-/issues/new).


## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of
conduct, and the process for submitting merge requests to us.


## Authors

### Current Lead Authors

- **Antonio Frangioni**  
  Dipartimento di Informatica  
  Università di Pisa

### Contributors


## License

This code is provided free of charge under the [GNU Lesser General Public
License version 3.0](https://opensource.org/licenses/lgpl-3.0.html) -
see the [LICENSE](LICENSE) file for details.


## Disclaimer

The code is currently provided free of charge under an open-source license.
As such, it is provided "*as is*", without any explicit or implicit warranty
that it will properly behave or it will suit your needs. The Authors of
the code cannot be considered liable, either directly or indirectly, for
any damage or loss that anybody could suffer for having used it. More
details about the non-warranty attached to this code are available in the
license description file.
