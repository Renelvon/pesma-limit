pesma-limit
===========

Parallel Equitable Stable Marriage with Limited Preference Lists, as presented on ALGOCLOUD 2016

See: [ESMA](https://github.com/equitable-stable-matching/esma)

Building source code
--------------------
A C++11 compiler is required, with OpenMP available. Tested under LinuxMint 17 with g++ 4.8.4.

    $ make

Running 
-------
Example run:

    $ ./pesma-limit4 --N 1000 --n 1000 --K 400
