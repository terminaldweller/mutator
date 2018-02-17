#!/bin/python3

import argparse
import code
import readline
import signal
import sys
from load import *

quad_code = [85, 72, 137, 229, 72, 131, 236, 32, 137, 125, 252, 137, 117, 248,
             137, 85, 244, 137, 77, 240, 139, 125, 252, 139, 117, 248, 232, 209,
             253, 255, 255, 139, 125, 244, 139, 117, 240, 137, 69, 236, 232, 195,
             253, 255, 255, 139, 77, 236, 1, 193, 137, 200, 72, 131, 196, 32, 93, 195]

def SigHandler_SIGINT(signum, frame):
    print()
    sys.exit(0)

class Argparser(object):
    def __init__(self):
        parser = argparse.ArgumentParser()
        parser.add_argument("--string", type=str, help="string")
        parser.add_argument("--obj", type=str, help="object path")
        parser.add_argument("--bool", action="store_true", help="bool", default=False)
        self.args = parser.parse_args()

def premain():
    check1 = bool()
    signal.signal(signal.SIGINT, SigHandler_SIGINT)
    argparser = Argparser()
    so = openSO_r(argparser.args.obj)
    elf = ELF(so)
    elf.init(64)
    x = elf.dump_symbol_string(ELF_ST_TYPE.STT_FUNC, False)
    for name in x:
        if name == "quad": check1 = True
    if not check1: sys.exit(1)
    y = elf.dump_symbol_string(ELF_ST_TYPE.STT_OBJECT,False)
    z = elf.dump_obj_size(ELF_ST_TYPE.STT_OBJECT, False)
    w = elf.dump_funcs(False)
    for i in range(0, len(quad_code)):
        if w[10][i] != quad_code[i]: sys.exit(1)

def main():
    #try:
    premain()
    #except:
    '''
    variables = globals().copy()
    variables.update(locals())
    shell = code.InteractiveConsole(variables)
    shell.interact(banner="DEBUG REPL")
    '''
    pass

if __name__ == "__main__":
    main()
