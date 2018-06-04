#!/usr/bin/python3

import argparse
import code
import json
import os
import readline
import signal
import sys

C_STRUCT = ['typedef struct XXX {', '}XXX;']
HEADER_GUARD = ['\n#ifndef _XXX_H\n#define _XXX_H\n', '#endif //end of inclusion guard\n\n']
EXTERN_C = ['#ifdef __cplusplus\nextern "C" {\n#endif\n', '#ifdef __cplusplus\n}\n#endif //end of extern c\n']
BEGIN_NOTE = "//Generated Automatically by luatablegen."
HEADER_LIST = ['#include "HHHlua.h"\n', '#include "HHHlauxlib.h"\n',
               '#include "HHHlualib.h"\n', '#include <inttypes.h>\n',
               '#include <stdbool.h>']
CONVERT = ['static XXX* convert_XXX (lua_State* __ls, int index) {\n',
           '\tXXX* dummy = (XXX*)lua_touserdata(__ls, index);\n',
           '\tif (dummy == NULL) printf("XXX:bad user data type.\\n");\n',
           '\treturn dummy;\n}\n']
CHECK = ['static XXX* check_XXX(lua_State* __ls, int index) {\n',
        '\tXXX* dummy;\n',
        '\tluaL_checktype(__ls, index, LUA_TUSERDATA);\n'
        '\tdummy = (XXX*)luaL_checkudata(__ls, index, "XXX");\n',
        '\tif (dummy == NULL) printf("XXX:bad user data type.\\n");\n',
        '\treturn dummy;\n}\n']
PUSH_SELF = [ 'XXX* push_XXX(lua_State* __ls) {\n',
            '\tlua_checkstack(__ls, 1);\n',
            '\tXXX* dummy = lua_newuserdata(__ls, sizeof(XXX));\n',
            '\tluaL_getmetatable(__ls, "XXX");\n',
            '\tlua_setmetatable(__ls, -2);\n',
            '\treturn dummy;\n}\n']
PUSH_ARGS = ['int XXX_push_args(lua_State* __ls, XXX* _st) {\n',
             '\tlua_checkstack(__ls, NNN);\n', '\treturn 0;\n}\n']
NEW = ['int new_XXX(lua_State* __ls) {\n', '\tlua_checkstack(__ls, NNN);\n',
       '\tXXX* dummy = push_XXX(__ls);\n', '\treturn 1;\n}\n']
GETTER_GEN = ['static int getter_XXX_YYY(lua_State* __ls) {\n',
              '\tXXX* dummy = check_XXX(__ls, 1);\n',
              '\tlua_pop(__ls, -1);\n',
              '\treturn 1;\n}\n']
SETTER_GEN = ['static int setter_XXX_YYY(lua_State* __ls) {\n',
              '\tXXX* dummy = check_XXX(__ls, 1);\n',
              '\tlua_settop(__ls, 1);\n',
              '\treturn 1;\n}\n']
REGISTER_TABLE_METHODS = ['static const luaL_Reg XXX_methods[] = {\n',
                          '\t{0,0}\n};\n']
REGISTER_META = ['static const luaL_Reg XXX_meta[] = {\n',
                 '\t{0, 0}\n};\n']
TABLE_REGISTER =  ['int XXX_register(lua_State* __ls) {\n',
  '\tluaL_openlib(__ls, "XXX", XXX_methods, 0);\n',
  '\tluaL_newmetatable(__ls, "XXX");\n',
  '\tluaL_openlib(__ls, 0, XXX_meta, 0);\n',
  '\tlua_pushliteral(__ls, "__index");\n',
  '\tlua_pushvalue(__ls, -3);\n',
  '\tlua_rawset(__ls, -3);\n',
  '\tlua_pushliteral(__ls, "__metatable");\n',
  '\tlua_pushvalue(__ls, -3);\n',
  '\tlua_rawset(__ls, -3);\n',
  '\tlua_pop(__ls, 1);\n',
  'return 1;\n}\n']
SOURCE_FILE_NAME='XXX_luatablegen.c'
HEADER_FILE_NAME='XXX_luatablegen.h'

def SigHandler_SIGINT(signum, frame):
    print()
    sys.exit(0)

class Argparser(object):
    def __init__(self):
        parser = argparse.ArgumentParser()
        parser.add_argument("--out", type=str, help="output directory")
        parser.add_argument("--tbg", type=str, help="the table gen file")
        parser.add_argument("--pre", type=str, help="path to source code file to add after header guard/extern c")
        parser.add_argument("--post", type=str, help="path to source code file to add before header guard/extern c end")
        parser.add_argument("--luaheader", type=str, help="path to lua header files")
        parser.add_argument("--dbg", action="store_true", help="debug", default=False)
        self.args = parser.parse_args()

class TbgParser(object):
    def __init__(self, tbg, out, argparser):
        self.tbg_file = json.load(open(tbg))
        self.argparser = argparser

    def begin(self, c_source, h_filename, struct_name):
        for header in HEADER_LIST:
            if self.argparser.args.luaheader:
                c_source.write(header.replace("HHH", self.argparser.args.luaheader+"/"))
            else:
                c_source.write(header.replace("HHH", ""))
        c_source.write(HEADER_GUARD[0].replace("XXX", struct_name))
        c_source.write(EXTERN_C[0])
        #c_source.write('#include "./'+h_filename+'"\n')
        c_source.write("\n")
        if self.argparser.args.pre:
            pre_file = open(self.argparser.args.pre)
            for line in pre_file:
                c_source.write(line)
        c_source.write("\n")
        pre_file.close()

    def struct(self, c_source, field_names, field_types, struct_name):
        c_source.write("typedef struct {\n")
        for field_type, field_name in zip(field_types, field_names):
            c_source.write("\t" + field_type + " " + field_name + ";\n")
        c_source.write("}" +struct_name+ ";\n")
        c_source.write("\n")

    def convert(self, c_source, struct_name):
        for line in CONVERT:
            c_source.write(line.replace("XXX", struct_name))
        c_source.write("\n")

    def check(self, c_source, struct_name):
        for line in CHECK:
            c_source.write(line.replace("XXX", struct_name))
        c_source.write("\n")

    def push_self(self, c_source, struct_name):
        for line in PUSH_SELF:
            c_source.write(line.replace("XXX", struct_name))
        c_source.write("\n")

    def push_args(self, c_source, struct_name, field_names, lua_types):
        dummy = str()
        c_source.write(PUSH_ARGS[0].replace("XXX", struct_name))
        c_source.write("\tlua_checkstack(__ls, " + repr(len(field_names)) + ");\n")
        for field_name, lua_type in zip(field_names, lua_types):
            if lua_type == "integer": dummy = "\tlua_pushinteger(__ls, _st->"+field_name+");\n"
            elif lua_type == "lightuserdata": dummy = "\tlua_pushlightuserdata(__ls, _st->"+field_name+");\n"
            elif lua_type == "number": dummy = "\tlua_pushnumber(__ls, _st->"+field_name+");\n"
            elif lua_type == "string": dummy = "\tlua_pushstring(__ls, _st->"+field_name+");\n"
            elif lua_type == "boolean": dummy = "\tlua_pushboolean(__ls, _st->"+field_name+");\n"
            else:
                print("bad lua_type entry in the json file")
                sys.exit(1)
            c_source.write(dummy)
            dummy = str()
        c_source.write(PUSH_ARGS[2])
        c_source.write("\n")

    def new(self, c_source, struct_name, field_types, field_names, lua_types):
        dummy = str()
        rev_counter = -len(field_types)
        c_source.write(NEW[0].replace("XXX", struct_name))
        c_source.write("\tlua_checkstack(__ls, " + repr(len(field_names)) + ");\n")
        for lua_type, field_name, field_type in zip(lua_types, field_names, field_types):
            if lua_type == "integer": dummy = "\t"+field_type +" "+field_name+" = "+"luaL_optinteger(__ls,"+repr(rev_counter)+",0);\n"
            elif lua_type == "lightuserdata": dummy = "\t"+field_type +" "+field_name+" = "+"lua_touserdata(__ls,"+repr(rev_counter)+");\n"
            elif lua_type == "number": pass
            elif lua_type == "string":dummy = "\t"+field_type +" "+field_name+" = "+"lua_tostring(__ls,"+repr(rev_counter)+",0);\n"
            elif lua_type == "boolean": pass
            else:
                print("bad lua_type entry in the json file")
                sys.exit(1)
            rev_counter += 1
            c_source.write(dummy)
            dummy = str()
        c_source.write(NEW[2].replace("XXX", struct_name))
        for field_name in field_names:
            c_source.write("\tdummy->" + field_name + " = " + field_name + ";\n")
        c_source.write(NEW[3].replace("XXX", struct_name))
        c_source.write("\n")

    def getter(self, c_source, struct_name, field_names, field_types, lua_types):
        dummy = str()
        for field_name, lua_type in zip(field_names, lua_types):
            c_source.write(GETTER_GEN[0].replace("XXX", struct_name).replace("YYY", field_name))
            c_source.write(GETTER_GEN[1].replace("XXX", struct_name))
            c_source.write(GETTER_GEN[2])
            if lua_type == "integer": dummy = "\tlua_pushinteger(__ls, dummy->"+field_name+");\n"
            elif lua_type == "lightuserdata": dummy = "\tlua_pushlightuserdata(__ls, dummy->"+field_name+");\n"
            elif lua_type == "number": dummy = "\tlua_pushnumber(__ls, dummy->"+field_name+");\n"
            elif lua_type == "string": dummy = "\tlua_pushstring(__ls, dummy->"+field_name+");\n"
            elif lua_type == "boolean": dummy = "\tlua_pushboolean(__ls, dummy->"+field_name+");\n"
            else:
                print("bad lua_type entry in the json file")
                sys.exit(1)
            c_source.write(dummy)
            dummy = str()
            c_source.write(GETTER_GEN[3])
        c_source.write("\n")

    def setter(self, c_source, struct_name, field_names, field_types, lua_types):
        dummy = str()
        for field_name, lua_type in zip(field_names, lua_types):
            c_source.write(SETTER_GEN[0].replace("XXX", struct_name).replace("YYY", field_name))
            c_source.write(SETTER_GEN[1].replace("XXX", struct_name))
            if lua_type == "integer": dummy = "\tdummy->" + field_name + " = " + "luaL_checkinteger(__ls, 2);\n"
            elif lua_type == "lightuserdata": dummy ="\tdummy->" + field_name + " = " + "luaL_checkudata(__ls, 2, "+'"'+struct_name+'"'+");\n"
            elif lua_type == "number": dummy ="\tdummy->" + field_name + " = " + "luaL_checknumber(__ls, 2);\n"
            elif lua_type == "string": dummy ="\tdummy->" + field_name + " = " + "luaL_checkstring(__ls, 2);\n"
            elif lua_type == "boolean": pass
            else:
                print("bad lua_type entry in the json file")
                sys.exit(1)
            c_source.write(dummy)
            dummy = str()
            c_source.write(SETTER_GEN[2])
            c_source.write(SETTER_GEN[3])
        c_source.write("\n")

    def gc(self):
        pass
    def tostring(self):
        pass

    def register_table_methods(self, c_source, struct_name, field_names):
        c_source.write(REGISTER_TABLE_METHODS[0].replace("XXX", struct_name))
        c_source.write('\t{"new", ' + "new_" + struct_name + "},\n")
        for field_name in field_names:
            c_source.write("\t{" + '"set_' + field_name + '"' + ", " + "setter_"+struct_name +"_"+ field_name + "},\n")
        for field_name in field_names:
            c_source.write("\t{" + '"' + field_name + '", ' + "getter_"+struct_name+"_"+field_name+"},\n")
        c_source.write(REGISTER_TABLE_METHODS[1])
        c_source.write("\n")

    def register_table_meta(self, c_source, struct_name):
        c_source.write(REGISTER_META[0].replace("XXX", struct_name))
        c_source.write(REGISTER_META[1])
        c_source.write("\n")

    def register_table(self, c_source, struct_name):
        for line in TABLE_REGISTER:
            c_source.write(line.replace("XXX", struct_name))

    def end(self, c_source):
        if self.argparser.args.post:
            c_source.write("\n")
            post_file = open(self.argparser.args.post)
            for line in post_file:
                c_source.write(line)
        c_source.write("\n")
        c_source.write(EXTERN_C[1])
        c_source.write(HEADER_GUARD[1])
        c_source.write("\n")

    def run(self):
        for k, v in self.tbg_file.items():
            struct_name = k
            field_names = v['field_name']
            field_types = v['field_type']
            lua_types = v['lua_type']
            methods = v['methods']
            c_filename = struct_name + "_tablegen.h"
            h_filename = struct_name + "_tablegen.h"
            if self.argparser.args.out[-1] == "/":
                c_source = open(self.argparser.args.out + c_filename, "w")
            else:
                c_source = open(self.argparser.args.out + "/" + c_filename, "w")
            self.begin(c_source, h_filename, struct_name)
            self.struct(c_source, field_names, field_types, struct_name)
            self.convert(c_source, struct_name)
            self.check(c_source, struct_name)
            self.push_self(c_source, struct_name)
            self.push_args(c_source, struct_name, field_names, lua_types)
            self.new(c_source, struct_name, field_types, field_names, lua_types)
            self.getter(c_source, struct_name, field_names, field_types, lua_types)
            self.setter(c_source, struct_name, field_names, field_types, lua_types)
            self.register_table_methods(c_source, struct_name, field_names)
            self.register_table_meta(c_source, struct_name)
            self.register_table(c_source, struct_name)
            self.end(c_source)
            c_source.close()

# write code here
def premain(argparser):
    signal.signal(signal.SIGINT, SigHandler_SIGINT)
    #here
    parser = TbgParser(argparser.args.tbg, argparser.args.out, argparser)
    parser.run()

def main():
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
            shell.interact(banner="DEBUG REPL")
    else:
        premain(argparser)

if __name__ == "__main__":
    main()
