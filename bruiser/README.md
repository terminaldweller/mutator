## bruiser

### What is it?
Bruiser is implemented as an interactive commandline interface. It features an embedded lua interpreter plus the history and auto-completion and suggestion features we have all come to expect from shell-like tools.<br/>
Regarding the actual functionality, it will feature non-blind selective mutations. You can ask it to list information regrading the source codes it is run on. The eventuality of this idea is to help with maintaining code or in giving the viewer an overview of the code. The final feature is the name-sake. It looks at the code and decides how to break it. For more explanation please read on.<br/>

### How it Works?
To put it simply, bruiser is an interactive lua interpreter that uses linenoise for shell-like features(history, tab-completion, auto-suggestion). You get the full power of lua plus the bruiser functions whcih are implemented as lua scripts that call back to the cpp code to get things done.<br/>
To put this into perspecttive, think you run `list vars` in bruiser. It gets you the list of vars but that's it. You can't save them to a file or do anything else with them. With the old way of doing things I had to add a command that did that and then you could do it but what if you wanted to do something else? what then? well you get the idea. That would also mean that bruiser's language would be made up gradually which would result in something ugly and warrant a lot of rewrites.<br/>
With the new way of doing things, the user is only limited by their imagination and lua, not me, and there is no learning curve for learning a garbage language that I would have to come up with.<br/>
Also, there is no reason to implement any extra features to be able to automate your use of bruiser. just run a lua script and tell bruiser to run that.<br/>

### DSL?
bruiser has an embedded lua interpreter so nobody would have to deal with a new DSL. It's good old lua.<br/>

### Lua vs Luajit
In the current implementation, bruiser will only support lua and not luajit. luajit is way faster than lua which will play an important role in bruiser's overall performance but luajit is generally less stable than lua and usually behind in terms of what new features of lua the language it supports.<br/>
The plan is to add both and for the user to be able to pick which one to use when running bruiser.<br/>

### Warning
The current implementation loads all lua libraries which also includes it's `os` library. To give you an idea, `os.execute()` is very similar to `system()` in C. This decision has been made to speed up testing and the dev process.<br/>
Also like `mutatord` and `mutatorclient`, bruiser does not need any sudo access.<br/>

### Non-blind Selective mutation?
bruiser looks at your code, learns your code and then decides how to mutate your code. That's non-blind selective mutation.<br/>

### How?
I'm going to wrire about it as soon as I get my thoughts organized. In the meantime you can look at the source code for some hints.<br/>
