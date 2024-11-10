## About
This is a project that aims to use AI to automatically generate optimal compiler flags based on a project's source code. 


## Flags
These are the list of flags that our AI will choose from when optimizing a program. 
`-O3`: The highest standard optimization level. Enables all optimizations that don't involve a space-speed tradeoff. More aggressive than -O2, may increase code size for slightly faster code.

`-falign-functions=32`: Aligns the start of functions to a 32-byte boundary, which can improve instruction fetch and cache performance.

`-falign-loops=32`: Similarly aligns loop code to 32-byte boundaries for better cache utilization.

`-ffast-math`: Enables aggressive floating-point optimizations that may violate IEEE/ISO standards. Includes reordering operations, assuming no NaNs or ±Inf will occur, treating reciprocal as exact. This may improve performance but reduces floating-point precision and predictability.

`-fipa-pta`: Enables interprocedural pointer analysis during compilation, helping optimize pointer usage across function boundaries.

`-flto`: Enables Link Time Optimization. Performs optimization across all compilation units during linking rather than just within individual files.

`-fmerge-all-constants`: Merges identical constants and strings across the entire program, reducing memory usage.

`-fno-math-errno`: Disables setting errno after math operations, assuming they won't generate errors. Improves performance of math functions.

`-fomit-frame-pointer`: Doesn't keep the frame pointer in a register for functions that don't need it. Frees up a register for other use.

`-fprefetch-loop-arrays`: Automatically insert prefetch instructions for array accesses in loops where beneficial.

`-ftree-vectorize`: Enables automatic vectorization of loops to use SIMD instructions when possible.

`-funroll-loops`: Unrolls loops whose number of iterations can be determined at compile time or runtime.

`-funsafe-math-optimizations`: Similar to -ffast-math, allows optimizations that may change floating-point behavior.

`-fwhole-program`: Assumes that the current compilation unit contains all of the program code, enabling more aggressive optimizations.

`-march=native`: Generates code for the processor type of the compiling machine, using all available instruction set features.

`-mtune=native`: Optimizes code for the processor type of the compiling machine without using instructions not available on other processors.
