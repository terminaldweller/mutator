# mutator daemon/server/client

This document talks about mutator's daemon, client and server.<br/>
Please keep in mind that this feature is still under development. That means that it's still rough around the edges and produces a lot of diagnostic messages.<br/>
With all that being said, it works so feel free to try it out.<br/>

### How it works?

It's simple. The client sends a command to the server run by the daemon(`mutatord`) to execute. The server then runs the command. by passing it to the driver(`mutator.sh`).<br/>
The mutator client is a thin client which is also the whole point of all of this: Plugin.<br/>

The daemon generates a log file in the `daemon` directory under root named `mutatordlog`.<br/>

### How to use it?

Just run the makefile in the `daemon` directory:<br/>

```bash

make all

```

After that, run `mutatord` to start up the server and then run `mutatorclient` and send your commands.<br/>
The server passes the commands to `mutator.sh` to execute. For a list of available options you can run `mutator.sh -h` or just read the `README.md` in project root.<br/>A
I have yet to decide how to set a home path variable for mutator so you need to pass all adresses to the server as absolute paths since the daemon changes the directory to `/`.<br/>

To kill the client and server(and also the daemon) just send `end_comm` as the command through the client.<br/>

## WARNING

Currently there are no checks on the commands sent to the server. The server uses `popen()` to run the commands so it will run any valid `sh` command it gets from `/` and the buffers are not exactly small either.<br/>

### Directory Content

* `mutatord.c` holds the source code for the daemon.<br/>
* `daemon_aux.c` contains the source code for the server run by the daemon.<br/>
* `mutatorclient` contains the source code for the client.<br/>
* `mutatorserver` contains the server as a standlone. This is only for testing.<br/>
* `README.md` is the thing you are reading right now.<br/>
* `makefile` builds the client/server/daemon.<br/>

### Limitations

* The server uses only one pipe so the only thing captured is stdout. I'll later add another pipe to capture stderr.<br/>
* For the time being the server can only accept one client. I'll add code to handle more clients if the need is felt.<br/>
* Currently the client(daemon) and server need to be on the same host. Again this limitation could be removed if it is ever needed.<br/>
