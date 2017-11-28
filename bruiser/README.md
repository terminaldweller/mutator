## bruiser

### What is it?
Bruiser is implemented as an interactive commandline interface. It features an embedded lua interpreter plus the history and auto-completion and suggestion features we have all come to expect from shell-like tools.<br/>
Regarding the actual functionality, it will feature non-blind selective mutations. You can ask it to list information regrading the source codes it is run on. The eventuality of this idea is to help with maintaining code or in giving the viewer an overview of the code. The final feature is the name-sake. It looks at the code and decides how to break it. For more explanation please read on.<br/>

### How does it work?
To put it simply, bruiser is an interactive lua interpreter that uses linenoise for shell-like features(history, tab-completion, auto-suggestion). You get the full power of lua plus the bruiser functions whcih are implemented as lua scripts that call back to the cpp code to get things done.<br/>
To put this into perspective, think you run `list vars` in bruiser. It gets you the list of vars but that's it. You can't save them to a file or do anything else with them. With the old way of doing things I had to add a command that did that and then you could do it but what if you wanted to do something else? What then? Well you get the idea. That would also mean that bruiser's language would be made up gradually which would result in something ugly and warrant a lot of rewrites.<br/>
With the new way of doing things, the user is only limited by their imagination and lua, not me, and there is no learning curve for learning a garbage language that I would have to come up with.<br/>
Also, there is no reason to implement any extra features to be able to automate your use of bruiser. Just run a lua script and tell bruiser to run that.<br/>

### DSL?
Bruiser has an embedded lua interpreter so nobody would have to deal with a new DSL. It's good old lua.<br/>

### Lua vs Luajit
In the current implementation, bruiser will only support lua and not luajit. luajit is way faster than lua which will play an important role in bruiser's overall performance but luajit is generally less stable than lua and usually behind in terms of what new features of lua the language it supports.<br/>
The plan is to add both and for the user to be able to pick which one to use when running bruiser.<br/>

### Prototyping
I embedded the ability to run python scripts from C++ in bruiser. The feature was added to facilitate fast prototyping since I'd rather first do the experimental features in python and run them through bruiser and then re-implement them in C++ if speed is an actual concern.<br/>

### Warning
The current implementation loads all lua libraries which also includes it's `os` library. To give you an idea, `os.execute()` is very similar to `system()` in C. This decision has been made to speed up testing and the dev process.<br/>
Also like `mutatord` and `mutatorclient`, bruiser does not need any sudo access.<br/>

### Non-blind Selective mutation?
Bruiser looks at your code, learns your code and then decides how to mutate your code. That's non-blind selective mutation. Now onto a real explanation:<br/>
`m0` generates two sets of reports. One is the rules it checks on code which is for the better part, at the time of writing this very similar to Misra-c. The second report is the ancestry of the node that caused `m0` to tag a node in the first report. The second report is an experimental first attempt at narrowing down the parts of the code that would be better targets for mutation.<br/>
The second point concerns the mutation operators. The classical mutation operators are blind. Let me demonstrate with an example:<br/>
Imagine we have a classical mutation operator that mutates all `+` operators to `-`. This mutation operator is blind. To put it in simple terms, it takes in text and spits out text with no regards to syntax or semantics.<br/>
`bruiser` will not be using classical blind mutation operators.<br/>

### How?
I'm going to write about it as soon as I get my thoughts organized. In the meantime you can look at the source code for some hints.<br/>

### Example
First you should clone the mutator repo and run `git submodule init` and `git submodule update` to get the cool third-party repos that enable mutator to run.<br/>
To build bruiser you can either run the makefile in bruiser's directory, then run `make` or just run the makefile at mutator's root directory and run `make bruiser`.<br/>
After building bruiser, you can run it like any other mutator tool. So for example if you want to run bruiser on its test file run:<br/>

```bash

./bruiser ../test/bruisertest/test.cpp

```

After that you can just run your commands.<br/>
To run you commands from a lua file, you can just use `dofile()` to call your script. Bruiser has an embedded lua interpreter with the bruiser functions registered in it, so you do have full access to all lua libraries and functionalities plus the added bruiser functionality.<br/>
For example you can run one of the example scripts that come with bruiser like this:<br/>

```lua

dofile("./lua-scripts/testfile1.lua")

```

You can also run bruiser in non-cli mode:<br/>
```bash

./bruiser ../test/bruisertest/test.cpp -lua="./lua-scripts/mutation-example.lua"

```

Bruiser requires a compilation database to run. If you don't have a compilation database, take a look at [Bear](https://github.com/rizsotto/Bear). If you're using `cmake`, just tell it to generate a compilation database.<br/>
