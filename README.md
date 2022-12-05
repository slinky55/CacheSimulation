
# Cache Simulation

A cache simulation written in C++.  You can choose between a fully associative, direct mapped, or set associative cache.

Below are steps for building and running the program


## Building

First, be sure to have cmake installed

**Windows:** [cmake install page](https://cmake.org/install/)

**MacOS:** [cmake install page](https://cmake.org/install/) **OR** `brew install cmake`

**Linux (Ubuntu/Debian):** `apt install cmake -y`



Inside the project directory, run

`cmake -B build -DCMAKE_BUILD_TYPE=Release`

`cmake --build build`
## Running

From the root directory run

`./build/Cache <cache size> <block size> <type> <trace file> [--lru]`

**OR**

`.\build\Cache` if on windows

**Cache size and block size must both be a power of 2**

Types include:

* `-f` - Fully Associative
* `-d` - Direct Mapped
* `-s` - Set Associative

The cache will look for trace files in the traces folder.

`--lru` is an optional flag, use it if you want to use the Least Recently Used replacement strategy.  
Otherwise the cache will use First In First Out (FIFO).

Logging is disabled by default to improve execution time.

To log hit/miss info, in `main.cpp`, before `#include "Cache.hpp"`, write `#define SHOW_DEBUG`