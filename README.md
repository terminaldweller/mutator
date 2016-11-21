# mutator

a C code mutator,Misra-C 2004 checker and when possible, a Misra-C implementer.(<br/>)
**mutator-lvl0.cpp** contains the Misra-C rules to check.(<br/>)
**mutator.cpp** contains the mutators which are not copiled for the time being since im working on Misra-C only for the time being, along with some Misra-C implementers.
**mutator-lvl2.cpp** contains some other Misra-C implementers. Rewriting the code in multiple stages allows for more simplistic rewrites and is also a check to see whether the output is actually buildable.
**mutator.sh** is the UI, which is supposed to work like just any other nix UI(option-wise).
the **utility** folder holds the C source and headers that are necessary to run the instruented code(currently unused).
**mutator-aux.cpp.h** hold the auxillary functions that most modules will need.
well there is the **makefile**.
the **test** folder holds the TDD tests.
