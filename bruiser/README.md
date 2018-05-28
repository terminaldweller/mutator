## bruiser

### What is it?
Bruiser is implemented as an interactive commandline interface. It features an embedded Lua 5.3.4 interpreter plus the history and auto-completion and suggestion features we have all come to expect from shell-like tools.<br/>
Regarding the actual functionality:<br/>
Xobj: pull in funtions from ELF objects, call them and get the result back.<br/>
ASMrewriter: Allows manipulation of machine code.<br/>
It will feature non-blind selective mutations. You can ask it to list information regrading the source codes it is run on. The eventuality of this idea is to help with maintaining code or in giving the viewer an overview of the code. The final feature is the name-sake. It looks at the code and decides how to break it. For more explanation please read on.<br/>

For working demos you can skip to the end of the README.<br/>

### Building
Running `make` from bruiser's make or `make bruiser` from the main makefile in the root directory of mutator will take care of that given that you already have all the requirements taken care of.<br/>
It is generally a good idea to run `make deepclean` on bruiser's makefile on every pull since I occasionally have to make changes to Lua's sources or makefile.<br/>

### How does it work?
To put it simply, bruiser is an interactive lua interpreter that uses linenoise for shell-like features(history, tab-completion, auto-suggestion). You get the full power of lua plus the bruiser functions whcih are implemented as lua scripts that call back to the cpp code to get things done.<br/>
To put this into perspective, think you run `list vars` in bruiser. It gets you the list of vars but that's it. You can't save them to a file or do anything else with them. With the old way of doing things I had to add a command that did that and then you could do it but what if you wanted to do something else? What then? Well you get the idea. That would also mean that bruiser's language would be made up gradually which would result in something ugly and warrant a lot of rewrites.<br/>
With the new way of doing things, the user is only limited by their imagination and lua, not me, and there is no learning curve for learning a garbage language that I would have to come up with.<br/>
Also, there is no reason to implement any extra features to be able to automate your use of bruiser. Just run a lua script and tell bruiser to run that.<br/>
bruiser has a built-in pipe to Python so adding plugin python scripts are simple.(currently the pipe works only one-way)<br/>

### Lua vs Luajit
For the first incarnation, bruiser will only support lua and not luajit. luajit is way faster than lua which will play an important role in bruiser's overall performance but luajit is generally less stable than lua and usually behind in terms of what new features of lua the language it supports.<br/>
The plan is to add both and for the user to be able to pick which one to use when running bruiser. Unfortunately there is no estimated date.<br/>

### Warning
The current implementation loads all lua libraries which also includes it's `os` library. To give you an idea, `os.execute()` is very similar to `system()` in C. This decision has been made to speed up testing and the dev process.<br/>
Also like `mutatord` and `mutatorclient`, bruiser does not need any sudo access.<br/>

### Useful Lua Scripts
The dir named `lua-scripts` houses demos, examples and useful lua scripts for bruiser.<br/>
If you happen to write a Lua script for bruiser that you think other people will find useful, then please add it to `lua-scripts` on your fork and make a PR.<br/>

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

#### Lua Defaults
Upon start-up, bruiser will look to find a file called `defaults.lua` in the same directory as the bruiser executable to run before running any user provided lua code, both in interactive and non-interactive modes. The path to the lua default file could be changed from the default value by the `LuaDefault` option passed to bruiser on startup.<br/>
The default script provided will run `luarocks path --bin` and add `paht` and `cpath` so that you can use your Lua modules from bruiser.<br/>
