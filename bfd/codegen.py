#!/bin/python3

import argparse
import code
import readline

default_header="#include <stdint.h>\n"
main_sig="int main(int argc, char** argv)>"
def_kw="#define "
def_name = []
separator="fff"
def_value = []

class Argparser(object):
    def __init__(self):
        parser = argparse.ArgumentParser()
        parser.add_argument("--arglist", nargs="+", type=str, help="list of args")
        parser.add_argument("--hex", action="store_true", help="generate hex(string) code, otherwise generate int", default=False)
        self.args = parser.parse_args()
        self.code = {}

class CodeGen_Arg(object):
    def __init__(self, arglist):
        self.arglist = arglist
        self.def_name = []
        self.def_value = []

    def get_ret_type(self, type):
        pass

    def gen_cast(self):
        for argtype in self.arglist:
            if argtype == "int8": self.def_name.append("i8")
            elif argtype == "uint8":self.def_name.append("u8")
            elif argtype == "uchar":self.def_name.append("c")
            elif argtype == "char":self.def_name.append("c")
            elif argtype == "lightuserdata":self.def_name.append("p")
            elif argtype == "bool":self.def_name.append("b")
            elif argtype == "int16":self.def_name.append("i16")
            elif argtype == "uint16":self.def_name.append("u16")
            elif argtype == "int32":self.def_name.append("i32")
            elif argtype == "uint32":self.def_name.append("u32")
            elif argtype == "int64":self.def_name.append("i64")
            elif argtype == "uint64":self.def_name.append("u64")
            elif argtype == "int128":self.def_name.append("i128")
            elif argtype == "uint128":self.def_name.append("u128")
            elif argtype == "float":self.def_name.append("f")
            elif argtype == "double":self.def_name.append("d")
            elif argtype == "long double":self.def_name.append("ld")
            elif argtype == "string":self.def_name.append("s")
            elif argtype == "custom":self.def_name.append("x")
            else:
                raise Exception("codegen : unknown type")

    def debugdump(self):
        for argtype in self.arglist:
            print(argtype)

    def genhex():
        pass

    def genint():
        pass

# write code here
def premain():
    argparser = Argparser()
    codegen = CodeGen_Arg(argparser.args.arglist)
    codegen.debugdump()

def main():
    try:
        premain()
    except:
        variables = globals().copy()
        variables.update(locals())
        shell = code.InteractiveConsole(variables)
        shell.interact(banner="DEBUG REPL")

if __name__ == "__main__":
    main()
