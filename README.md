# FP

(c)2009 Felix L. Winkelmann

This project and all code included is in the public domain.

## Introduction

This is a compiler and runtime-system for a dialect of John Backus' "FP" language. The compiler ("fpc") is written in FP and does fully self-compile. FP code is translated to C and can then be compiled to a binary executable. The compiler processes a single file including any support libraries - it is a "whole-program" compiler and statically resolves all functions. The generated C code uses `gcc`-specific extensions.

## Requirements

The system has so far been tested on the following platforms:

* Linux/x86
* Linux/x86-64
* Mac OS X 10.4/PowerPC
* Windows Vista

You will need

* `gcc`, the GNU C Compiler

## Building

To build the compiler, enter:

```
sh bootstrap.sh
```

on UNIX-like systems or

```
bootstrap.bat
```

on Windows.

This will compile a pretranslated version of the compiler (`fpcboot.c`) and perform a 3-stage bootstrap, comparing the output of the stage 2 and 3 compilers to ensure everything works correctly. As a result, the `fpc` executable is generated which is the final compiler to be used. You can now use the compiler from the build directory. For proper installation, see below.

## Installation

If you want to install the compiler in a particular location to be able to use from anywhere in your system, it has to be configured to use the correct include files and prelude. The settings where the support files should be installed are specified in a source file of the compiler called `configuration.fp`. Edit this file and modify the paths like this:

```
% configuration.fp

configuration = {
  cflags = ~"-I<PREFIX>/include"    % i.e. "/usr/local/include"
  prelude = ~"<PREFIX>/prelude.fp"  % i.e. "/usr/local/share/fp/prelude.fp"
  cc = ~"gcc"                       % change, if not in PATH
  limit = ~5
}
```

Then rebuild the compiler with (omit the leading "./" on Windows systems):

```
./fpcboot -c fpc.fp -prelude prelude.fp
```

Now copy all necessary files into the proper places, on UNIX (changing "<PREFIX>" to the right installation prefix):

```
install -m755 fpc <PREFIX>/bin
cp -r fp <PREFIX>/include
mkdir -p <PREFIX>/share/fp
cp prelude.fp <PREFIX>/share/fp
```

## Syntax highlighting for Emacs

A very simple emacs-mode for FP is available in "misc/fp-mode.el". To install it, add the following code to your emacs startup file (usually "`~/.emacs`"):

```
(add-to-list 'load-path "path/to/fp-mode.el")
(setq auto-mode-alist
  (append 
   '(("\\.fp\\'" . fp-mode))
   auto-mode-alist) )
(require 'fp-mode)
```

## Documentation

Please see ["manual.txt"](manual.txt) for a basic description of the language and compiler.

## Acknowledgements

Thanks to John Nowak for encouragement and many helpful suggestions.

## Contact

If you have any questions or suggestions, please don't hesitate to contact <felix@call-with-current-continuation.org>
