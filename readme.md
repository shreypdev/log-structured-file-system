## Table of contents
* [Author Info](#author-info)
* [Problem Statement](#problem-statement)
* [Setup](#setup)
* [Known Bugs](#known-bugs)
* [References](#references)
* [Worked With](#worked-with)

## Author Info
Name: Shrey Patel
V#: V00900319
Course: CSC 360
Section: A1
Assignment#: 3

## Problem Statement
This is a project on log structured file system. Basic goals of this project are listed below:

   1. Manipulate a simulated disk, and format that disk with version of the Log Structured File System (LSFS).
   2. LSFS to support:
      - reading and writing of files in the root directory.
      - creation of sub-directories.
      - reading and writing of files in any directory.
      - deletion of files and directories.
   3. Making LSFS robust, so that the file system is less likely to be corrupted if the machine it is running on crashes.
   4. Making test files to exercise functionality.

## Setup
To run this project, follow below given steps:

```shell
$ cd ../Project_Directory
$ make
$ make run-test   #For running ready made test cases.
$ make run-shell  #For running interactive shell, to write commands manually and test.
$ make clean
$ clear
```

## Known Bugs
Rmfile function can also be use to remove dir and the way around, Rmdir function can be used to delete file.

## References
* https://www.geeksforgeeks.org/log-structured-file-system-lfs/
* https://linuxconfig.org/bash-scripting-tutorial-for-beginners
* https://www.tutorialspoint.com/c_standard_library/c_function_fopen.htm
* Stack overflow

## Worked With
* Amee Gorana (V#: V00882943)