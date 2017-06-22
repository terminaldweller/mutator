# mutator

[![Build Status](https://travis-ci.org/bloodstalker/mutator.svg?branch=master)](https://travis-ci.org/bloodstalker/mutator)
<a href="https://scan.coverity.com/projects/bloodstalker-mutator">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/11154/badge.svg"/>
</a>
[![CII Best Practices](https://bestpractices.coreinfrastructure.org/projects/553/badge)](https://bestpractices.coreinfrastructure.org/projects/553)
[![Coverage Status](https://coveralls.io/repos/github/bloodstalker/mutator/badge.svg?branch=master)](https://coveralls.io/github/bloodstalker/mutator?branch=master)
<a href="https://twitter.com/xashmith" class="twitter-follow-button" data-show-count="false">Follow @xashmith</a><script async src="//platform.twitter.com/widgets.js" charset="utf-8"></script>


Here's the elevator pitch: mutator is a suite of tools aimed at analysis and automation of C/C++ code development.<br/>
Here's a detailed list of what's currently available:<br/>
mutator-lvl0(m0) will run static checks on the source code, which at the time of writing, includes SaferCpp, Misra-c:2004 and most of MSC2012 and MSC98 rules.<br/>
Safercpp runs the automatic refactoring sets on your source code, automatically changing your code to use the SaferCpp libraries.<br/>
mutator-lvl1 and mutator-lvl2 currently only have a few simple refactorings mostly related to code formatting.<br/>
bruiser is an exciting yet experimental feature. You can read about the idea in bruiser's directory.<br/>
mutatord, the mutator server and the client are also provided as optional features.<br/>

#### So why should I choose to use m0 over another static analysis tool?
That would be because m0's requirement set is different. m0's tests are aimed at lowering the bug-count(bug prevention, not finding bugs), increasing portablity and maintainability across different architectures and environments.<br/>

#### What is SaferCpp?
SaferCPlusPlus is essentially a collection of safe data types that are compatible with, and can substitute for, common unsafe native C++ types. You can read more [here](https://github.com/duneroadrunner/SaferCPlusPlus).<br/>

Reports are generated in XML,JSON and simple text(AWK-friendly:`RS="\n";FS=":"`. Look at `ReportPrintPretty.sh` under `extra-tools`.).<br/>
You can also run the mutator daemon(`mutatord`) which runs it in a client-server mode, with the client being a thin client.<br/>
You can Join the Maillist here, [mutator maillist](https://www.freelists.org/list/mutator).<br/>
You can follow Project `mutator` on twitter, @xashmith.
<br/>

## mutator

mutator is a suite of tools aimed at analysis and automation of C/C++ code development with thin client-server architectur written using the Clang front-end(LibTooling) as a stand-alone in C++. It consists of three(well so far) executables and a UI written in bash. You can run executables like any other CLI tool or just run them through the UI which again acts like a CLI tool. `mutator` also accepts action files that tell it what to do.<br/>

<br/>
**mutator-lvl0** will run the Misra-C checks.<br/>
**mutator** will run the level-1 implementers and mutators.<br/>
**mutator-lvl2** will run the level-2 implementers and mutators.<br/>
Mutation levels have nothing to do with the order of mutants.<br/>
**mutatord** is the mutator daemon that runs the server.<br/>
**mutatorclient** is the thin client that sends commands to the server.<br/>
**safercpp-arr** is SaferCPP's automatic refactoring tool for arrays.<br/>
**bruiser** the short explanation is that bruiser is an interactive shell that mutates code on demand, gives you insight on the code-base loaded and more. For more info read the README on bruiser's folder in project root.<br/>
<br/>

## How To get project mutator

Assuming you already have the LLVM/Clang libraries, just run :

```bash

git clone https://github.com/bloodstalker/mutator
git submodule init
git submodule update
make
make install

```

If you don't have them, you can build them or get them from a repo.<br/>
To build LLVM/Clang from source take a look at [here](https://clang.llvm.org/get_started.html) and [here](http://llvm.org/docs/GettingStarted.html).<br/>
To build `safercpp-arr` you to need to build Clang with RTTI enabled.<br/>
On Fedora you can just get the Requirements by dnf. For Ubuntu and Debian either look at mutator's `.travis.yaml` or check out the [nightly builds for Debian/Ubuntu](http://apt.llvm.org).<br/>

### Dev Status
All the as-of-yet implemented features of the project are very much buildable and usable at all times, even during the dev phase on the master branch. If something's not working properly let me know.<br/>

* `mutator-lvl0` is the executable responsible for the Misra-C rule checks. Currently it has reached a release candidate, soon to be branched and have unit tests written for it.<br/>
* `mutator-lvl1` and **`mutator-lvl2`** are collectively the code muatation and code transformation(automatic-refactoring) executables. Currently the automatic code transformations implemented are only limited to adding braces to blocks that are missing it, fixing SwitchStmts with adding a default clause if missing and breaks, swapping the RHS and LHS when the RHS is a constant and adding `else` if an if-else is missing one. The mutation is only limited to statement and condition tagging for the time-being.<br/>

### Dev Plans
* Bruiser
* Branch a release candidate for `mutator-lvl0`, then start writing the unit tests. This might take a while, since I will try to find testers and code reviewers. If you are willing to help with testing, Email me at `thabogre@gmail.com`. Please set the subject to `mutator-lvl0 test` so I wouldn't miss it. I'll do the unit tests and reviews myself if I don't manage to find volunteers for that. While it's far from ideal, it's better than nothing.
* Implementing the automatic refactoring features of mutator.
* Upgrading the UI to be able to handle the new automatic-refactoring features.

### Test Plans

#### For a detailed list, you can view `tests.md` under `docs`.<br/>

* The Dev method is TDD so of course, we currently have TDD tests.
* For static analysis tools, mutator uses [Coverity](https://scan.coverity.com/projects/bloodstalker-mutator) which is integrated with [Travic CI](https://travis-ci.org/bloodstalker/mutator) so it runs everytime on every commit.<br/>
* For dynamic analysis tools, currently mutator is using [Valgrind](http://valgrind.org). You can run it using `./mutator.sh -test mutator-lvl0 valgrind`. You do need to have `valgrind` installed.<br/>
* The code will be reviewed after the first pre-release version. I'm hoping to find some reviewers but if not, I'll have to do it myself.<br/>
* There will be unit tests after a first pre-release version.<br/>

## Announcements

* mutator has a new experimental member, bruiser. The idea is that we are already inside the code, so why not break it?<br/>
* mutator now has a daemon,a server and a client. It works, but we all know how much weight we can put on "it just works", don't we?  I'll be polishing it over the coming days. For more info and detail see `README.md` under `daemon` in project root. Also, please do note that you don't have to use the server feature. You can just run mutator like before. It's an added functionality. It does not modify previous functionality.<br/>
* mutator will be implementing [SaferCPlusPlus](https://github.com/duneroadrunner/SaferCPlusPlus) rule checks and automatic refactoring of code bases to use SaferCPlusPlus libraries. The first phase will begin with implementing the compliancy checks. You can read more about SaferCPlusPlus [here](http://duneroadrunner.github.io/SaferCPlusPlus/).<br/>
* mutator's first website is up: [project mutator](https://bloodstalker.github.io/mutator/).<br/>
* project mutator has changed licenses from LGPLv3 to GPLv2.<br/>
* `mutator-lvl0` has reached a release candidate. I will branch a release candidate for it and then we can start the unit tests.<br/>
* The Implementation of the automatic refactoring facilities of `mutator` has begun. The UI is not yet capable of accomodating the current features so I'll try to add them as soon as possible.<br/>
* LLVM has bumped to 5.0 so mutator has also changed to using that(trunk:292415). Everything is in working order.<br/>
* The Travis build is now using 4.0 for build checks. LLVM 3.9 is still supported minus rule check 8.8 which uses a non-existant matcher in 3.9.<br/> 
* There are no plans in regards to keeping or dropping LLVM 3.9 support but do keep in mind the libraries mutator uses are not guaranteed to keep backwards-compatibility by their developers as they are still under development.<br/>
* There are only 4 actual defects on mutator's Coverity scan that belong to mutator's source code and not the library. 4 are potential defects which are not really defects.<br/>
* I will be taking a break of sorts for the next two weeks (probably less but still) starting from 1.25.2017.<br/>

## Building and Running

### Building

#### Linux

First clone the repo and then initialize and update the submodule in it:<br/>

```bash

git clone https://github.com/bloodstalker/mutator
git submodule init
git submodule update

```

To build the project, you need to have the LLVM libraries 4.0 or higher. mutator can not be built with LLVM 3.9 or lower. The latest tested is LLVM trunk:301395.<br/>
Here Are the build options:<br/>

* Running `make` will build the default target which is `all`. This will build all three executables, without support for coverage instrumentation.<br/>
* Running `make target-name` will only build the target. So for example, if you are only interested in building the Misra-C rule checker you can run `make mutator-lvl0`.<br/>
* The makefile option `CXX` tells the makefile which compiler to use. The default value is `clang++`. Currently the only two supported values are `clang++` and `g++`.<br/>
* The makefile option `BUILD_MODE` determines the build mode regarding coverage and support for builds with `g++`.<br/>
	* `COV_USE` and `COV_GEN` are for use with the `profdata` format. This option can only be used to build with `clang++`.<br/>
	* `COV_GNU` will generate `gcov` compliant coverage data. This option can only be used to build with `clang++`.<br/>
	* `COV_NO_CLANG` will build the executable with no source coverage instrumentation. This option can only be used to build with `clang++`.<br/>
	* `COV_NO_CLANG_1Z` will build with support for C++1z support. I use this for dev builds.<br/>
	* `WIN_BUILD` will later be used to support Windows builds. It assumes there is a llvm-config and it's in windows path.<br/>
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

After building the executables, you need to run:<br/>

```bash

make install

```

#### Windows

Currently a Windows build is not officially supported but if you can build LLVM/Clang, then you can build mutator too. Currently the latest version of LLVM/Clang available on Cygwin is 3.8 and that does not include the dev-libraries so you can't use those. Just use the Guide on LLVM for building using Visual Studio. After you have the headers and libraries and llvm-config, just use `BUILD_MODE=WIN_BUILD` with Clang and you should be good to go.<br/>
Let me know if you decide to try this and/or have any problems with it.<br/>

### Running

To run any of the tree executables, just give a filename or a whitespace-separated list of files. The executables will print out the results to stdout.<br/>
To run the executables with the mutator UI, you can use `mutator.sh`. For a list of available options, you can type `./mutator.sh -h`.<br/>

* `-h, --help` prints out the help.<br/>
* `-f,	--file` tells mutator to run the commands from the file.<br/>
* `-c, --command` specifies the command you want to use.<br/>
	* `clean` runs make clean.<br/>
	* `build-all` runs make all.<br/>
	* `run` runs the `mutator` and `mutator-lvl2` executables on the inputs.<br/>
	* `default` runs build-all and then run.<br/>
	* `format` calls `clang-format` to format the mutant. Later to be used for the test command.<br/>
	* `test` runs the tests on the executables and checks the results (not implemented yet).<br/>
	* `misrac` checks for misrac rules.<br/>
* `-v, --version` prints out the version.<br/>
* `-i, --input, -input` lets you choose the input file(or a white-space-separated list of files) that is going to be passed to the mutator executable(s).<br/>
* `-o, --output, -output` lets you choose where to put the mutant.<br/>
* `-pp,	--print-pretty`, prints the output in a pretty format in a new file. The new file has the same name with a "-pretty" added to the name in the same directory.<br/>
* `-t,	--test`, runs the tests on the built executables. It should be followed by an executable name and the test to run on it. The accepted options are: tdd,valgrind. For example: `-test mutator-lvl0 valgrind`.<br/>
* `-opts 	--options, pass options to the executable(s). The executables support all the clang options. please enclose all the options in double quatation. This is basically a pass-through option. Everything appearing inside will be passed through to the executable.`<br/>
* `-copts	--customoptions`, just like `-opts` but passes the custom options defined for each executable. It is pass-through. Example: `-copts "-MainOnly=false -SysHeader"`.<br/>

`mutator-lvl0` options:<br/>

* SysHeader, will let the executable know that you wish the checks to run through system headers as well. Off by default.<br/>
* MainOnly, will only publish check results for matches residing in the main file,i.e. The current TU(Translation Unit).<br/>
* MisraC2004,MisraC2012,C2,C3 will let the executable know which Misra guidelines you want the source to be checked against. Currently only supports MisraC2004 or C2.<br/>

#### Note

Some of Misra-C rules are already implemented by Clang as warnings, so for those cases, mutator uses the built-in diagnostics instead of re-inventing the wheel. For those diagnostics to appear in the mutator report you should refrain from using the Clang `-w` flag as that silences the warnings.<br/>

If you are running the executables using `mutator.sh` you don't need to read this note through. If you are running the executable directly however, then you have to pass groups of arguments in a specific order otherwise the executable won't be able to recognize the options and will throw errors. For example this is the right way to run `mutator-lvl0`:<br/>

```bash

./mutator-lvl0 -SysHeader=false -MainOnly=true  ./test/testFuncs3.c -- -std=c90 -Wall -I/lib/gcc/x86_64-redhat-linux/5.3.1/include

```

So for example if you want to run the TDD tests for the Misra-C checker, you run:<br/>

```bash

./mutator.sh -c misrac -i ./test/testFuncs2.c ./test/testFuncs1.c -opts "-Wall -std=c90"

```

Do note that if your file has included standard header libraries, you do need to tell it where to look for them, so for the above example on Fedora, you would need to run:<br/>

```bash

./mutator.sh -c misrac -i ./test/testFuncs2.c ./test/testFuncs1.c -opts "-Wall -I/lib/gcc/x86_64-redhat-linux/5.3.1/include/"

```

Here's the command I use to run the TDD tests:<br/>

```bash

/mutator.sh -c misrac -i ./test/testFuncs1.c ./test/testFuncs2.c -pp -opts "-std=c90 -I/lib/gcc/x86_64-redhat-linux/5.3.1/include" -copts "-SysHeader=false -MainOnly=true" 2> /dev/null

```

#### The Action File

`mutator` can accept a file which tells it what to do. Currently this feature is only supported for `mutator-lvl0`. You can find a sample under `./samples` named `action_file.mutator`.

```bash

action_name:my_action1
executable_name:mutator-lvl0
#these are the options specific to the executable
exec_opts:-SysHeader=false -MainOnly=true
in_files:./test/testFuncs1.c ./test/testFuncs2.c ./test/testFuncs3.c
#clang options
libtooling_options:-std=c90 -I/lib/gcc/x86_64-redhat-linux/5.3.1/include
#the output file
out_files:./test/misra-log
#the log file
log_files:
print_pretty:true
end_action:run

```
Here's the explanation for the fields:<br/>

* `action_name` lets you specify a name for the action to run. Marks the start of an action.<br/>
* `executable_name` is used for determining which executable to run.<br/>
* `exec_opts` is the field used for passing the executable-specific options.<br/>
* `in_files` is a list of the input files to pass to the executable.<br/>
* `libtooling_options` is used for passing the clang options.<br/>
* `out_files` is used to pass the result file to mutator. For `mutator-lvl0`, this field determines the Misra-C check results.<br/>
* `log_files` is used to pass the file to hold the log. Mostly meant to be used with `mutator` and `mutator-lvl2`.<br/>
* `print_pretty` is a boolean switch. Used for running `ReportPrintPretty.sh`.<br/>
* `end_action` is used to tell `mutator.sh` what action to do. Currently the only supported options are "run" and "stop". "run" will run the action and "stop" will not run it. Also marks the end of an action.<br/>
* Lines starting with a hash(`#`) are considered comments.<br/>

Field names shall not have preceding whitespace or tab. The `:` character shall immediately follow the field name with options appearing after it.<br/>
`end_action` field should appear as the last field of a action.<br/>

You can run the sample action file with this:<br/>

```bash

./mutator.sh --file samples/action_file.mutator

```

Currently, the mutation-only features(mutation for the sake of mutation, technically implementing Misra-C is also a form of mutation) are turned off in **mutator** and **mutator-lvl2** though some automatic code refactoring features work in both executables. Just run a sample code through **mutator** and then **mutator-lvl2** for a demo.<br/>
<br/>
If your code needs a compilation database for clang to understand it and you don't have one,you can use [Bear](https://github.com/rizsotto/Bear). Please note that bear will capture what the make runs, not what is in the makefile. So run `make clean` before invoking `bear make target`.<br/>

### Implementation Notes
This part contains notes regarding the implementation of the mutator executables.

#### mutator-lvl0
* The implementation for the Misra-C:2004 rules 11.1,11.2,11.4 and 11.5 might seem unorthodox. Here's the explanation. The essence of the 11.1,11.2,11.3 and 11.4 rules as a collective is (after being translated into clang AST) that any time there is an `ImplicitCastExpr` or `CStyleCastExpr` that has `CastKind = CK_BitCast` the rule-checker should tag it. `CK_BitCast` means that a bit-pattern of one kind is being interpreted as a bit-pattern of another kind which is dangerous. This `CastKind` couple with the other `CastKinds` provided by the clang frontend enable us to tag cases where there is a deviation from the specified rules. Of course it is possible to check for exactly what the rules ask for but execution-time.<br/>

* The implementation for the Misra-C:2004 rule 16.7 only checks for changes made to the pointee object when the pointer is the LHS of a binary `=` operator. Changes can be made to the pointee object through UnaryOperators `++` and `--` but since the `*` UnaryOperator is also involved, the behaviour is undefined as to which unaryOperator is evaluated at first so depending on the original build toolchain's implementation, it could modify the pointer itself or the pointee object. such cases are already being tagged by 12.13 and the fix to that rule will either make the source code in a way that will be tagged by 16.7 or fix it so it wont get tagged. Either way, it is pointless to look for changes to pointee objects through `++` and `--` operators, postfix or prefix. Do note that even if at later times, mutator supports pragmas for different compiler implementation behaviours, the last argument still stands.<br/>

Here's a quick look into the project files and directories:<br/>

* **mutator-lvl0.cpp** contains the Misra-C rules to check. The Executable named after it, will run the Misra-C rule checks.<br/>
* **mutator-lvl1.cpp** contains the mutators which are not copiled for the time being since im working on Misra-C only for the time being, along with some Misra-C implementers.<br/>
* **mutator-lvl2.cpp** contains some other Misra-C implementers. Rewriting the code in multiple stages allows for more simplistic rewrites and is also a check to see whether the output is actually buildable.<br/>
* **mutator.sh** is the UI, which is supposed to work like just any other nix UI(option-wise).<br/>
* The **utility** folder holds the C source and headers that are necessary to run the instrumented code(currently unused).<br/>
* **mutator-aux.cpp.h** hold the auxiliary functions that most modules will need.<br/>
* Well there is the **makefile**.<br/>
* The **test** folder holds the **TDD** tests.<br/>
* The **docs** folder contains the documents related to the project. Currently the doc for the current status of the Misra-C:2004 implementation is there.<br/>
* The folder named **tinyxml2** holds the tinyxml2 source files.<br/>
* The folder named **extra-tools** holds some tool that help the dev process.<br/>
* The folder named **samples** holds the output samples for the project. Currently, you can find the text and XML output of the Misra-C rule checker run over the TDD tests.<br/>

#### **The Misra-C rule checking portion has not been extensively tested since it is still WIP but is very much buildable and usable.**<br/>

### Dev Method
TDD tests are created for each added feature which are stored under the **test** folder in the repo.<br/>
Smoke tests and Daily builds are conducted to make sure the code base builds correctly more than once every day.<br/>
Every time there is a new commit, the code base is buildable and runnable. If you are having problems, raise an issue or let me know.<br/>
The code base uses Coverity for static analysis and CI Travis for checking the build matrix.<br/>

### Notes

#### **The project will be updated everytime there is a major LLVM release and will use those libraries instead of the old ones.**<br/>

#### **As soon as I manage to find a copy of the Misra-C:2012 document, I'll implement that. Currently the tool only supports Misra-C:2004.**<br/>
Misra-C rule checker outputs a simple text or xml report. JSON support will be implemented in the future.<br/>
I'm using **TDD**. The files under the **test** folder are for that purpose. They are not unit tests or are not meant to test that the build process was successful.Those tests will be added later.<br/>
The project has been tested to biuld on Fedora25(other major linux distros should be fine). Windows remains untested. I might give it a try when I feel masochistic enough.<br/>
The project might, at a later point in time, start using **Cmake** for the build process. Currently the TDD tests use CMake as an extra check.<br/>
Misra 2012 support will be added in the future.<br/>
Also a note regarding building the LLVM libraries. It is safer to build the libraries with clang++ if youre going to later use those libraries with clang++(you can get the distro version of clang from your distro's repo). The same applies to g++.<br/>
The master branch is the dev version. Release versions will be forked.<br/>
Doxygen comments will be added later on.<br/>

### Acknowledgements
Project mutator uses the following cool libraries:
* [SaferCPP](https://github.com/duneroadrunner/SaferCPlusPlus)
* [TinyXML2](https://github.com/leethomason/tinyxml2)
* [JSON](https://github.com/nlohmann/json)
* [LLVM/Clang](http://llvm.org)
* [Linenoise](https://github.com/antirez/linenoise)
* [Lua](https://github.com/lua/lua)
* [LuaJIT](https://github.com/LuaJIT/LuaJIT)

### Feedback
If you run into an issue please raise one here or just contact me with proper information(including source code that causes the issue if there is any).<br/>

### Suggestions and Feature Requests
If you have any suggestions or have any feature requests for project mutator, you can send them to `bloodstalker@zoho.com`. I'll try to keep an open mind, so even if you feel like it might not be right up mutator's alley, do send them. Worst case, I'll just say no.<br/> 

### TODO List
* Misra-c:2012 and 98 check support<br/>
* Ability to turn off some rule checks<br/>
* Using Appveyor to test windows builds<br/>
* Have the server capture stderr<br/>

### Support
Well, I don't have the Misra-C:2012 Document. If you or your organization/company are willing to donate a copy to mutator, hit me up.<br/>
If the company/organization you represent wants to sponsor mutator, let me know.<br/>

#### Testers are always welcome. If you are interested, let me know. Testing mutator is as important, if not more, than implementing it.<br/>

### Contact
You can email me at thabogre@gmail.com, there is also the twitter account for the mutator project, @xashmith and there is the mutator maillist, mutator-repost@freelists.org. You need to be a member though to be able to send mail on the maillist. The maillist is moderated.<br/>
