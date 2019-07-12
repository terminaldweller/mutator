# mutator

[![Build Status](https://travis-ci.org/bloodstalker/mutator.svg?branch=master)](https://travis-ci.org/bloodstalker/mutator)
[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2Fbloodstalker%2Fmutator.svg?type=shield)](https://app.fossa.io/projects/git%2Bgithub.com%2Fbloodstalker%2Fmutator?ref=badge_shield)
<a href="https://scan.coverity.com/projects/bloodstalker-mutator">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/11154/badge.svg"/>
</a>
[![CII Best Practices](https://bestpractices.coreinfrastructure.org/projects/553/badge)](https://bestpractices.coreinfrastructure.org/projects/553)
[![Coverage Status](https://coveralls.io/repos/github/bloodstalker/mutator/badge.svg?branch=master)](https://coveralls.io/github/bloodstalker/mutator?branch=master)

## Table of Contents

- [Overview](#overview)
  - [bruiser](#bruiser)
  - [cgrep](#cgrep)
  - [delf](#delf)
  - [dwasm](#dwasm)
  - [luatablegen](#luatablegen)
  - [obfuscator](#obfuscator)
  - [m0](#m0)
  - [Safercpp](#safercpp)
  - [mutatord](#mutatord)
- [License](#license)
- [How to get project mutator](#how-to-get-project-mutator)
- [Dev Status](#dev-status)
- [Dev Plans](#dev-plans)
- [Test Plans](#test-plans)
- [Announcements](#announcements)
- [Buillding and Running](#building-and-running)
  - [Building](#building)
  - [Requirements](#requirements)
    - [Linux and Mac](#linux-and-mac)
    - [Windows](#windows)
  - [Running](#running)
    - [Note](#note)
    - [The Action File](#the-action-file)
- [Implementation Notes](#implementation-notes)
  - [mutator-lvl0](#mutator-lvl0)
- [Directory Outline](#directory-outline)
- [Acknowledgements](#acknowledgements)
- [Feedback](#feedback)
- [Suggestions and Feature Requests](#suggestions-and-feature-requests)
- [TODO List](#todo-list)
- [Contributions](#contributions)
- [Support](#support)
- [Contact](#contact)
- [Gource](#gource)


## Overview

mutator is a suite of tools aimed at analysis and automation of C,C++ and machine code.<br/>
Here's a detailed list of what's currently available:<br/>

### bruiser
Essentially bruiser is a Lua REPL plus:
* You get tab-completion, suggestions and history(like a shell).<br>
* bruiser comes with its own extensions and libraries implemented in C and Cpp.<br/>
* Through bruiser's Xobj feature, you can pull in functions from object code, run them and get the result back.<br/>
* Through the ASMRewriter functionality you can manipulate the machine code and push it back in the object. For more detail you can look at the wiki or check out bruiser's README.md.<br/>
* Luarocks: You can use your Luarocks modules/libraries in bruiser too. Just make sure `luarocks` is in your path and bruiser will take care of the rest.<br/>

### cgrep
cgrep is grep for c/c++ source files. simple as that.<br/>
cgrep is added here as a submodule for more exposure. You can find the main repo [here](https://github.com/bloodstalker/cgrep).<br/>

### obfuscator
obfuscator is a C/C++ source code obfuscator.<br/>

### delf
`delf` is a custom ELF dump script developed for bruiser. bruiser uses it to interact with ELF files.<br/>
You can also use the script as a standalone to dump info on the ELF file to stdout.<br/>
delf is also hosted ona mirror repo [here](https://github.com/bloodstalker/delf).<br/>

### dwasm
'dwasm' is a custom WASM dump script. bruiser uses it to interact with WASM object files.<br/>
The script is also usable in an standalone manner.<br/>
dwasm is also hosted on a mirror repo [here](https://github.com/bloodstalker/dwasm).<br/>

### luatablegen
`luatablegen` is a python script that takes a json file including the details of a C structure, and generates C source and header files, a lua file including some convinience fields for the lua table and a markdown file including a summary of the table fields and their expected arg types and return types.<br/>
luatablegen is also hosted on a mirror repo [here](https://github.com/bloodstalker/luatablegen).<br/>

### m0
Run static checks on the source code, which at the time of writing, includes SaferCpp, Misra-c:2004 and most of MSC2012 and MSC98 rules.<br/>
m0's reports are generated in XML,JSON and simple text(AWK-friendly:`RS="\n";FS=":"`. Look at `ReportPrintPretty.sh` under `extra-tools`.).<br/>
`m0` also accpets a formatted file as its input, passing it all the options needed to run it. This feature is only available if `m0` is called through `mutator.sh`. For an example please look below.<br/>
Also to refrain from confusions, `m0`'s executable is actually named `mutator-lvl0` but for the sake of berevity it will be referred to as m0.<br/>

### Safercpp
Runs the automatic refactoring sets on your source code, automatically changing your code to use the SaferCpp libraries.<br/>
SaferCPlusPlus is essentially a collection of safe data types that are compatible with, and can substitute for, common unsafe native C++ types. You can read more [here](https://github.com/duneroadrunner/SaferCPlusPlus).<br/>

### mutatord
The mutator server/daemon and the client are also provided as optional features.<br/>
At the time of writing the client and server are provided to facilitate use of `m0` as a plugin.<br/>


You can Join the Maillist here, [mutator maillist](https://www.freelists.org/list/mutator).<br/>
You can follow Project `mutator` on twitter, @xashmith.<br/>

## License
SaferCpp is currently licensed under GPL-2.0.<br/>
All 3rd party libraries/code have their own respective license.<br/>
Excluding SaferCpp and other 3rd party code/libraries, everything else under project mutator is provided under GPL-3.0.<br/>


[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2Fbloodstalker%2Fmutator.svg?type=large)](https://app.fossa.io/projects/git%2Bgithub.com%2Fbloodstalker%2Fmutator?ref=badge_large)

## How to get project mutator

Before you run make, make sure you have all the dependencies:<br/>
* You need LLVM 5, 6 or 8.<br/>
* For `safercpp` you will need to have LLVM RTTI also.<br/>
* For `bruiser` you will need the python 3.5 or higher's dev package(source code and libpython), libcapstone, libkeystone and libffi.<br/>
* The other libraries used are either submodules or copied inside.<br/>
* `load.py` needs capstone. You can get it through pip(`pip install capstone`).<br/>
For More details you can look at the `Building` section.<br/>

Assuming you already have the dependencies:<br/>

```bash

git clone https://github.com/bloodstalker/mutator
git submodule init
git submodule update
make

```

mutator is also being hosted using [IPFS](https://github.com/ipfs/ipfs) but it's never up-to-date. To get it from IPFS just run:<br/>
```bash

git clone https://ipfs.io/ipfs/QmdBBG76K5rNSWB4iK4ZhTtiZAkSsyDpiWzcPsPfnHY2ZA/mutator

```
NOTE: this is mostly a novelty feature. The copy you can fetch from IPFS is usually outdated.<br/>

To build LLVM/Clang from source take a look at [here](https://clang.llvm.org/get_started.html) and [here](http://llvm.org/docs/GettingStarted.html).<br/>
To build `safercpp-arr` you to need to build Clang with RTTI enabled.<br/>
If you need any help regarding getting the requirements you can look at mutator's `.travis.yaml` or check out the [nightly builds for Debian/Ubuntu](http://apt.llvm.org).<br/>

### Dev Status
Currently there is only the master branch which is the dev branch. All the as-of-yet implemented features of the project are very much buildable and usable at all times, even during the dev phase on the master branch even if they are called "experimantal". If something's broken, please make a new issue on it.<br/>

* All tools are in the development stage.<br/>

### Dev Plans
* Bruiser: have bruiser support nested function calls and calls to external SOs.<br/>

### Test Plans

#### For a detailed list, you can view `tests.md` under `docs`.<br/>

* The Dev method I'm using is TDD so of course, we currently have TDD tests.<br/>
* For static analysis tools, mutator uses [Coverity](https://scan.coverity.com/projects/bloodstalker-mutator) which is integrated with [Travic CI](https://travis-ci.org/bloodstalker/mutator) so it runs every time on every commit.<br/>
* For dynamic analysis tools, currently mutator is using [Valgrind](http://valgrind.org). You can run it using `./mutator.sh -test mutator-lvl0 valgrind`. You do need to have `valgrind` installed.<br/>
* The code will be reviewed after the first pre-release version. I'm hoping to find some reviewers but if not, I'll have to do it myself.<br/>
* There will be unit tests after a first pre-release version.<br/>

## Announcements

* Introducing cgrep, grep for c/c++ source files.<br/>
* I will be taking a one month break from mutator to learn/work on some other things.<br/>
* Project mutator will be re-licensed to GPL-3.0.<br/>
* bruiser has a working poc demo for asmrewriter.<br/>
* bruiser has a working poc demo for Xobjs. For more info checkout bruiser's `README.md`.<br/>
* announcing `obfuscator`, the newest mutator family member. it's a C/C++ source obfuscation tool.<br/>
* mutator has a new experimental member, bruiser. The idea is that we are already inside the code, so why not break it?<br/>
* mutator now has a daemon,a server and a client. It works, but we all know how much weight we can put on "it just works", don't we?  I'll be polishing it over the coming days. For more info and detail see `README.md` under `daemon` in project root. Also, please do note that you don't have to use the server feature. You can just run mutator like before. It's an added functionality. It does not modify previous functionality.<br/>
* mutator will be implementing [SaferCPlusPlus](https://github.com/duneroadrunner/SaferCPlusPlus) rule checks and automatic refactoring of code bases to use SaferCPlusPlus libraries. The first phase will begin with implementing the compliancy checks. You can read more about SaferCPlusPlus [here](http://duneroadrunner.github.io/SaferCPlusPlus/).<br/>
* mutator's first website is up: [project mutator](https://bloodstalker.github.io/mutator/).<br/>
* project mutator has changed licenses from LGPLv3 to GPLv2.<br/>
* There are no plans in regards to keeping or dropping LLVM 3.9 support but do keep in mind the libraries mutator uses are not guaranteed to keep backwards-compatibility by their developers as they are still under development.<br/>

## Building and Running

### Building

#### Requirements
* `LLVM/Clang` 5.0, 6.0 or 8.0(we skip 7.0). For 9.0, the latest tested trunk version is 355787.<br/>
* `libffi`<br/>
* `libcapstone`<br/>
* `libkeystone`<br/>
* `libpython` 3.5 or higher<br/>
If capstone and keystone are not included inside your distro's reposotory and you're lazy like me, take a look under `extra-tools`. There are two scripts(one for each) to get those for the Travis image. You can use those.<br/>
The other requirements are either directly included or have to be included through `git submodule update`.<br/>

#### Linux

First clone the repo and then initialize and update the submodule in it:<br/>

```bash

git clone https://github.com/bloodstalker/mutator
git submodule init
git submodule update

```
Here Are the build options:<br/>

* Running `make` will build the default target which is `all`. This will build all the executables, without support for coverage instrumentation.<br/>
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
* The `PY_CONF` option tells make which `python-config` to use. The default is `python3-config`.<br/>

So for example if you want to build the code with `clang++` without any coverage, and you only want to build the Misra-C rule checker, you should run:<br/>
<br/>
`make mutator-lvl0 CXX=clang++ BUILD_MODE=COV_NO_CLANG`<br/>
<br/>
Note: if you are building the llvm and clang libraries from source, then the llvm-config name will be `llvm-config` but if you are getting the libraries from a repository the llvm-config executable name may not be the same. In that case, you need to also pass `make` the `LLVM_CONF` variable. For example on Ubuntu trusty, if you get the repositories from llvm nightly builds, the llvm-config executable name will be `llvm-config-3.9` so you need to run:<br/>
<br/>
`make mutator-lvl0 CXX=clang++ BUILD_MODE=COV_NO_CLANG LLVM_CONF=llvm-config-3.9`<br/>
<br/>
Also do note that building the llvm libraries from source in Debug mode will require big space on your harddrive and will need quite some RAM and needless say is slower. Release mode is less resource-greedy, of course.<br/>
Finally if you are having problems with the build, you could take a look at `.travis.yml` or under `CITPreBuildDep.sh` under `extra-tools` for some hints or help apart from asking for help, of course.<br/>

After building the executables, you can run(i personally don't):<br/>

```bash

make install

```

#### Windows

There is no official windows support and there are no plans to do so. Right now I don't have the resources to do so.<br/>
Let me know if you decide to try this and/or have any problems with it.<br/>

### Running

To run any of the executables, just give a filename or a whitespace-separated list of files. The executables will print out the results to stdout.<br/>
To run the executables with the mutator UI, you can use `mutator.sh`. For a list of available options, you can type `./mutator.sh -h`.<br/>

* `-h, --help` prints out the help.<br/>
* `-f,  --file` tells mutator to run the commands from the file.<br/>
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
* `-pp, --print-pretty`, prints the output in a pretty format in a new file. The new file has the same name with a "-pretty" added to the name in the same directory.<br/>
* `-t,  --test`, runs the tests on the built executables. It should be followed by an executable name and the test to run on it. The accepted options are: tdd,valgrind. For example: `-test mutator-lvl0 valgrind`.<br/>
* `-opts  --options, pass options to the executable(s). The executables support all the clang options. please enclose all the options in double quatation. This is basically a pass-through option. Everything appearing inside will be passed through to the executable.`<br/>
* `-copts --customoptions`, just like `-opts` but passes the custom options defined for each executable. It is pass-through. Example: `-copts "-MainOnly=false -SysHeader"`.<br/>

`m0` options:<br/>

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

`mutator` can accept a file which tells it what to do. Currently this feature is only supported for `m0`. You can find a sample under `./samples` named `action_file.mutator`.

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
If your code needs a compilation database for clang to understand it and you don't have one,you can use [Bear](https://github.com/rizsotto/Bear). Please note that bear will capture what the make runs, not what is in the makefile. So run `make clean` before invoking `bear make target`. `cmake` can also generate compilation databases if you are using it.<br/>

### Implementation Notes
This part contains notes regarding the implementation of m0, m1 and m2.<br/>

#### mutator-lvl0
* The implementation for the Misra-C:2004 rules 11.1,11.2,11.4 and 11.5 might seem unorthodox. Here's the explanation. The essence of the 11.1,11.2,11.3 and 11.4 rules as a collective is (after being translated into clang AST) that any time there is an `ImplicitCastExpr` or `CStyleCastExpr` that has `CastKind = CK_BitCast` the rule-checker should tag it. `CK_BitCast` means that a bit-pattern of one kind is being interpreted as a bit-pattern of another kind which is dangerous. This `CastKind` couple with the other `CastKinds` provided by the clang frontend enable us to tag cases where there is a deviation from the specified rules. Of course it is possible to check for exactly what the rules ask for but execution-time.<br/>

* The implementation for the Misra-C:2004 rule 16.7 only checks for changes made to the pointee object when the pointer is the LHS of a binary `=` operator. Changes can be made to the pointee object through UnaryOperators `++` and `--` but since the `*` UnaryOperator is also involved, the behaviour is undefined as to which unaryOperator is evaluated at first so depending on the original build toolchain's implementation, it could modify the pointer itself or the pointee object. such cases are already being tagged by 12.13 and the fix to that rule will either make the source code in a way that will be tagged by 16.7 or fix it so it wont get tagged. Either way, it is pointless to look for changes to pointee objects through `++` and `--` operators, postfix or prefix. Do note that even if at later times, mutator supports pragmas for different compiler implementation behaviours, the last argument still stands.<br/>


### Directory Outline
Here's a quick look into the project files and directories:<br/>

* **mutator-lvl0.cpp** contains the Misra-C rules to check. The Executable named after it, will run the Misra-C rule checks.<br/>
* **mutator-lvl1.cpp** contains the mutators which are not compiled for the time being since im working on Misra-C only for the time being, along with some Misra-C implementers.<br/>
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

### Dev Method
TDD tests are created for each added feature which are stored under the **test** folder in the repo.<br/>
Smoke tests and Daily builds are conducted to make sure the code base builds correctly more than once every day.<br/>
Every time there is a new commit, the code base is buildable and runnable. If you are having problems, raise an issue or let me know.<br/>
The code base uses Coverity for static analysis and CI Travis for checking the build matrix.<br/>
Coveralls integration with Travis for code coverage.<br/>
Also the `precommitTests.sh` script under `extra-tools` is run before every commit to make sure commits are not horribly broken.<br\>

### Notes

#### **The project will be updated every time there is a major LLVM release and will use those libraries instead of the old ones for development**<br/>

The project might, at a later point in time, start using **Cmake** for the build process.<br/>
Misra 2012 support will be added in the future.<br/>
Also a note regarding building the LLVM libraries. It is safer to build the libraries with clang++ if you're going to later use those libraries with clang++(you can get the distro version of clang from your distro's repo). The same applies to g++.<br/>
The master branch is the dev version. Release versions will be forked.<br/>

### Acknowledgements
Project mutator uses the following libraries:
* [LLVM/Clang](http://llvm.org)
* [Lua](https://github.com/lua/lua)
* [Linenoise](https://github.com/antirez/linenoise)
* [capstone](https://github.com/aquynh/capstone)
* [keystone](https://github.com/keystone-engine/keystone)
* [TinyXML2](https://github.com/leethomason/tinyxml2)
* [JSON](https://github.com/nlohmann/json)
* [SaferCPP](https://github.com/duneroadrunner/SaferCPlusPlus)
* [LuaJIT](https://github.com/LuaJIT/LuaJIT)
* [keccak-tiny](https://github.com/coruus/keccak-tiny)
* Thanks to [Jonathan Brossard](https://github.com/endrazine) for [WCC](https://github.com/endrazine/wcc), specifically `wsh` which is the inspiration for `bruiser`. Check it out if you haven't already.<br/>

All mutator source code is provided under GPL-3.0.<br/>
All libraries have their respective licences. For more info you can just visit their respective links.<br/>

### Feedback
If you run into an issue please make a new issue.<br/>

### Suggestions and Feature Requests
You can make a new issue for requests and suggestion. Label them with "Feauture Request".<br/>
Besides that, If you have any suggestions or have any feature requests for project mutator, you can send them to `thabogre@gmail.com`. I'll try to keep an open mind, so even if you feel like it might not be right up mutator's alley, do send them. Worst case, I'll just say no.<br/>

### TODO List
For a list of things that need to be done, take a look at the list of issues.<br/>

### Contributions
For a full description please read `Contributions.md` in the repo root.<br/>

### Support
Well, I don't have the Misra-C:2012 Document. If you or your organization/company are willing to donate a copy to mutator, hit me up.<br/>
#### Testers are always welcome. If you are interested, let me know. Testing mutator is as important, if not more, than implementing it.<br/>

### Contact
You can email me at thabogre@gmail.com, there is also the twitter account for the mutator project, @xashmith and there is the mutator maillist, mutator-repost@freelists.org. You need to be a member though to be able to send mail on the maillist. The maillist is moderated.<br/>

### Gource
Mostly because why not:<br/>
[![gource-vid](/web-resources/img/gource.png)](https://www.youtube.com/watch?v=A53auytlNMM)

<a href="https://twitter.com/xashmith" class="twitter-follow-button" data-show-count="false">Follow @xashmith</a><script async src="//platform.twitter.com/widgets.js" charset="utf-8"></script>