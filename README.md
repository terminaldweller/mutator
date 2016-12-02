# mutator

A C code mutator,Misra-C checker and when possible, a Misra-C implementer using the Clang frontend written mostly in C++ and some bash.<br/>
<br/>
Here's a quick look into the project files and directories:<br/>
* **mutator-lvl0.cpp** contains the Misra-C rules to check. The Executable named after it, will run the Misra-C rule checks.<br/>
* **mutator.cpp** contains the mutators which are not copiled for the time being since im working on Misra-C only for the time being, along with some Misra-C implementers.<br/>
* **mutator-lvl2.cpp** contains some other Misra-C implementers. Rewriting the code in multiple stages allows for more simplistic rewrites and is also a check to see whether the output is actually buildable.<br/>
* **mutator.sh** is the UI, which is supposed to work like just any other nix UI(option-wise).<br/>
* The **utility** folder holds the C source and headers that are necessary to run the instrumented code(currently unused).<br/>
* **mutator-aux.cpp.h** hold the auxillary functions that most modules will need.<br/>
* Well there is the **makefile**.<br/>
* The **test** folder holds the **TDD** tests.<br/>
* The **docs** folder contains the documents related to the project. Currently the doc for the current status of the Misra-C:2004 implementation is there.<br/>
* The folder named **tinyxml2** holds the tinyxml2 source files.<br/>
* The folder named **extra-tools** holds some tool that help the dev process. Right now it only holds a little script that has some limited argument parsing abilities to dump AST.<br/>
* The folder named **samples** holds the output samples for the project. Currently, you can find the text and XML output of the Misra-C rule checker run over the TDD tests.<br/>

#### **The Misra-C rule checking portion has not been extensively tested since it is still WIP but is very much buildable and usable.**<br/>

###Building and Running
To build the project, you need to have the LLVM libraries 4.0(that's the version the project is currently using) to avoid any unforseen results. The project can not be built with LLVM 3.8 or lower, but I haven't tested LLVM 3.9. Just run **make** and you're good to go. Running make will build three executables which can be used independently or with **mutator.sh**(use -h to see a list of options.)<br/>
You need to make sure that **llvm-configure** is in path since that's how the make file access the build parameters used to build the library (Thank you LLVM!).<br/>
The makefile uses **clang++ 4.0** as the compiler to build the project. On paper, any latest version of g++ should do the trick but this remains untested.<br/>
**mutator-lvl0** will run the Misra-C:2004 checks.<br/>
**mutator** will run the level-1 Misra-C:2004 implementers.<br/>
**mutator-lvl2** will run the level-2 Misra-C:2004 implementers.<br/>
<br/>
Currently, the mutation-only features(mutation for the sake of mutation, technically implementing Misra-C is also a form of mutation) are turned off in **mutator** and **mutator-lvl2** though some automatic code refactoring features work in both executables. Just run a sample code through **mutator** and then **mutator-lvl2** for a demo.<br/>
<br/>
If your code needs a compilation database for clang to understand it and you don't have one,you can use [Bear](https://github.com/rizsotto/Bear). Please note that bear will capture what the make runs, not what is in the makefile.<br/>

### Notes
#### **The project will be updated everytime there is a major LLVM release and will use those libraries instead of the old ones.**
#### **As soon as I manage to find a copy of the Misra-C:2012 document, I'll implement that. Currently the tool only supports Misra-C:2004.**
Misra-C rule checker outputs a simple text or xml report. JSON support will be implemented in the future.<br/>
I'm using **TDD**. The files under the **test** folder are for that purpose. They are not unit tests or are not meant to test that the build process was successful.Those tests will be added later.<br/>
The project has been tested to biuld on Fedora23(other major linux distros should be fine). Windows remains untested. I might give it a try when I feel masochistic enough.<br/>
The project might, at a later point in time, start using **Cmake** for the build process. Currently the TDD tests use CMake as an extra check.<br/>
Misra 2012 support will be added in the future.<br/>
Also a note regarding building the LLVM libraries. It is safer to build the libraries with clang++ if youre going to later use those libraries with clang++(you can get the distro version of clang from your distro's repo). The same applies to g++.<br/>
The master branch is the dev version. Release versions will be forked.<br/>
Doxygen comments will be added later on.<br/>
The XML Misra-C report uses [TinyXML-2](https://github.com/leethomason/tinyxml2). It is lighweight and fast and the license suits the mutator project.<br/>

### Future Feature
* Misra-c:2012 check support<br/>
* Support for JSON report output<br/>

###Support
Well, I don't have the Misra-C:2012 Document. If you or your organization/company are willing to donate a copy to mutator, hit me up.<br/>
