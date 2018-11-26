## bruiser

### What is it?
bruiser is a object-file manipulation tool implemented in C/C++ which provides its functionality through Lua.<br/>
Regarding the actual functionality:<br/>
Object file libraries: Object file manipulation libraries are implemented in C and wrapped for use in Lua.<br/>
Xobj: Pull in funtions from ELF objects, call them and get the result back(basically ffi).<br/>
ASMrewriter: Currently returns a table containing all the jumps in the x86-64 machine code.<br/>
Ramdump: Get the memory of a running process.<br/>

For working demos you can skip to the end of the README.<br/>

### Building

## Requirements
* libffi<br/>
* libcapstone<br/>
* libkeystone<br/>
* python 3.5(or higher) development packages<br/>
* LLVM/Clang(5.0,6.0 or 8.0. 7.0 not supported)<br/>
Other dependencies(lua, [faultreiber](https://github.com/bloodstalker/faultreiber), [luatablegen](https://github.com/bloodstalker/luatablegen), [linenoise](https://github.com/antirez/linenoise)) are self-contained.<br/>

## Make
Running `make` from bruiser's make or `make bruiser` from the main makefile in the root directory of mutator will take care of building bruiser given that you already taken care of all the requirements.<br/>
It is generally a good idea to run `make deepclean` on bruiser's makefile on every pull since I occasionally have to make changes to Lua's sources, makefile or we need to re-generate the code-gen files.<br/>

### User Engagement
bruiser provides base-line functionality and libraries. The eventual goal is for users to use the baseline and provide more useful and abstracted functionalities in the form of lua libraries built on top of the bruiser environment and python plugins.<br/>
So without user engagement bruiser is meaningless. Feel free to make suggestions on how I can make it more friendly for other people to get involved.<br/>

### supported object file formats
bruiser currently supports the following object formats:<br/>
* WASM
buirser will eventually support the following formats:<br/>
* ELF
* PE
* Macho

### Python pipe
bruiser has a built-in python pipe. There are two reasons for it being there:<br/>
* one, this way i can test some ideas in python instead of a hard c/c++ implementation which is faster.<br/>
* two, eventually the python pipe is intended to act the same way as in gdb.<br/>

### How does it work?
bruiser's main code is implemented in C++. The lower-level-interfacing parts are usually implemented in C. The object-file manipulation libraries are generated through two code-generators which make the code base more maintable.<br/>
Currently bruiser used two code-generators, [faultreiber](https://github.com/bloodstalker/faultreiber) and [luatablegen](https://github.com/bloodstalker/luatablegen). faultreiber generates a binary file-format parser library for a given format. luatablegen wraps all the structures related to that file format for Lua. Both code generators can use the same XML file which provides them with the definition of the file format. As a disclaimer, I implemented both faultriber and luatablegen for bruiser but they are general-purpose and can work without the use of each other.<br/>
bruiser also features a built-in Python3 pipe which currently allows you to call your python functions from bruiser(i.e. Lua). Eventually the python pipe will turn into a plugin-enabler for bruiser.<br/>

#### Lua Defaults
You can think of this as the bruiser dot file.<br/>
Upon start-up, bruiser will look to find a file called `defaults.lua` in the same directory as the bruiser executable to run before running any user provided lua code, both in interactive and non-interactive modes. The path to the lua default file can be changed from the default value by the `LuaDefault` option passed to bruiser on startup.<br/>
The current lua default script provided will run `luarocks path --bin` and add `paht` and `cpath` so that you can use your Lua modules from bruiser.<br/>

### Lua vs Luajit
For the first incarnation, bruiser will only support lua and not luajit. luajit is way faster than lua which will play an important role in bruiser's overall performance but luajit is generally less stable than lua and usually behind in terms of what new features of lua the language it supports.<br/>
The plan is to add both and for the user to be able to pick which one to use when running bruiser. Unfortunately there is no estimated date.<br/>

### Warning
The current implementation loads all lua libraries which also includes it's `os` library. To give you an idea, `os.execute()` is very similar to `system()` in C. This decision has been made to speed up testing and the dev process.<br/>
Also like `mutatord` and `mutatorclient`, bruiser does not need any sudo access.<br/>
briuser's executable expects to stay where it is originally built in, don't move it. use symlinks, aliases, ... whatever to suit your needs.<br/>

### Useful Lua Scripts
The dir named `lua-scripts` houses demos, examples and useful lua scripts for bruiser.<br/>
If you happen to write a Lua script for bruiser that you think other people will find useful, then please add it to `lua-scripts` on your fork and make a PR.<br/>

### Run All Demos
Run `run.sh` inside bruiser's directory. This will run all the demos buirser currently has, which at the time of writng include the xobj demo, the jump table demo, the disassembly demo and the wasm object demo.<br/>

### Examples
First you should clone the mutator repo and run `git submodule init` and `git submodule update` to get the third-party repos that enable mutator to run.<br/>
To build bruiser you can either run the makefile in bruiser's directory, then run `make` or just run the makefile at mutator's root directory and run `make bruiser`.<br/>
After building bruiser, you can run it like any other mutator tool. So for example if you want to run bruiser on its test file run:<br/>

```bash

./bruiser ../test/bruisertest/test.cpp

```

or if you're lazy like me just run the shellscript `run.sh` in bruiser's directory.<br/>

After that you can just run your commands.<br/>
To run you commands from a lua file, you can just use `dofile()` to call your script. Bruiser has an embedded lua interpreter with the bruiser functions registered in it, so you do have full access to all lua libraries and functionalities plus the added bruiser functionality.<br/>
For example you can run one of the example scripts that come with bruiser like this:<br/>

```lua

dofile("./lua-scripts/demo1.lua")

```

You can also run bruiser in non-cli mode:<br/>
```bash

./bruiser ../test/bruisertest/test.cpp -lua="./lua-scripts/demo2.lua"

```
The demo scripts, `demo1.lua` and `demo2.lua` require the file `bfd/test/test` and `bfd/test/test.so` to be built. Run make in `bfd/test/` to get `test` and `test.so`.<br/>

Bruiser requires a compilation database to run. If you don't have a compilation database, take a look at [Bear](https://github.com/rizsotto/Bear). If you're using `cmake`, just tell it to generate a compilation database.<br/>

TLDR; now let's look at some useful example.<br/>

#### ELF info, Xobjs, ASMRewriter
mutator has it's own pyelf script which resides at `/bfd`, named `load.py`. `load.py` reads an ELF file and then returns the results to lua in the form of tables. For more detailed info please look at the wiki entry.<br/>
Running the following command will return a table containing the names of the objects found in the specified ELF file. To build `../bfd/test/test.so` go to the test dir for bfd and run the makefile.<br/>
```lua
objload("elf_get_obj_names", "../bfd/test/test.so", "symbol_list")
```
For a more detailed example look at the wiki here on github.<br/>

The Xobj module along with `load.py` allows you to load a function from an ELF shared object library into executable memory and call it.<br/>
The xobj functionality is provided as a lua module. You can use it by:<br/>
```lua
xobj = require("lua-scripts.xobj")
```
For a working example on xobjs, you can run `lua-scripts/demo1.lua`. The example requires `ansicolors`. You can get that by `luarocks install ansicolors`.<br/>

The ASMRewriter functionality allows you to look through the machine code and make changes to the executable.<br/>
For working examples which demonstrate how much the implementation has improved you can run `lua-scripts/demo2.lua` and `lua-scripts/df-demo.lua`. `demo2.lua` requires `ansicolor`. `df-demo.lua` uses the dwarf fortress executable as an example so you will have to first get that and then change the path in the lua file.<br/>

For more detailed information on the modules and the methods they provide, you can look at the wiki.<br/>
