#!/usr/bin/python3

import argparse
import code
import readline
import signal
import sys
from load import premain, CLIArgParser, SigHandler_SIGINT

def main():
    argparser = CLIArgParser()
    if argparser.args.dbg:
        try:
            premain(argparser)
        except Exception as e:
            print(e.__doc__)
            if e.message: print(e.message)
            signal.signal(signal.SIGINT, SigHandler_SIGINT)
            variables = globals().copy()
            variables.update(locals())
            shell = code.InteractiveConsole(variables)
            shell.interact(banner="DELF REPL")
    else:
        premain(argparser)

if __name__ == "__main__":
    main()
