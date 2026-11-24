# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- initial module skeleton generated from ModuleTemplate

- `GeneralizedAssignmentBlock`, the Generalized Assignment Problem as one
  minimization `BinaryKnapsackBlock` for each agent, linked by the
  assignment constraints, with the instances of types A, B, C and D of the
  OR-Library and of types D and E of Yagiura in the GitLab Package Registry,
  which CMake downloads into `data/txt` when the tests need them

- `get_R3_Block()` gives an identical copy of the
  `GeneralizedAssignmentBlock`, whose sub-Block are the copies of its own,
  and `map_back_solution()` / `map_forward_solution()` write the solution of
  one into the other

### Changed

- the makefiles of a new module give its shared library by default on Linux
  and on macOS, as CMake does, and its objects with `SMSppSTATIC` [see
  `lib/makefile-shared` of SMS++]

