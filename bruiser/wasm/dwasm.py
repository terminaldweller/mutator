#!/usr/bin/python3

import argparse
import code
import readline
import signal
import sys
from parse import premain
from utils import ParseFlags

def SigHandler_SIGINT(signum, frame):
    print()
    sys.exit(0)

class Argparser(object):
    def __init__(self):
        parser = argparse.ArgumentParser()
        parser.add_argument("--wast", type=str, help="path to the wasm text file")
        parser.add_argument("--wasm", type=str, nargs='+', help="path to the wasm object file")
        parser.add_argument("--asb", type=str, help="path to the wast file to assemble")
        parser.add_argument("--dis", type=str, help="path to the wasm file to disassemble")
        parser.add_argument("-o", type=str, help="the path to the output file")
        parser.add_argument("--dbg", action='store_true', help="print debug info", default=False)
        parser.add_argument("--unval", action='store_true', help="skips validation tests", default=False)
        parser.add_argument("--memdump", type=int, help="dumps the linear memory")
        parser.add_argument("--idxspc", action='store_true', help="print index space data", default=False)
        parser.add_argument("--run", action='store_true', help="runs the start function", default=False)
        parser.add_argument("--metric", action='store_true', help="print metrics", default=False)
        parser.add_argument("--gas", action='store_true', help="print gas usage", default=False)
        parser.add_argument("--entry", type=str, help="name of the function that will act as the entry point into execution")
        parser.add_argument("--link", type=str, nargs="+", help="link the following wasm modules")
        parser.add_argument("--sectiondump", type=str, help="dumps the section provided")
        parser.add_argument("--hexdump", type=int, help="dumps all sections")
        parser.add_argument("--dbgsection", type=str, help="dumps the parsed section provided", default="")
        parser.add_argument("--interactive", action='store_true', help="open in cli mode", default=False)
        parser.add_argument("--rawdump", type=int, nargs=2, help="dumps all sections")
        self.args = parser.parse_args()
        if self.args.wasm is not None and self.args.wast is not None:
            raise Exception("the --wast option and the --wasm option cannot\
                            be set at the same time. you need to choose one.")

    def getParseFlags(self):
        return(ParseFlags(self.args.wast, self.args.wasm, self.args.asb, self.args.dis,
                          self.args.o, self.args.dbg, self.args.unval, self.args.memdump,
                          self.args.idxspc, self.args.run, self.args.metric, self.args.gas, self.args.entry))

def main():
    signal.signal(signal.SIGINT, SigHandler_SIGINT)
    argparser = Argparser()
    if argparser.args.dbg:
        try:
            premain(argparser)
        except Exception as e:
            print(e.__doc__)
            if e.message: print(e.message)
            variables = globals().copy()
            variables.update(locals())
            shell = code.InteractiveConsole(variables)
            shell.interact(banner="DEVIWASM REPL")
    else:
        premain(argparser)

if __name__ == "__main__":
    main()
