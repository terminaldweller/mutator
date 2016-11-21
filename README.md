# mutator

A C code mutator,Misra-C 2004 checker and when possible, a Misra-C implementer.<br/>
**mutator-lvl0.cpp** contains the Misra-C rules to check.<br/>
**mutator.cpp** contains the mutators which are not copiled for the time being since im working on Misra-C only for the time being, along with some Misra-C implementers.<br/>
**mutator-lvl2.cpp** contains some other Misra-C implementers. Rewriting the code in multiple stages allows for more simplistic rewrites and is also a check to see whether the output is actually buildable.<br/>
**mutator.sh** is the UI, which is supposed to work like just any other nix UI(option-wise).<br/>
The **utility** folder holds the C source and headers that are necessary to run the instruented code(currently unused).<br/>
**mutator-aux.cpp.h** hold the auxillary functions that most modules will need.<br/>
Well there is the **makefile**.<br/>
The **test** folder holds the TDD tests.<br/>

To build the project, you need to have the LLVM libraries 4.0 to avoid any unforseen results. The project ccan not be built with LLVM 3.8 or lower, but I havent tested LLVM 3.9. Just run **make** and you're good to go. Running make will build three executables which can be used independently or with **mutator.sh**(use -h to see a list of options.)<br/>
**mutator-lvl0** will run the Misra-C:2004 checks.<br/>
**mutator** will run the level-1 Misra-C:2004 implementers.<br/>
**mutator-lvl2** will run the level-2 Misra-C:2004 implementers.<br/>
Currently, the mutation-only features(mutation for the sake of mutation, technically implementing Misra-C is also a form of mutation) are turned off in **mutator** and **mutator-lvl2**.<br/>
If your code needs a compilation database for clang to understand it and you don't have one,you can use [Bear](https://github.com/rizsotto/Bear).<br/>
