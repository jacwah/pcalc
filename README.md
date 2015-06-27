# Pcalc

Pcalc is a simple command line calculator for POSIX systems. It supports three
types of mathematical notation and allows expressions to both be given as
program arguments and in a prompt.

I created this project as an educational exercise for myself, but host it on
Github for anyone's eventual enjoyment.

## Compiling

Run `make` in the project's root directory to compile. The software has
been tested on Linux with Gcc and on OS X with Clang.

`$ make`

## Usage

Evaluate an expression by giving it as arguments to the program

```
$ pcalc 32 / 4
8
```

or by running multiple arguments after each other in the interactive prompt
mode.

```
$ pcalc
Type 'q' or 'quit' to exit
pcalc[i]> 3 * 7
21
pcalc[i]> -1 * ans
-21
pcalc[i]> quit
```

Pcalc supports infix, prefix and postfix notations through command line options.

```
$ pcalc -i 3 - 2
1
$ pcalc -p - 3 2
1
$ pcalc -r 3 2 -
1
```

Run with -h to see full option reference.

`pcalc -h`

## Contact

Pcalc is developed by Jacob Wahlgren and licensed under the MIT License. See
LICESE more information.

You can reach me at jacob.wahlgen at gmail.
