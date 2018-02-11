#!/bin/python3

import argparse
import code
import readline
import signal
import sys
import capstone
from load import ELF

def SigHandler_SIGINT(signum, frame):
    print()
    sys.exit(0)

class Argparser(object):
    def __init__(self):
        parser = argparse.ArgumentParser()
        parser.add_argument("--arglist", nargs="+", type=str, help="list of args")
        parser.add_argument("--hex", action="store_true", help="generate hex(string) code, otherwise generate int", default=False)
        self.args = parser.parse_args()
        self.code = {}

class Call_Rewriter(object):
    def __init__(self, obj_code, arch, mode):
        self.obj_code = obj_code
        #self.md = Cs(CS_ARCG_X86, CS_MODE_64)
        self.md = Cs(arch, mode)

    def run():
        for i in md.disasm(self.obj_code, 0x0):
            print("0x%x:\t%s\t%s" %(i.address, i.mnemonic, i.op_str))

class Global_Rewriter(object):
    def __init__(self):
        pass

# Main is here
def premain():
    signal.signal(signal.SIGINT, SigHandler_SIGINT)
    argparser = Argparser()
    # write code here

###############################################################################
def main():
    try:
        premain()
    except:
        variables = globals().copy()
        variables.update(locals())
        shell = code.InteractiveConsole(variables)
        shell.interact(banner="CALL REWRITER DEBUG REPL")

if __name__ == "__main__":
    main()
