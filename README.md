# mutator

[![Build Status](https://travis-ci.org/bloodstalker/mutator.svg?branch=master)](https://travis-ci.org/bloodstalker/mutator)
<a href="https://scan.coverity.com/projects/bloodstalker-mutator">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/11154/badge.svg"/>
</a>
[![CII Best Practices](https://bestpractices.coreinfrastructure.org/projects/553/badge)](https://bestpractices.coreinfrastructure.org/projects/553)


A C code mutator,Misra-C checker and code transformation tool written using the Clang frontend(LibTooling) as a stand-alone in C++.<br/>
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

### Dev Status
All the as-of-yet implemented features of the project are very much buildable and usable at all times, even during the dev phase.<br/>
* **`mutator-lvl0`** is the executable responsible for the Misra-C rule checks. Currently it can check for 102 out of the 143 Misra-C:2004 rules. For a more accurate list please check out `misrac.ods` under `docs` in project's root.<br/>
* **`mutator`** and **`mutator-lvl2`** are collectively the code muatation and code transformation executables. Currently the automatic code transformation implemented is only limited to adding braces to blocks that are missing it and adding `else` if an if-else if is missing one. The mutation is only limited to statement and condition tagging for the time-being.<br/>

### Dev Plans
* Finish the Misra-C:2004 implementation.
* Start the unit tests for `mutator-lvl0`.
* start implementing `mutator` and `mutator-lvl2`.

##Building and Running

###Building
To build the project, you need to have the LLVM libraries 3.9 and up. The project can not be built with LLVM 3.8 or lower.<br/>
Here Are the build options:<br/>
* Running `make` will build the default target which is `all`. This will build all three executables, without support for coverage instrumentation.<br/>
* Running `make target-name` will only build the target. So for example, if you are only interested in building the Misra-C rule checker you can run `make mutator-lvl0`.<br/>
* The makefile option `CXX` tells the makefile which compiler to use. The default value is `clang++`. Currently the only two supported values are `clang++` and `g++`.<br/>
* The makefile option `BUILD_MODE` determines the build mode regarding coverage and support for builds with `g++`.<br/>
	* `COV_USE` and `COV_GEN` are for use with the `profdata` format. This option can only be used to build with `clang++`.<br/>
	* `COV_GNU` will generate `gcov` compliant coverage data. This option can only be used to build with `clang++`.<br/>
	* `COV_NO_CLANG` will build the executable with no source coverage instrumentation. This option can only be used to build with `clang++`.<br/>
	* `GNU_MODE` will build the executable with no source code coverage instrumentation for `g++`. Can only be used to build with `g++`.<br/>
* The `LLVM_CONF` option is used to tell the compiler which `llvm-config` to use. The default value is `llvm-config`.<br/>

So for example if you want to build the code with `clang++` without any coverage, and you only want to build the Misra-C rule checker, you should run:<br/>
<br/>
`make mutator-lvl0 CXX=clang++ BUILD_MODE=COV_NO_CLANG`<br/>
<br/>
Note: if you are building the llvm and clang libraries from source, then the llvm-config name will be `llvm-config` but if you are getting the libraries from a repository the llvm-config executable name may not be the same. In that case, you need to also pass `make` the `LLVM_CONF` variable. For example on Ubuntu trusty, if you get the repositories from llvm nightly builds, the llvm-config executable name will be `llvm-config-3.9` so you need to run:<br/>
<br/>
`make mutator-lvl0 CXX=clang++ BUILD_MODE=COV_NO_CLANG LLVM_CONF=llvm-config-3.9`<br/>
<br/>
Also do note that building the llvm libraries from source in Debug mode will require big space on your harddrive and will need more than 4GB of RAM. Release mode is less resource-greedy, of course.<br/>
Finally if you are having problems with the build, you could take a look at `.travis.yml` or under `CITPreBuildDep.sh` under `extra-tools` for some hints or help apart from asking for help, of course.<br/>
As a general rule, if you have Clang and LLVM libraries 3.9 or up on your platform, you can build `mutator`. If there are any problems with builds on platforms other than the ones in `.travis.yml` let me know.<br/>

###Running
To run any of the tree executables, just give a filename or a whitespace-separated list of files. The executables will print out the results to stdout.<br/>
To run the executables with the mutator UI, you can use `mutator.sh`. For a list of available options, you can type `./mutator.sh -h`.<br/>
* `-h, --help` prints out the help.<br/>
* `-c, --command` specifies the command you want to use.<br/>
	* `clean` runs make clean.<br/>
	* `build-all` runs make all.<br/>
	* `run` runs the `mutator` and `mutator-lvl2` executables on the inputs.<br/>
	* `default` runs build-all and then run.<br/>
	* `format` calls `clang-format` to format the mutant. later to be used for the test command.<br/>
	* `test` runs the tests on the executables and checks the results (not implemented yet).<br/>
	* `misrac` checks for misrac rules.<br/>
* `-v, --version` prints out the version.<br/>
* `-i, --input, -input` lets you choose the input file(or a white-space-separated list of files) that is going to be passed to the mutator executable(s).<br/>
* `-o, --output, -output` lets you choose where to put the mutant.<br/>
* `-opts 	--options, pass options to the executable(s). The executables support all the clang options. please enclose all the options in double quatation.`<br/>

So for example if you want to run the TDD tests for the Misra-C checker, you run:<br/>
<br/>
`./mutator.sh -c misrac -i ./test/testFuncs2.c ./test/testFuncs1.c -opts "-Wall"`<br/>
<br/>

<br/>
**mutator-lvl0** will run the Misra-C:2004 checks.<br/>
**mutator** will run the level-1 Misra-C:2004 implementers.<br/>
**mutator-lvl2** will run the level-2 Misra-C:2004 implementers.<br/>
<br/>
Currently, the mutation-only features(mutation for the sake of mutation, technically implementing Misra-C is also a form of mutation) are turned off in **mutator** and **mutator-lvl2** though some automatic code refactoring features work in both executables. Just run a sample code through **mutator** and then **mutator-lvl2** for a demo.<br/>
<br/>
If your code needs a compilation database for clang to understand it and you don't have one,you can use [Bear](https://github.com/rizsotto/Bear). Please note that bear will capture what the make runs, not what is in the makefile. So run `make clean` before invoking `bear make target`.<br/>

###Implementation Notes
This parts contains notes regarding the implementation of the mutator executables.

#### mutator-lvl0
* The implementation for the Misra-C:2004 rules 11.1,11.2,11.4 and 11.5 might seem unorthodox. Here's the explanation. The essence of the 11.1,11.2,11.3 and 11.4 rules as a collective is (after being translated into clang AST) that any time there is an `ImplicitCastExpr` or `CStyleCastExpr` that has `CastKind = CK_BitCast` the rule-checker should tag it. `CK_BitCast` means that a bit-pattern of one kind is being interpreted as a bit-pattern of another kind which is dangerous. This `CastKind` couple with the other `CastKinds` provided by the clang frontend enable us to tag cases where there is a deviation from the specified rules. Of course it is possible to check for exactly what the rules ask for but execution-time.<br/>

### Dev Method
TDD tests are created for each added feature which are stored under the **test** folder in the repo.<br/>
Smoke tests and Daily builds are conducted to make sure the code base builds correctly more than once every day.<br/>
Everytime there is a new commit, the code base is buildable and runnable. If you are having problems, raise an issue or let me know.<br/>
The code base uses Coverity for static analysis and CI Travis for checking the build matrix.<br/>

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

### Feedback
If you run into an issue please raise one here or just contact me with proper information(including source code that causes the issue if there is any).<br/>

### Future Features
* Misra-c:2012 check support<br/>
* Support for JSON report output<br/>
* Ability to turn off some rule checks<br/>

###Support
Well, I don't have the Misra-C:2012 Document. If you or your organization/company are willing to donate a copy to mutator, hit me up.<br/>
If the company/organization you represent wants to sponsor mutator, let me know.<br/>
#### Testers are always welcome. If you are interested, let me know. Testing mutator is as important, if not more, than implementing it.<br/>
