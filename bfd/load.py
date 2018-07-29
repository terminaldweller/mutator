#!/usr/bin/python3
#***************************************************Project Mutator****************************************************/
# yet another elfdump in python
#*Copyright (C) 2018 Farzad Sadeghi

#This program is free software; you can redistribute it and/or
#modify it under the terms of the GNU General Public License
#as published by the Free Software Foundation; either version 3
#of the License, or (at your option) any later version.

#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.

#You should have received a copy of the GNU General Public License
#along with this program; if not, write to the Free Software
#Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.*/
#**********************************************************************************************************************/
import argparse
import sys
import readline
import code
import signal
import os
import sys
import shutil
from capstone import *
from capstone.x86 import *

class ELF_TYPE_SIZES:
    ELF32_HALF = 2
    ELF64_HALF = 2
    ELF32_WORD = 4
    ELF32_SWORD = 4
    ELF64_WORD = 4
    ELF64_SWORD = 4
    ELF32_XWORD = 8
    ELF32_SXWORD = 8
    ELF64_XWORD = 8
    ELF64_SXWORD = 8
    ELF32_ADDR = 4
    ELF64_ADDR = 8
    ELF32_OFF = 4
    ELF64_OFF = 8
    ELF32_SECTION = 2
    ELF64_SECTION = 2

def SigHandler_SIGINT(signum, frame):
    print()
    sys.exit(0)

class ExceptionHandler(object):
    def __init__(self, globals, locals):
        self.variables = globals().copy()
        self.variables.update(locals())
        self.shell = code.InteractiveConsole()
        shell.interact(banner="Object Loader Prototype")

class CLIArgParser(object):
    def __init__(self):
        parser = argparse.ArgumentParser()
        parser.add_argument("--dbg", action="store_true", help="debug", default=False)
        parser.add_argument("--obj", type=str, help="path to the executbale, shared object or object you want to load in bruiser")
        parser.add_argument("--header", action='store_true', help="dump headers", default=False)
        parser.add_argument("--symboltable", action='store_true', help="dump symbol table", default=False)
        parser.add_argument("--phdrs", action='store_true', help="dump program haeders", default=False)
        parser.add_argument("--shdrs", action='store_true', help="dump section haeders", default=False)
        parser.add_argument("--symbolindex", action='store_true', help="dump symbol index", default=False)
        parser.add_argument("--stentries", action='store_true', help="dump section table entries", default=False)
        parser.add_argument("--objcode", action='store_true', help="dump objects", default=False)
        parser.add_argument("--test", action='store_true', help="test switch", default=False)
        parser.add_argument("--test2", action='store_true', help="test switch 2", default=False)
        parser.add_argument("--funcs", action='store_true', help="dump functions", default=False)
        parser.add_argument("--objs", action='store_true', help="dump objects", default=False)
        parser.add_argument("--dynsym", action='store_true', help="dump dynamic symbol table", default=False)
        parser.add_argument("--dlpath", action='store_true', help="dump dynamic linker path", default=False)
        parser.add_argument("--phdynent", action='store_true', help="dump ph PT_DYNAMIC entries", default=False)
        parser.add_argument("--section", type=str, help="dump a section")
        parser.add_argument("--dumpfunc", type=str, help="dump a functions machine code")
        parser.add_argument("--dumpfuncasm", type=str, help="dump a functions assembly code")
        parser.add_argument("--textasm", action='store_true', help="disassemble the text section", default=False)
        parser.add_argument("--dynsecents", action='store_true', help="dynamic section entries", default=False)
        parser.add_argument("--reladyn", action='store_true', help=".rela.dyn entries", default=False)
        parser.add_argument("--relaplt", action='store_true', help=".rela.plt entries", default=False)
        parser.add_argument("--rodata", action='store_true', help="dump .rodata", default=False)
        parser.add_argument("--disass", type=str, help="disassembls a section by name in section headers")
        parser.add_argument("--disassp", type=int, help="disassembls a section by index in program headers")
        parser.add_argument("--got", action="store_true", help="dump .got section", default=False)
        parser.add_argument("--gotplt", action="store_true", help="dump .got.plt section",default=False)
        self.args = parser.parse_args()
        if self.args.obj is None:
            raise Exception("no object file provided. please specify an object with --obj.")

def byte2int(value,sign = False):
    return int.from_bytes(value, byteorder="little", signed=sign)

def byte2hex(value):
    return hex(int.from_bytes(value, byteorder="little", signed=False))

def LEB128UnsignedDecode(bytelist):
    result = 0
    shift = 0
    for byte in bytelist:
        result |= (byte & 0x7f) << shift
        if (byte & 0x80) == 0:
            break
        shift += 7
    return(result)

def LEB128SignedDecode(bytelist):
    result = 0
    shift = 0
    for byte in bytelist:
        result |= (byte & 0x7f) << shift
        last_byte = byte
        shift += 7
        if (byte & 0x80) == 0:
            break
    if last_byte & 0x40:
        result |= - (1 << shift)
    return(result)

def LEB128UnsignedEncode(int_val):
    if int_val < 0:
        raise Exception("value must not be negative")
    elif int_val == 0:
        return bytes([0])
    byte_array = bytearray()
    while int_val:
        byte = int_val & 0x7f
        byte_array.append(byte | 0x80)
        int_val >>= 7
    byte_array[-1] ^= 0x80
    return(byte_array)

def LEB128SignedEncode(int_val):
    byte_array = bytearray()
    while True:
        byte = int_val & 0x7f
        byte_array.append(byte | 0x80)
        int_val >>= 7
        if (int_val == 0 and byte&0x40 == 0) or (int_val == -1 and byte&0x40):
            byte_array[-1] ^= 0x80
            break
    return(byte_array)

class ELF_REL():
    def __init__(self, r_offset, r_info):
        self.r_offset = r_offset
        self.r_info = r_info

class ELF_RELA():
    def __init__(self, r_offset, r_info, r_addend):
        self.r_offset = r_offset
        self.r_info = r_info
        self.r_addend = r_addend

def ffs(offset,header_list, numbered, *args):
    cn = Colors.green
    ch = Colors.cyan
    cd = Colors.blue
    cb = Colors.BOLD
    ci = Colors.red
    ce = Colors.ENDC
    max_column_width = []
    lines = []
    numbers_f = []
    dummy = []

    if numbered:
        numbers_f.extend(range(1, len(args[-1])+1))
        max_column_width.append(max([len(repr(number)) for number in numbers_f]) if  numbers_f else 6)
        header_list.insert(0, "idx")

    for arg in args:
        max_column_width.append(max([len(repr(argette)) for argette in arg]) if arg else 6)

    index = range(0, len(header_list))
    for header, width, i in zip(header_list, max_column_width, index):
        max_column_width[i] = max(len(header), width) + offset

    for i in index:
        dummy.append(ch + cb + header_list[i].ljust(max_column_width[i]) + ce)
    lines.append("".join(dummy))
    dummy.clear()

    index2 = range(0, len(args[-1]))
    for i in index2:
        if numbered:
            dummy.append(ci+cb+repr(i).ljust(max_column_width[0])+ce)
            for arg, width in zip(args, max_column_width[1:]):
                dummy.append(cd+repr(arg[i]).ljust(width)+ce)
        else:
            for arg, width in zip(args, max_column_width):
                dummy.append(cd+repr(arg[i]).ljust(width)+ce)
        lines.append("".join(dummy))
        dummy.clear()
    return lines

def get_section_type_string(number):
    if number == 0x0: return "NULL"
    if number == 0x1: return "PROGBITS"
    if number == 0x2: return "SYMTAB"
    if number == 0x3: return "STRTAB"
    if number == 0x4: return "RELA"
    if number == 0x5: return "HASH"
    if number == 0x6: return "DYNAMIC"
    if number == 0x7: return "NOTE"
    if number == 0x8: return "NOBITS"
    if number == 0x9: return "REL"
    if number == 0xa: return "SHLIB"
    if number == 0xb: return "DYNSYM"
    if number == 0xe: return "INIT_ARRAY"
    if number == 0xf: return "FINI_ARRAY"
    if number == 0x10: return "PREINIT"
    if number == 0x11: return "GROUP"
    if number == 0x12: return "SYMTAB"
    if number == 0x13: return "NUM"
    if number == 0x60000000: return "LOOS"
    if number == 0x6ffffff6: return "GNU_HASH"
    if number == 0x6fffffff: return "VERSYM"
    if number == 0x6ffffffe: return "VERNEED"

class sh_type_e:
    SHT_NULL = 0x0
    SHT_PROGBITS = 0x1
    SHT_SYMTAB = 0x2
    SHT_STRTAB = 0x3
    SHT_RELA = 0x4
    SHT_HASH = 0x5
    SHT_DYNAMIC = 0x6
    SHT_NOTE = 0x7
    SHT_NOBITS = 0x8
    SHT_REL = 0x9
    SHT_SHLIB = 0xa
    SHT_DYNSYM = 0xb
    SHT_INIT_ARRAY = 0xe
    SHT_FINI_ARRAY = 0xf
    SHT_PREINIT = 0x10
    SHT_GROUP = 0x11
    SHT_SYMTAB_SHNDX = 0x12
    SHT_NUM = 0x13
    SHT_LOOS = 0x60000000
    GNU_HASH = 0x6ffffff6
    VERSYM = 0x6fffffff
    VERNEED= 0x6ffffffe

class sh_flags_e:
    SHF_WRITE = 0x1
    SHF_ALLOC = 0x2
    SHF_EXECINSTR = 0x4
    SHF_MERGE = 0x10
    SHF_STRINGS = 0x20
    SHF_INFO_LINK = 0x40
    SHF_LINK_ORDER = 0x80
    SHF_OS_NONCONFORMING = 0x100
    SHF_GROUP = 0x200
    SHF_TLS = 0x400
    SHF_MASKOS = 0x0ff00000
    SHF_MASKPROC = 0xf0000000
    SHF_ORDERED = 0x4000000
    SHF_EXCLUDE = 0x8000000

class p_type_e:
    PT_NULL = 0x0
    PT_LOAD = 0x1
    PT_DYNAMIC = 0x2
    PT_INTERP = 0x3
    PT_NOTE = 0x4
    PT_SHLIB = 0x5
    PT_PHDR = 0x6
    PT_LOOS = 0x60000000
    PT_HIOS = 0x6FFFFFFF
    PT_LOPROC = 0x70000000
    PT_HIPROC = 0x7FFFFFFF
    GNU_EH_FRAME = 0x6474e550
    GNU_STACK = 0x6474e551
    GNU_RELRO = 0x6474e552

def get_ph_type(value):
    if value == p_type_e.PT_NULL: return "NULL"
    elif value == p_type_e.PT_LOAD: return "LOAD"
    elif value == p_type_e.PT_DYNAMIC: return "DYNAMIC"
    elif value == p_type_e.PT_INTERP: return "INTERP"
    elif value == p_type_e.PT_NOTE: return "NOTE"
    elif value == p_type_e.PT_SHLIB: return "SHLIB"
    elif value == p_type_e.PT_PHDR: return "PHDR"
    elif value == p_type_e.PT_LOOS: return "LOOS"
    elif value == p_type_e.PT_HIOS: return "HIOS"
    elif value == p_type_e.PT_LOPROC: return "LOPROC"
    elif value == p_type_e.PT_HIPROC: return "HIPROC"
    elif value == p_type_e.GNU_EH_FRAME: return "GNU_EH_FRAME"
    elif value == p_type_e.GNU_STACK: return "GNU_STACK"
    elif value == p_type_e.GNU_RELRO: return "GNU_RELRO"
    else: return None

class ph_dynamic_entry:
    def __init__(self, d_tag, d_un):
        self.d_tag = d_tag;
        self.d_un = d_un

class elf_seg_flags:
    PF_X = 0x1
    PF_W = 0x2
    PF_R = 0x4

def get_elf_seg_flag(value):
    ret = []
    if value & 0x01 == 1: ret.append("X")
    if (value & 0x02) >> 1 == 1: ret.append("W")
    if (value & 0x04) >> 2 == 1: ret.append("R")
    return ''.join(ret)

class PH_DYN_TAG_TYPE:
    DT_NULL  = 0
    DT_NEEDED  = 1
    DT_PLTRELSZ = 2
    DT_PLTGOT  = 3
    DT_HASH  = 4
    DT_STRTAB  = 5
    DT_SYMTAB  = 6
    DT_RELA  = 7
    DT_RELASZ  = 8
    DT_RELAENT  = 9
    DT_STRSZ  = 10
    DT_SYMENT  = 11
    DT_INIT = 12
    DT_FINI = 13
    DT_SONAME = 14
    DT_RPATH = 15
    DT_SYMBOLIC = 16
    DT_REL = 17
    DT_RELSZ = 18
    DT_RELENT = 19
    DT_PLTREL = 20
    DT_DEBUG = 21
    DT_TEXTREL = 22
    DT_JMPREL = 23
    DT_LOPROC = 0x70000000
    DT_HIPROC = 0x7FFFFFFF
    DT_BIND_NOW = 24
    DT_INIT_ARRAY = 25
    DT_FINI_ARRAY = 26
    DT_INIT_ARRAYSZ = 27
    DT_FINI_ARRAYSZ = 28
    DT_RUNPATH = 29
    DT_FLAGS = 30
    DT_ENCODING = 32
    DT_PREINIT_ARRAY = 32
    DT_PREINIT_ARRAYSZ = 33
    DT_NUM = 34
    DT_LOOS = 0x6000000d
    DT_HIOS = 0x6ffff000
    DT_PROC_NUM = 0x0
    DT_MIPS_NUM = 0x0
    DT_VALRNGLO = 0x6ffffd00
    DT_GNU_PRELINKED = 0x6ffffdf5
    DT_GNU_CONFLICTSZ = 0x6ffffdf6
    DT_GNU_LIBLISTSZ  = 0x6ffffdf7
    DT_CHECKSUM = 0x6ffffdf8
    DT_PLTPADSZ = 0x6ffffdf9
    DT_MOVEENT = 0x6ffffdfa
    DT_MOVESZ = 0x6ffffdfb
    DT_FEATURE_1 = 0x6ffffdfc
    DT_POSFLAG_1 = 0x6ffffdfd
    DT_SYMINSZ = 0x6ffffdfe
    DT_SYMINENT = 0x6ffffdff
    DT_VALRNGHI = 0x6ffffdff
    DT_VALNUM  = 12
    DT_ADDRRNGLO = 0x6ffffe00
    DT_GNU_HASH = 0x6ffffef5
    DT_TLSDESC_PLT = 0x6ffffef6
    DT_TLSDESC_GOT = 0x6ffffef7
    DT_GNU_CONFLICT = 0x6ffffef8
    DT_GNU_LIBLIST = 0x6ffffef9
    DT_CONFIG = 0x6ffffefa
    DT_DEPAUDIT = 0x6ffffefb
    DT_AUDIT = 0x6ffffefc
    DT_PLTPAD = 0x6ffffefd
    DT_MOVETAB = 0x6ffffefe
    DT_SYMINFO = 0x6ffffeff
    DT_ADDRRNGHI = 0x6ffffeff
    DT_ADDRNUM = 11
    DT_VERSYM = 0x6ffffff0
    DT_RELACOUNT = 0x6ffffff9
    DT_RELCOUNT = 0x6ffffffa
    DT_FLAGS_1 = 0x6ffffffb
    DT_VERDEF = 0x6ffffffc
    DT_VERDEFNUM = 0x6ffffffd
    DT_VERNEED = 0x6ffffffe
    DT_VERNEEDNUM = 0x6fffffff
    DT_VERSIONTAGNUM = 16
    DT_AUXILIARY = 0x7ffffffd
    DT_FILTER = 0x7fffffff
    DT_EXTRANUM = 3

def get_ph_dynamic_ent_tag_type(value):
    if value == PH_DYN_TAG_TYPE.DT_NULL: return "DT_NULL"
    elif value == PH_DYN_TAG_TYPE.DT_NEEDED: return "DT_NEEDED"
    elif value == PH_DYN_TAG_TYPE.DT_PLTRELSZ: return "DT_PLTRELSZ"
    elif value == PH_DYN_TAG_TYPE.DT_PLTGOT: return "DT_PLTGOT"
    elif value == PH_DYN_TAG_TYPE.DT_HASH: return "DT_HASH"
    elif value == PH_DYN_TAG_TYPE.DT_STRTAB: return "DT_STRTAB"
    elif value == PH_DYN_TAG_TYPE.DT_SYMTAB: return "DT_SYMTAB"
    elif value == PH_DYN_TAG_TYPE.DT_RELA: return "DT_RELA"
    elif value == PH_DYN_TAG_TYPE.DT_RELASZ: return "DT_RELASZ"
    elif value == PH_DYN_TAG_TYPE.DT_RELAENT: return "DT_RELAENT"
    elif value == PH_DYN_TAG_TYPE.DT_STRSZ: return "DT_STRSZ"
    elif value == PH_DYN_TAG_TYPE.DT_SYMENT: return "DT_SYMENT"
    elif value == PH_DYN_TAG_TYPE.DT_INIT: return "DT_INIT"
    elif value == PH_DYN_TAG_TYPE.DT_FINI: return "DT_FINI"
    elif value == PH_DYN_TAG_TYPE.DT_SONAME: return "DT_SONAME"
    elif value == PH_DYN_TAG_TYPE.DT_RPATH: return "DT_RPATH"
    elif value == PH_DYN_TAG_TYPE.DT_SYMBOLIC: return "DT_SYMBOLIC"
    elif value == PH_DYN_TAG_TYPE.DT_REL: return "DT_REL"
    elif value == PH_DYN_TAG_TYPE.DT_RELSZ: return "DT_RELSZ"
    elif value == PH_DYN_TAG_TYPE.DT_RELENT: return "DT_RELENT"
    elif value == PH_DYN_TAG_TYPE.DT_PLTREL: return "DT_PLTREL"
    elif value == PH_DYN_TAG_TYPE.DT_DEBUG: return "DT_DEBUG"
    elif value == PH_DYN_TAG_TYPE.DT_TEXTREL: return "DT_TEXTREL"
    elif value == PH_DYN_TAG_TYPE.DT_JMPREL: return "DT_JMPREL"
    elif value == PH_DYN_TAG_TYPE.DT_LOPROC: return "DT_LOPROC"
    elif value == PH_DYN_TAG_TYPE.DT_HIPROC: return "DT_HIPROC"
    elif value == PH_DYN_TAG_TYPE.DT_BIND_NOW: return "DT_BIND_NOW"
    elif value == PH_DYN_TAG_TYPE.DT_INIT_ARRAY: return "DT_INIT_ARRAY"
    elif value == PH_DYN_TAG_TYPE.DT_FINI_ARRAY: return "DT_FINI_ARRAY"
    elif value == PH_DYN_TAG_TYPE.DT_INIT_ARRAYSZ: return "DT_INIT_ARRAYSZ"
    elif value == PH_DYN_TAG_TYPE.DT_FINI_ARRAYSZ: return "DT_FINI_ARRAYSZ"
    elif value == PH_DYN_TAG_TYPE.DT_RUNPATH: return "DT_RUNPATH"
    elif value == PH_DYN_TAG_TYPE.DT_FLAGS: return "DT_FLAGS"
    elif value == PH_DYN_TAG_TYPE.DT_ENCODING: return "DT_ENCODING"
    elif value == PH_DYN_TAG_TYPE.DT_PREINIT_ARRAY: return "DT_PREINIT_ARRAY"
    elif value == PH_DYN_TAG_TYPE.DT_PREINIT_ARRAYSZ: return "DT_PREINIT_ARRAYSZ"
    elif value == PH_DYN_TAG_TYPE.DT_NUM: return "DT_NUM"
    elif value == PH_DYN_TAG_TYPE.DT_LOOS: return "DT_LOOS"
    elif value == PH_DYN_TAG_TYPE.DT_HIOS: return "DT_HIOS"
    #elif value == PH_DYN_TAG_TYPE.DT_PROC_NUM: return "DT_PROC_NUM"
    #elif value == PH_DYN_TAG_TYPE.DT_MIPS_NUM: return "DT_MIPS_NUM"
    elif value == PH_DYN_TAG_TYPE.DT_VALRNGLO: return "DT_VALRNGLO"
    elif value == PH_DYN_TAG_TYPE.DT_GNU_PRELINKED: return "DT_GNU_PRELINKED"
    elif value == PH_DYN_TAG_TYPE.DT_GNU_CONFLICTSZ: return "DT_GNU_CONFLICTSZ"
    elif value == PH_DYN_TAG_TYPE.DT_GNU_LIBLISTSZ: return "DT_GNU_LIBLISTSZ"
    elif value == PH_DYN_TAG_TYPE.DT_CHECKSUM: return "DT_CHECKSUM"
    elif value == PH_DYN_TAG_TYPE.DT_PLTPADSZ: return "DT_PLTPADSZ"
    elif value == PH_DYN_TAG_TYPE.DT_MOVEENT: return "DT_MOVEENT"
    elif value == PH_DYN_TAG_TYPE.DT_MOVESZ: return "DT_MOVESZ"
    elif value == PH_DYN_TAG_TYPE.DT_FEATURE_1: return "DT_FEATURE_1"
    elif value == PH_DYN_TAG_TYPE.DT_POSFLAG_1: return "DT_POSFLAG_1"
    elif value == PH_DYN_TAG_TYPE.DT_SYMINSZ: return "DT_SYMINSZ"
    elif value == PH_DYN_TAG_TYPE.DT_SYMINENT: return "DT_SYMINENT"
    elif value == PH_DYN_TAG_TYPE.DT_VALRNGHI: return "DT_VALRNGHI"
    #DT_VALNUM  = 12
    elif value == PH_DYN_TAG_TYPE.DT_ADDRRNGLO: return "DT_ADDRRNGLO"
    elif value == PH_DYN_TAG_TYPE.DT_GNU_HASH: return "DT_GNU_HASH"
    elif value == PH_DYN_TAG_TYPE.DT_TLSDESC_PLT: return "DT_TLSDESC_PLT"
    elif value == PH_DYN_TAG_TYPE.DT_TLSDESC_GOT: return "DT_TLSDESC_GOT"
    elif value == PH_DYN_TAG_TYPE.DT_GNU_CONFLICT: return "DT_GNU_CONFLICT"
    elif value == PH_DYN_TAG_TYPE.DT_GNU_LIBLIST: return "DT_GNU_LIBLIST"
    elif value == PH_DYN_TAG_TYPE.DT_CONFIG: return "DT_CONFIG"
    elif value == PH_DYN_TAG_TYPE.DT_DEPAUDIT: return "DT_DEPAUDIT"
    elif value == PH_DYN_TAG_TYPE.DT_AUDIT: return "DT_AUDIT"
    elif value == PH_DYN_TAG_TYPE.DT_PLTPAD: return "DT_PLTPAD"
    elif value == PH_DYN_TAG_TYPE.DT_MOVETAB: return "DT_MOVETAB"
    elif value == PH_DYN_TAG_TYPE.DT_SYMINFO: return "DT_SYMINFO"
    elif value == PH_DYN_TAG_TYPE.DT_ADDRRNGHI: return "DT_ADDRRNGHI"
    #DT_ADDRNUM = 11
    elif value == PH_DYN_TAG_TYPE.DT_VERSYM: return "DT_VERSYM"
    elif value == PH_DYN_TAG_TYPE.DT_RELACOUNT: return "DT_RELACOUNT"
    elif value == PH_DYN_TAG_TYPE.DT_RELCOUNT: return "DT_RELCOUNT"
    elif value == PH_DYN_TAG_TYPE.DT_FLAGS_1: return "DT_FLAGS_1"
    elif value == PH_DYN_TAG_TYPE.DT_VERDEF: return "DT_VERDEF"
    elif value == PH_DYN_TAG_TYPE.DT_VERDEFNUM: return "DT_VERDEFNUM"
    elif value == PH_DYN_TAG_TYPE.DT_VERNEED: return "DT_VERNEED"
    elif value == PH_DYN_TAG_TYPE.DT_VERNEEDNUM: return "DT_VERNEEDNUM"
    elif value == PH_DYN_TAG_TYPE.DT_VERSIONTAGNUM: return "DT_VERSIONTAGNUM"
    elif value == PH_DYN_TAG_TYPE.DT_AUXILIARY: return "DT_AUXILIARY"
    elif value == PH_DYN_TAG_TYPE.DT_FILTER: return "DT_FILTER"
    #DT_EXTRANUM = 3
    else: return str(value)
    #else: return "UNKNOWN"

class X86_REL_TYPE:
    R_386_NONE = 0
    R_386_32 = 1
    R_386_PC32 = 2
    R_386_GOT32 = 3
    R_386_PLT32 = 4
    R_386_COPY = 5
    R_386_GLOB_DAT = 6
    R_386_JMP_SLOT = 7
    R_386_RELATIVE = 8
    R_386_GOTOFF = 9
    R_386_GOTPC = 10
    R_386_32PLT = 11
    R_386_16 = 12
    R_386_PC16 = 13
    R_386_8 = 14
    R_386_PC8 = 15
    R_386_SIZE32 = 16

def get_x86_rel_type(val):
    if val == X86_REL_TYPE.R_386_NONE: return "R_386_NONE"
    elif val == X86_REL_TYPE.R_386_32: return "R_386_32"
    elif val == X86_REL_TYPE.R_386_PC32: return "R_386_PC32"
    elif val == X86_REL_TYPE.R_386_GOT32: return "R_386_GOT32"
    elif val == X86_REL_TYPE.R_386_PLT32: return "R_386_PLT32"
    elif val == X86_REL_TYPE.R_386_COPY: return "R_386_COPY"
    elif val == X86_REL_TYPE.R_386_GLOB_DAT: return "R_386_GLOB_DAT"
    elif val == X86_REL_TYPE.R_386_JMP_SLOT: return "R_386_JMP_SLOT"
    elif val == X86_REL_TYPE.R_386_RELATIVE: return "R_386_RELATIVE"
    elif val == X86_REL_TYPE.R_386_GOTOFF: return "R_386_GOTOFF"
    elif val == X86_REL_TYPE.R_386_GOTPC: return "R_386_GOTPC"
    elif val == X86_REL_TYPE.R_386_32PLT: return "R_386_32PLT"
    elif val == X86_REL_TYPE.R_386_16: return "R_386_16"
    elif val == X86_REL_TYPE.R_386_PC16: return "R_386_PC16"
    elif val == X86_REL_TYPE.R_386_8: return "R_386_8"
    elif val == X86_REL_TYPE.R_386_PC8: return "R_386_PC8"
    elif val == X86_REL_TYPE.R_386_SIZE32: return "R_386_SIZE32"
    else: return "UNKNOWN"

class X86_64_REL_TYPE:
    R_AMD64_NONE = 0
    R_AMD64_64 = 1
    R_AMD64_PC32 = 2
    R_AMD64_GOT32 = 3
    R_AMD64_PLT32 = 4
    R_AMD64_COPY = 5
    R_AMD64_GLOB_DAT = 6
    R_AMD64_JUMP_SLOT = 7
    R_AMD64_RELATIVE = 8
    R_AMD64_GOTPCREL = 9
    R_AMD64_32 = 10
    R_AMD64_32S = 11
    R_AMD64_16 = 12
    R_AMD64_PC16 = 13
    R_AMD64_8 = 14
    R_AMD64_PC8 = 15
    R_AMD64_PC64 = 24
    R_AMD64_GOTOFF64 = 25
    R_AMD64_GOTPC32 = 26
    R_AMD64_SIZE32 = 32
    R_AMD64_SIZE64 = 33

class X86_64_REL_TYPE_2:
    R_X86_64_NONE = 0
    R_X86_64_64 = 1
    R_X86_64_PC32 = 2
    R_X86_64_GOT32 = 3
    R_X86_64_PLT32 = 4
    R_X86_64_COPY = 5
    R_X86_64_GLOB_DAT = 6
    R_X86_64_JUMP_SLOT = 7
    R_X86_64_RELATIVE = 8
    R_X86_64_GOTPCREL = 9
    R_X86_64_32 = 10
    R_X86_64_32S = 11
    R_X86_64_16 = 12
    R_X86_64_PC16 = 13
    R_X86_64_64_8 = 14
    R_X86_64_PC8 = 15
    R_X86_64_DTPMOD64 = 16
    R_X86_64_DTPOFF64 = 17
    R_X86_64_TPOFF64 = 18
    R_X86_64_TLSGD = 19
    R_X86_64_TLSLD = 20
    R_X86_64_DTPOFF32 = 21
    R_X86_64_GOTTPOFF = 22
    R_X86_64_TPOFF32 = 23
    R_X86_64_PC64 = 24
    R_X86_64_GOTOFF64 = 25
    R_X86_64_GOTPC32 = 26
    R_X86_64_SIZE32 = 32
    R_X86_64_SIZE64 = 33
    R_X86_64_GOTPC32_TLDSEC = 34
    R_X86_64_TLDSEC_CALL = 35
    R_X86_64_TLDSEC = 36
    R_X86_64_IRELATIVE = 37

def get_x86_64_rel_type(val):
    if val == X86_64_REL_TYPE.R_AMD64_NONE: return "R_386_NONE"
    elif val == X86_64_REL_TYPE.R_AMD64_64: return "R_AMD64_64"
    elif val == X86_64_REL_TYPE.R_AMD64_PC32: return "R_AMD64_PC32"
    elif val == X86_64_REL_TYPE.R_AMD64_GOT32: return "R_AMD64_GOT32"
    elif val == X86_64_REL_TYPE.R_AMD64_PLT32: return "R_AMD64_PLT32"
    elif val == X86_64_REL_TYPE.R_AMD64_COPY: return "R_AMD64_COPY"
    elif val == X86_64_REL_TYPE.R_AMD64_GLOB_DAT: return "R_AMD64_GLOB_DAT"
    elif val == X86_64_REL_TYPE.R_AMD64_JUMP_SLOT: return "R_AMD64_JUMP_SLOT"
    elif val == X86_64_REL_TYPE.R_AMD64_RELATIVE: return "R_AMD64_RELATIVE"
    elif val == X86_64_REL_TYPE.R_AMD64_GOTPCREL: return "R_AMD64_GOTPCREL"
    elif val == X86_64_REL_TYPE.R_AMD64_32: return "R_AMD64_32"
    elif val == X86_64_REL_TYPE.R_AMD64_32S: return "R_AMD64_32S"
    elif val == X86_64_REL_TYPE.R_AMD64_16: return "R_AMD64_16"
    elif val == X86_64_REL_TYPE.R_AMD64_PC16: return "R_AMD64_PC16"
    elif val == X86_64_REL_TYPE.R_AMD64_8: return "R_AMD64_8"
    elif val == X86_64_REL_TYPE.R_AMD64_PC8: return "R_AMD64_PC8"
    elif val == X86_64_REL_TYPE.R_AMD64_PC64: return "R_AMD64_PC64"
    elif val == X86_64_REL_TYPE.R_AMD64_GOTOFF64: return "R_AMD64_GOTOFF64"
    elif val == X86_64_REL_TYPE.R_AMD64_GOTPC32: return "R_AMD64_GOTPC32"
    elif val == X86_64_REL_TYPE.R_AMD64_SIZE32: return "R_AMD64_SIZE32"
    elif val == X86_64_REL_TYPE.R_AMD64_SIZE64: return "R_AMD64_SIZE64"
    else: return "UNKNOWN"

def get_x86_64_rel_type_2(val):
    if val == X86_64_REL_TPE_2.R_X86_64_NONE: return"R_X86_64_NONE"
    elif val == X86_64_REL_TPE_2.R_X86_64_64: return"R_X86_64_64"
    elif val == X86_64_REL_TPE_2.R_X86_64_PC32: return"R_X86_64_PC32"
    elif val == X86_64_REL_TPE_2.R_X86_64_GOT32: return"R_X86_64_GOT32"
    elif val == X86_64_REL_TPE_2.R_X86_64_PLT32: return"R_X86_64_PLT32"
    elif val == X86_64_REL_TPE_2.R_X86_64_COPY: return"R_X86_64_COPY"
    elif val == X86_64_REL_TPE_2.R_X86_64_GLOB_DAT: return"R_X86_64_GLOB_DAT"
    elif val == X86_64_REL_TPE_2.R_X86_64_JUMP_SLOT: return"R_X86_64_JUMP_SLOT"
    elif val == X86_64_REL_TPE_2.R_X86_64_RELATIVE: return"R_X86_64_RELATIVE"
    elif val == X86_64_REL_TPE_2.R_X86_64_GOTPCREL: return"R_X86_64_GOTPCREL"
    elif val == X86_64_REL_TPE_2.R_X86_64_32: return"R_X86_64_32"
    elif val == X86_64_REL_TPE_2.R_X86_64_32S: return"R_X86_64_32S"
    elif val == X86_64_REL_TPE_2.R_X86_64_16: return"R_X86_64_16"
    elif val == X86_64_REL_TPE_2.R_X86_64_PC16: return"R_X86_64_PC16"
    elif val == X86_64_REL_TPE_2.R_X86_64_64_8: return"R_X86_64_64_8"
    elif val == X86_64_REL_TPE_2.R_X86_64_PC8: return"R_X86_64_PC8"
    elif val == X86_64_REL_TPE_2.R_X86_64_DTPMOD64: return"R_X86_64_DTPMOD64"
    elif val == X86_64_REL_TPE_2.R_X86_64_DTPOFF64: return"R_X86_64_DTPOFF64"
    elif val == X86_64_REL_TPE_2.R_X86_64_TPOFF64: return"R_X86_64_TPOFF64"
    elif val == X86_64_REL_TPE_2.R_X86_64_TLSGD: return"R_X86_64_TLSGD"
    elif val == X86_64_REL_TPE_2.R_X86_64_TLSLD: return"R_X86_64_TLSLD"
    elif val == X86_64_REL_TPE_2.R_X86_64_DTPOFF32: return"R_X86_64_DTPOFF32"
    elif val == X86_64_REL_TPE_2.R_X86_64_GOTTPOFF: return"R_X86_64_GOTTPOFF"
    elif val == X86_64_REL_TPE_2.R_X86_64_TPOFF32: return"R_X86_64_TPOFF32"
    elif val == X86_64_REL_TPE_2.R_X86_64_PC64: return"R_X86_64_PC64"
    elif val == X86_64_REL_TPE_2.R_X86_64_GOTOFF64: return"R_X86_64_GOTOFF64"
    elif val == X86_64_REL_TPE_2.R_X86_64_GOTPC32: return"R_X86_64_GOTPC32"
    elif val == X86_64_REL_TPE_2.R_X86_64_SIZE32: return"R_X86_64_SIZE32"
    elif val == X86_64_REL_TPE_2.R_X86_64_SIZE64: return"R_X86_64_SIZE64"
    elif val == X86_64_REL_TPE_2.R_X86_64_GOTPC32_TLDSEC: return"R_X86_64_GOTPC32_TLDSEC"
    elif val == X86_64_REL_TPE_2.R_X86_64_TLDSEC_CALL: return"R_X86_64_TLDSEC_CALL"
    elif val == X86_64_REL_TPE_2.R_X86_64_TLDSEC: return"R_X86_64_TLDSEC"
    elif val == X86_64_REL_TPE_2.R_X86_64_IRELATIVE: return"R_X86_64_IRELATIVE"
    else: return "UNKNOWN"

class ELF_ST_BIND:
    STB_LOCAL = 0
    STB_GLOBAL = 1
    STB_WEAK = 2
    STB_LOOS = 10
    STB_HIOS = 12
    STB_LOPROC = 13
    STB_HIPROC = 15

def get_elf_st_bind_string(value):
    if value == ELF_ST_BIND.STB_LOCAL: return "STB_LOCAL"
    elif value == ELF_ST_BIND.STB_GLOBAL: return "STB_GLOBAL"
    elif value == ELF_ST_BIND.STB_WEAK: return "STB_WEAK"
    elif value == ELF_ST_BIND.STB_LOOS: return "STB_LOOS"
    elif value == ELF_ST_BIND.STB_HIOS: return "STB_HIOS"
    elif value == ELF_ST_BIND.STB_LOPROC: return "STB_LOPROC"
    elif value == ELF_ST_BIND.STB_LOPROC: return "STB_HIPROC"
    else: return None

class ELF_ST_TYPE:
    STT_NOTYPE = 0
    STT_OBJECT = 1
    STT_FUNC = 2
    STT_SECTION = 3
    STT_FILE = 4
    STT_COMMON = 5
    STT_TLS = 6
    STT_LOOS = 10
    STT_HIOS = 12
    STT_LOPROC = 13
    STT_SPARC_REGISTER = 13
    STT_HIPROC = 15

def get_elf_st_type_string(value):
    if value == ELF_ST_TYPE.STT_NOTYPE: return "STT_NOTYPE"
    elif value == ELF_ST_TYPE.STT_OBJECT: return "STT_OBJECT"
    elif value == ELF_ST_TYPE.STT_FUNC: return "STT_FUNC"
    elif value == ELF_ST_TYPE.STT_SECTION: return "STT_SECTION"
    elif value == ELF_ST_TYPE.STT_FILE: return "STT_FILE"
    elif value == ELF_ST_TYPE.STT_COMMON: return "STT_COMMON"
    elif value == ELF_ST_TYPE.STT_TLS: return "STT_TLS"
    elif value == ELF_ST_TYPE.STT_LOOS: return "STT_LOOS"
    elif value == ELF_ST_TYPE.STT_HIOS: return "STT_HIOS"
    elif value == ELF_ST_TYPE.STT_LOPROC: return "STT_LOPROC"
    elif value == ELF_ST_TYPE.STT_SPARC_REGISTER: return "STT_SPARC_REGISTER"
    elif value == ELF_ST_TYPE.STT_HIPROC: return "STT_HIPROC"
    else: return None

class ELF_VIS:
    STV_DEFAULT = 0
    STV_INTERNAL = 1
    STV_HIDDEN = 2
    STV_PROTECTED = 3
    STV_EXPORTED = 4
    STV_SINGLETON = 5
    STV_ELIMINATE = 6

def get_elf_vis_string(value):
    if value == ELF_VIS.STV_DEFAULT: return "STV_DEFAULT"
    elif value == ELF_VIS.STV_INTERNAL: return "STV_INTERNAL"
    elif value == ELF_VIS.STV_HIDDEN: return "STV_HIDDEN"
    elif value == ELF_VIS.STV_PROTECTED: return "STV_PROTECTED"
    elif value == ELF_VIS.STV_EXPORTED: return "STV_EXPORTED"
    elif value == ELF_VIS.STV_SINGLETON: return "STV_SINGLETON"
    elif value == ELF_VIS.STV_ELIMINATE: return "STV_ELIMINATE"
    else: return None

class Colors:
    purple = '\033[95m'
    blue = '\033[94m'
    green = '\033[92m'
    yellow = '\033[93m'
    red = '\033[91m'
    grey = '\033[1;37m'
    darkgrey = '\033[1;30m'
    cyan = '\033[1;36m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

def openSO_r(path):
    so = open(path, "rb")
    return so

def openSO_w(path):
    so = open(path, "wb")
    return so

class ELFHDR():
    def __init__(self, ei_mag, ei_class, ei_data, ei_version, ei_osabi, ei_abiversion,
                 ei_pad, e_type, e_machine, e_version, e_entry, e_phoff,
                 e_shoff, e_flags, e_ehsize, e_phentsize, e_phnum, e_shentsize,
                 e_shnum, e_shstrndx):
        self.ei_mag = ei_mag
        self.ei_class = ei_class
        self.ei_data = ei_data
        self.ei_version = ei_version
        self.ei_osabi = ei_osabi
        self.ei_abiversion = ei_abiversion
        self.ei_pad = ei_pad
        self.e_type = e_type
        self.e_machine = e_machine
        self.e_version = e_version
        self.e_entry = e_entry
        self.e_phoff = e_phoff
        self.e_shoff = e_shoff
        self.e_flags = e_flags
        self.e_ehsize = e_ehsize
        self.e_phentsize = e_phentsize
        self.e_phnum = e_phnum
        self.e_shentsize = e_shentsize
        self.e_shnum = e_shnum
        self.e_shstrndx = e_shstrndx

class PHDR():
    def __init__(self, p_type, p_flags, p_offset, p_vaddr, p_paddr, p_filesz,
                 p_memsz, p_flags2, p_align):
        self.p_type = p_type
        self.p_flags = p_flags
        self.p_offset = p_offset
        self.p_vaddr = p_vaddr
        self.p_paddr = p_paddr
        self.p_filesz = p_filesz
        self.p_memsz = p_memsz
        self.p_flags2 = p_flags2
        self.p_align = p_align

class SHDR():
    def __init__(self, sh_name, sh_type, sh_flags, sh_addr, sh_offset, sh_size,
                 sh_link, sh_info, sh_addralign, sh_entsize):
        self.sh_name = sh_name
        self.sh_type = sh_type
        self.sh_flags = sh_flags
        self.sh_addr = sh_addr
        self.sh_offset = sh_offset
        self.sh_size = sh_size
        self.sh_link = sh_link
        self.sh_info = sh_info
        self.sh_addralign = sh_addralign
        self.sh_entsize = sh_entsize

class Symbol_Table_Entry64():
    def __init__(self, st_name, st_info, st_other, st_shndx, st_value, st_size, st_bind, st_type):
        self.st_name = st_name
        self.st_info = st_info
        self.st_other = st_other
        self.st_shndx = st_shndx
        self.st_value = st_value
        self.st_size = st_size
        self.st_bind = st_bind
        self.st_type = st_type

class ELF(object):
    def __init__(self, so):
        self.so = so
        self.so.seek(0, 0)
        self.elfhdr = ELFHDR(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0)
        self.phdr = []
        self.shhdr = []
        self.size = int()
        self.string_tb_e = []
        self.string_tb_e_dyn = []
        self.symbol_table_e = []
        self.data_section = []
        self.text_section = []
        self.dlpath = str()
        self.ph_dyn_ent = []
        self.dyn_section = []
        self.dyn_section_ents = []
        self.rela_dyn = []
        self.rela_dyn_ents = []
        self.rela_plt = []
        self.rela_plt_ents = []
        self.rodata = []
        self.plt = []
        self.got = []
        self.got_plt = []
        self.plt_got = []
        self.plt_ents = []
        self.plt_got_ents = []
        self.got_ents = []
        self.got_plt_ents = []

    def init(self, size):
        self.size = size
        self.read_ELF_H(size)
        self.so.seek(byte2int(self.elfhdr.e_phoff))
        phnum = byte2int(self.elfhdr.e_phnum)
        for i in range(0, phnum):
            self.read_PHDR(size)
        self.so.seek(byte2int(self.elfhdr.e_shoff))
        shnum = byte2int(self.elfhdr.e_shnum)
        for i in range(0, shnum):
            self.read_SHDR(size)
        for i in range(0, shnum):
            type = byte2int(self.shhdr[i].sh_type)
            if type == sh_type_e.SHT_SYMTAB:
                self.so.seek(byte2int(self.shhdr[i].sh_offset), 0)
                symbol_tb = self.so.read(byte2int(self.shhdr[i].sh_size))
                offset = 0
                num = int(byte2int(self.shhdr[i].sh_size) / 24)
                for j in range(0, num):
                    self.read_st_entry(symbol_tb[offset:offset + 24], self.string_tb_e)
                    offset += 24
            if type == sh_type_e.SHT_DYNSYM:
                self.so.seek(byte2int(self.shhdr[i].sh_offset), 0)
                symbol_tb = self.so.read(byte2int(self.shhdr[i].sh_size))
                offset = 0
                num = int(byte2int(self.shhdr[i].sh_size) / 24)
                for j in range(0, num):
                    self.read_st_entry(symbol_tb[offset:offset + 24], self.string_tb_e_dyn)
                    offset += 24
        self.pop_data_section()
        self.pop_text_section()
        self.get_ph_dyn_entries()
        self.pop_dynamic_entries(".dynamic", self.dyn_section_ents)
        self.pop_rela(".rela.plt", self.rela_plt, self.rela_plt_ents)
        self.pop_rela(".rela.dyn", self.rela_dyn, self.rela_dyn_ents)
        #self.pop_rel()

    def read_ELF_H(self, size):
        self.elfhdr.ei_mag = self.so.read(4)
        self.elfhdr.ei_class = self.so.read(1)
        self.elfhdr.ei_data = self.so.read(1)
        self.elfhdr.ei_version = self.so.read(1)
        self.elfhdr.ei_osabi = self.so.read(1)
        self.elfhdr.ei_abiversion = self.so.read(1)
        self.elfhdr.ei_pad = self.so.read(7)
        self.elfhdr.e_type = self.so.read(2)
        self.elfhdr.e_machine = self.so.read(2)
        self.elfhdr.e_version = self.so.read(4)
        if size == 32: self.elfhdr.e_entry = self.so.read(4)
        elif size == 64: self.elfhdr.e_entry = self.so.read(8)
        if size == 32: self.elfhdr.e_phoff = self.so.read(4)
        elif size == 64: self.elfhdr.e_phoff = self.so.read(8)
        if size == 32: self.elfhdr.e_shoff = self.so.read(4)
        elif size == 64: self.elfhdr.e_shoff = self.so.read(8)
        self.elfhdr.e_flags = self.so.read(4)
        self.elfhdr.e_ehsize = self.so.read(2)
        self.elfhdr.e_phentsize = self.so.read(2)
        self.elfhdr.e_phnum = self.so.read(2)
        self.elfhdr.e_shentsize = self.so.read(2)
        self.elfhdr.e_shnum = self.so.read(2)
        self.elfhdr.e_shstrndx = self.so.read(2)

    def read_PHDR(self, size):
        dummy = PHDR(0,0,0,0,0,0,0,0,0)
        dummy.p_type = self.so.read(4)
        if size == 64: dummy.p_flags = self.so.read(4)
        if size == 32: dummy.p_offset = self.so.read(4)
        elif size == 64: dummy.p_offset = self.so.read(8)
        if size == 32: dummy.p_vaddr = self.so.read(4)
        elif size == 64: dummy.p_vaddr = self.so.read(8)
        if size == 32: dummy.p_paddr = self.so.read(4)
        elif size == 64: dummy.p_paddr = self.so.read(8)
        if size == 32: dummy.p_filesz = self.so.read(4)
        elif size == 64: dummy.p_filesz = self.so.read(8)
        if size == 32: dummy.p_memsz = self.so.read(4)
        elif size == 64: dummy.p_memsz = self.so.read(8)
        if size == 32: dummy.p_flags2 = self.so.read(4)
        elif size == 64: pass
        if size == 32: dummy.p_align = self.so.read(4)
        elif size == 64: dummy.p_align = self.so.read(8)
        self.phdr.append(dummy)

    def read_SHDR(self, size):
        dummy = SHDR(0,0,0,0,0,0,0,0,0,0)
        dummy.sh_name = self.so.read(4)
        dummy.sh_type = self.so.read(4)
        if size == 32: dummy.sh_flags = self.so.read(4)
        elif size == 64: dummy.sh_flags = self.so.read(8)
        if size == 32: dummy.sh_addr = self.so.read(4)
        elif size == 64: dummy.sh_addr = self.so.read(8)
        if size == 32: dummy.sh_offset = self.so.read(4)
        elif size == 64: dummy.sh_offset = self.so.read(8)
        if size == 32: dummy.sh_size = self.so.read(4)
        elif size == 64: dummy.sh_size = self.so.read(8)
        dummy.sh_link = self.so.read(4)
        dummy.sh_info = self.so.read(4)
        if size == 32: dummy.sh_addralign = self.so.read(4)
        elif size == 64: dummy.sh_addralign = self.so.read(8)
        if size == 32: dummy.sh_entsize = self.so.read(4)
        elif size == 64: dummy.sh_entsize = self.so.read(8)
        self.shhdr.append(dummy)

    def read_st_entry(self, st, entry_list):
        dummy = Symbol_Table_Entry64(0,0,0,0,0,0,0,0)
        dummy.st_name = st[0:4]
        dummy.st_info = st[4:5]
        dummy.st_other = st[5:6]
        dummy.st_shndx = st[6:8]
        dummy.st_value = st[8:16]
        dummy.st_size = st[16:24]
        dummy.st_bind = byte2int(dummy.st_info) >> 4
        dummy.st_type = byte2int(dummy.st_info) & 0x0f
        entry_list.append(dummy)

    def read_section_name(self, index):
        shstrtab_index = byte2int(self.elfhdr.e_shstrndx)
        name = []
        self.so.seek(byte2int(self.shhdr[shstrtab_index].sh_offset), 0)
        strings = self.so.read(byte2int(self.shhdr[shstrtab_index].sh_size))
        char = strings[index]
        while chr(char) != "\0":
            index += 1
            name.append(chr(char))
            char = strings[index]
        return ''.join(name)

    def get_ph_dyn_entries(self):
        size = 0
        for phdr in self.phdr:
            if byte2int(phdr.p_type) == p_type_e.PT_DYNAMIC:
                self.so.seek(byte2int(phdr.p_offset), 0)
                size = byte2int(phdr.p_filesz)
                ph_dyn = self.so.read(size)
        for i in range(int(size/8)):
            d_tag = byte2int(ph_dyn[8*i:8*i + 4])
            d_un = byte2int(ph_dyn[8*i + 4:8*i + 8])
            self.ph_dyn_ent.append(ph_dynamic_entry(d_tag, d_un))

    def dump_ph_dyn_entries(self):
        header = ["d_tag", "d_un"]
        tag_list = [get_ph_dynamic_ent_tag_type(ph.d_tag) for ph in self.ph_dyn_ent]
        un_list = [ph.d_un for ph in self.ph_dyn_ent]
        lines = ffs(2, header, True, tag_list, un_list)
        for line in lines:
            print(line)

    def dump_funcs(self, dump_b):
        ret_list = []
        dummy = []
        ret_list_int = []
        for iter in self.string_tb_e:
            if iter.st_type == ELF_ST_TYPE.STT_FUNC:
                self.so.seek(int.from_bytes(iter.st_value, byteorder="little"))
                obj = self.so.read(int.from_bytes(iter.st_size, byteorder="little"))
                ret_list.append(obj)
                for byte in obj:
                    dummy.append(int(byte))
                ret_list_int.append(dummy)
                dummy = []
        if dump_b:
            for obj in ret_list_int:
                for byte in obj:
                    print(format(byte, "02x") + " ", end="")
                print("\n")

        return ret_list_int

    def dump_symbol_string(self, stt_type, dump_b):
        ret_list = []
        for entry in self.string_tb_e:
            if entry.st_type == stt_type:
                ret_list.append("".join(self.get_st_entry_symbol_string(byte2int(entry.st_name), ".strtab")))
        if dump_b:
            for name in ret_list:
                print(name)
        return ret_list

    def dump_section(self, section_name, dump):
        hit = False
        for section in self.shhdr:
            name = self.read_section_name(byte2int(section.sh_name))
            if name == section_name:
                hit = True
                self.so.seek(byte2int(section.sh_offset))
                obj = self.so.read(byte2int(section.sh_size))
                if section_name == ".interp":  self.dlpath = repr(obj)
                count = int()
                if dump:
                    strrep = []
                    for byte in obj:
                        if count%16 == 0:
                            for ch in strrep:
                                if ord(ch) > 32 and ord(ch) < 127: print(ch, end = '')
                                else: print(" ", end="")
                            print()
                            strrep = []
                            print(format(count, "06x"), ': ', end='')
                            strrep.append(str(chr(byte)))
                            print(format(byte, '02x') + ' ', end='')
                        else:
                            pass
                            strrep += str(chr(byte))
                            print(format(byte, '02x') + ' ', end='')
                        count += 1
                    for i in range(0, 16-count%16): print("   ", end="")
                    for ch in strrep:
                            if ord(ch) > 32 and ord(ch) < 127: print(ch, end = '')
                            else: print(" ", end="")
                    print()

                ret_dummy = []
                for i in range(0, len(obj)):
                    ret_dummy.append(obj[i])
                #print(ret_dummy)
                return ret_dummy
        if not hit: print(Colors.red + Colors.BOLD + "section is not present" + Colors.ENDC)

    def dump_obj_size(self, stt_type, dump_b):
        ret_list = []
        for entry in self.string_tb_e:
            if entry.st_type == stt_type:
                ret_list.append(byte2int(entry.st_size))
        if dump_b:
            for name in ret_list:
                print(name)
        return ret_list

    def dump_symbol_idx(self):
        header = ["name", "size", "value", "info", "other", "shndx"]
        name_list = [byte2int(st.st_name) for st in self.string_tb_e]
        size_list = [byte2int(st.st_size) for st in self.string_tb_e]
        value_list = [byte2int(st.st_value) for st in self.string_tb_e]
        info_list = [byte2int(st.st_info) for st in self.string_tb_e]
        other_list = [byte2int(st.st_other) for st in self.string_tb_e]
        shndx_list = [byte2int(st.st_shndx) for st in self.string_tb_e]
        lines = ffs(2, header, True, name_list, size_list, value_list, info_list, other_list, shndx_list)
        print(Colors.green + Colors.BOLD + "symbol:" + Colors.ENDC)
        for line in lines:
            print(line)
        print(Colors.green + Colors.BOLD + "dyn symbol:" + Colors.ENDC)
        header = ["name", "size", "value", "info", "other", "shndx"]
        name_list = [byte2int(st.st_name) for st in self.string_tb_e_dyn]
        size_list = [byte2int(st.st_size) for st in self.string_tb_e_dyn]
        value_list = [byte2int(st.st_value) for st in self.string_tb_e_dyn]
        info_list = [byte2int(st.st_info) for st in self.string_tb_e_dyn]
        other_list = [byte2int(st.st_other) for st in self.string_tb_e_dyn]
        shndx_list = [byte2int(st.st_shndx) for st in self.string_tb_e_dyn]
        lines = ffs(2, header, True, name_list, size_list, value_list, info_list, other_list, shndx_list)
        for line in lines:
            print(line)

    def dump_header(self):
        header = ["ei_mag", "ei_class", "ei_data", "ei_version", "ei_osabi", "ei_abiversion", "ei_pad",
                  "e_type", "e_machine", "e_version", "e_version", "e_entry", "e_phoff", "e_shoff", "e_flags",
                  "e_entsize", "e_phentsize", "e_phnum", "e_shentsize", "e_shnum", "e_shstrndx"]
        mag_list = [self.elfhdr.ei_mag]
        class_list = [byte2int(self.elfhdr.ei_class)]
        data_list = [byte2int(self.elfhdr.ei_data)]
        version_list = [byte2int(self.elfhdr.ei_version)]
        osabi_list = [byte2int(self.elfhdr.ei_osabi)]
        abiversion_list = [byte2int(self.elfhdr.ei_abiversion)]
        pad_list = [byte2int(self.elfhdr.ei_pad)]
        type_list = [byte2int(self.elfhdr.e_type)]
        machine_list = [byte2int(self.elfhdr.e_machine)]
        version_list = [byte2int(self.elfhdr.e_version)]
        entry_list = [byte2int(self.elfhdr.e_entry)]
        phoff_list = [byte2int(self.elfhdr.e_phoff)]
        shoff_list = [byte2int(self.elfhdr.e_shoff)]
        flags_list = [byte2int(self.elfhdr.e_flags)]
        ehsize_list = [byte2int(self.elfhdr.e_ehsize)]
        phentsize_list = [byte2int(self.elfhdr.e_phentsize)]
        phnum_list = [byte2int(self.elfhdr.e_phnum)]
        shentsize_list = [byte2int(self.elfhdr.e_shentsize)]
        shnum_list = [byte2int(self.elfhdr.e_shnum)]
        shstrndx_list = [byte2int(self.elfhdr.e_shstrndx)]
        lines = ffs(2, header, True, mag_list, class_list, data_list, version_list, osabi_list, abiversion_list,
                    pad_list, type_list, machine_list, version_list, entry_list, phoff_list, shoff_list,
                    flags_list, ehsize_list, phentsize_list, phnum_list, shentsize_list, phnum_list, shentsize_list, shnum_list, shstrndx_list)
        for line in lines:
            print(line)

    def dump_phdrs(self):
        header = ["p_type", "p_flags", "p_offset", "p_vaddr", "p_paddr", "p_filesz", "p_memsz", "p_flags2", "p_align"]
        type_list = [get_ph_type(byte2int(phdr.p_type)) for phdr in self.phdr]
        flags_list = [get_elf_seg_flag(byte2int(phdr.p_type)) for phdr in self.phdr]
        offset_list = [byte2int(phdr.p_offset) for phdr in self.phdr]
        vaddr_list = [byte2int(phdr.p_vaddr) for phdr in self.phdr]
        paddr_list = [byte2int(phdr.p_paddr) for phdr in self.phdr]
        filesz_list = [byte2int(phdr.p_filesz) for phdr in self.phdr]
        memsz_list = [byte2int(phdr.p_memsz) for phdr in self.phdr]
        flags2_list = [phdr.p_flags2 for phdr in self.phdr]
        align_list = [byte2hex(phdr.p_align) for phdr in self.phdr]

        lines = ffs(2, header, True, type_list, flags_list, offset_list, vaddr_list, paddr_list, filesz_list, memsz_list, flags2_list, align_list)
        for line in lines:
            print(line)

    def dump_shdrs(self):
        header = ["sh_name", "sh_type", "sh_flags", "sh_addr", "sh_offset", "sh_size", "sh_link", "sh_info", "sh_addralign", "sh_entsize"]
        name_list = [self.read_section_name(byte2int(shhdr.sh_name)) for shhdr in self.shhdr]
        type_list = [get_section_type_string(byte2int(shhdr.sh_type)) for shhdr in self.shhdr]
        flag_list = [byte2int(shhdr.sh_flags) for shhdr in self.shhdr]
        addr_list = [byte2int(shhdr.sh_addr) for shhdr in self.shhdr]
        offset_list = [byte2int(shhdr.sh_offset) for shhdr in self.shhdr]
        size_list = [byte2int(shhdr.sh_size) for shhdr in self.shhdr]
        link_list = [byte2int(shhdr.sh_link) for shhdr in self.shhdr]
        info_list = [byte2int(shhdr.sh_info) for shhdr in self.shhdr]
        allign_list = [byte2int(shhdr.sh_addralign) for shhdr in self.shhdr]
        entsize_list = [byte2int(shhdr.sh_entsize) for shhdr in self.shhdr]

        lines = ffs(2, header, True, name_list, type_list, flag_list, addr_list, offset_list, size_list, link_list, info_list, allign_list, entsize_list)
        for line in lines:
            print(line)

    def dump_symbol_tb(self, name, type):
        for i in range(0, byte2int(self.elfhdr.e_shnum)):
            if byte2int(self.shhdr[i].sh_type) == type:
                if name == self.read_section_name(byte2int(self.shhdr[i].sh_name)):
                    print(Colors.BOLD + Colors.yellow + "STRING TABLE:" + Colors.ENDC)
                    self.so.seek(byte2int(self.shhdr[i].sh_offset), 0)
                    symbol_tb = self.so.read(byte2int(self.shhdr[i].sh_size))
                    for byte in symbol_tb:
                        print(chr(byte), end='')
                        if chr(byte) == '\0': print()


    def dump_st_entries(self):
        header = ["name_index", "name", "value", "size", "info", "other", "shndx", "bind", "type"]
        idx_list = [byte2int(entry.st_name) for entry in self.string_tb_e]
        name_list = [ "".join(self.get_st_entry_symbol_string(byte2int(entry.st_name), ".strtab")) for entry in self.string_tb_e]
        value_list = [byte2int(entry.st_value) for entry in self.string_tb_e]
        size_list = [byte2int(entry.st_size) for entry in self.string_tb_e]
        info_list = [byte2int(entry.st_info) for entry in self.string_tb_e]
        other_list = [byte2int(entry.st_other) for entry in self.string_tb_e]
        shndx_list = [byte2int(entry.st_shndx) for entry in self.string_tb_e]
        bind_list = [get_elf_st_bind_string(entry.st_bind) for entry in self.string_tb_e]
        type_list = [get_elf_st_type_string(entry.st_type) for entry in self.string_tb_e]

        lines = ffs(2, header, True, idx_list, name_list, value_list, size_list, info_list, other_list, shndx_list, bind_list, type_list)
        for line in lines:
            print(line)

    def dump_st_entries_dyn(self):
        header = ["name_index", "name", "value", "size", "info", "other", "shndx", "bind", "type"]
        idx_list = [byte2int(entry.st_name) for entry in self.string_tb_e_dyn]
        name_list = [ "".join(self.get_st_entry_symbol_string(byte2int(entry.st_name), ".dynstr")) for entry in self.string_tb_e_dyn]
        value_list = [byte2int(entry.st_value) for entry in self.string_tb_e_dyn]
        size_list = [byte2int(entry.st_size) for entry in self.string_tb_e_dyn]
        info_list = [byte2int(entry.st_info) for entry in self.string_tb_e_dyn]
        other_list = [byte2int(entry.st_other) for entry in self.string_tb_e_dyn]
        shndx_list = [byte2int(entry.st_shndx) for entry in self.string_tb_e_dyn]
        bind_list = [get_elf_st_bind_string(entry.st_bind) for entry in self.string_tb_e_dyn]
        type_list = [get_elf_st_type_string(entry.st_type) for entry in self.string_tb_e_dyn]

        lines = ffs(2, header, True, idx_list, name_list, value_list, size_list, info_list, other_list, shndx_list, bind_list, type_list)
        for line in lines:
            print(line)

    def dump_dyn_sec_ents(self, who):
        header = ["type_string", "tag", "value", "value(hex)"]
        tag_string_list = [entry["type_string"] for entry in who]
        tag_list = [entry["tag"] for entry in who]
        value_list = [entry["value"] for entry in who]
        value_list_hex = [hex(entry["value"]) for entry in who]
        lines = ffs(2, header, True, tag_string_list, tag_list, value_list, value_list_hex)
        for line in lines:
            print(line)

    def dump_rela(self, to_dump):
        header = ["r_offset", "r_info", "r_addend"]
        r_offset_list = [entry["r_offset"] for entry in to_dump]
        r_info_list = [entry["r_info"] for entry in to_dump]
        addend_list = [entry["r_addend"] for entry in to_dump]
        lines = ffs(2, header, True, r_offset_list, r_info_list, addend_list)
        for line in lines: print(line)

    def dump_rel(self, to_dump):
        header = ["r_offset", "r_info"]
        r_offset_list = [entry["r_offset"] for entry in to_dump]
        r_info_list = [entry["r_info"] for entry in to_dump]
        lines = ffs(2, header, True, r_offset_list, r_info_list)
        for line in lines: print(line)

    def get_st_entry_symbol_string(self, index, section_name):
        symbol = []
        for i in range(0, byte2int(self.elfhdr.e_shnum)):
            name = self.read_section_name(byte2int(self.shhdr[i].sh_name))
            if byte2int(self.shhdr[i].sh_type) == sh_type_e.SHT_STRTAB and name == section_name:
                self.so.seek(byte2int(self.shhdr[i].sh_offset) + index, 0)
                byte = self.so.read(1)
                while chr(byte[0]) != "\0":
                    if chr(byte[0]) != "\0": symbol.append(chr(byte[0]))
                    byte = self.so.read(1)
                return symbol

    def get_symbol_string_table(self, offset):
        symbol = []
        for i in range(0, int.from_bytes(self.elfhdr.e_shnum, byteorder="little", signed=False)):
            if int.from_bytes(self.shhdr[i].sh_type, byteorder="little", signed=False) == sh_type_e.SHT_STRTAB:
                self.so.seek(int.from_bytes(self.shhdr[i].sh_offset, byteorder="little", signed=False) + offset - 0, 0)
                byte = self.so.read(1)
                while chr(byte[0]) != "\0":
                    if chr(byte[0]) != "\0": symbol.append(chr(byte[0]))
                    byte = self.so.read(1)
                return symbol

    def dump_inst_sections(self):
        indices= []
        for section in self.shhdr:
            if int.from_bytes(section.sh_flags, byteorder="little", signed=False) & sh_flags_e.SHF_EXECINSTR == sh_flags_e.SHF_EXECINSTR:
                indices.append(int.from_bytes(section.sh_name, byteorder="little"))
        return indices

    def pop_data_section(self):
        for section in self.shhdr:
            name = self.read_section_name(byte2int(section.sh_name))
            if name == ".data":
                self.so.seek(byte2int(section.sh_offset))
                self.data_section = self.so.read(byte2int(section.sh_size))

    def pop_text_section(self):
        for section in self.shhdr:
            name = self.read_section_name(byte2int(section.sh_name))
            if name == ".text":
                self.so.seek(byte2int(section.sh_offset))
                self.text_section = self.so.read(byte2int(section.sh_size))

    def pop_dynamic_entries(self, section_name, struct_to_pop):
        for section in self.shhdr:
            name = self.read_section_name(byte2int(section.sh_name))
            if name == section_name:
                self.so.seek(byte2int(section.sh_offset))
                self.dyn_section = self.so.read(byte2int(section.sh_size))
        length = int(len(self.dyn_section))
        tag_type = int()
        type_string = str()
        value = int()
        dummy = {}
        jmp_val = int()
        if self.size == 64: jmp_val = 8
        elif self.size == 32: jmp_val = 4
        else: jmp_val = 8; print("self.size is not set for class elf.going with 8 as a default.")
        for offset in range(0, length, jmp_val*2):
            tag_type = byte2int(self.dyn_section[offset:offset+jmp_val])
            dummy["tag"] = tag_type
            value = byte2int(self.dyn_section[offset+jmp_val:offset+(jmp_val*2)])
            dummy["value"] = value
            type_string = get_ph_dynamic_ent_tag_type(tag_type)
            dummy["type_string"] = type_string
            type_string = str()
            struct_to_pop.append(dummy)
            dummy = {}

    def pop_rela(self, section_name, section_whole, to_pop):
        size = int()
        entsize = int()
        dummy = {}
        step = int()
        if self.size == 64: step = 8
        if self.size == 32: step = 4
        for section in self.shhdr:
            name = self.read_section_name(byte2int(section.sh_name))
            if name == section_name:
                self.so.seek(byte2int(section.sh_offset))
                section_whole = self.so.read(byte2int(section.sh_size))
                size = byte2int(section.sh_size)
                entsize = byte2int(section.sh_entsize)
        if entsize != 0:
            for i in range(0, int(size/entsize)):
                dummy["r_offset"] = byte2int(section_whole[i*entsize:i*entsize+step])
                dummy["r_info"] = byte2int(section_whole[i*entsize+step:i*entsize+(step*2)])
                dummy["r_addend"] = byte2int(section_whole[i*entsize+(step*2):i*entsize+(step*3)], sign=True)
                to_pop.append(dummy)
                dummy = {}

    def pop_rel(self, section_name, section_whole, to_pop):
        size = int()
        entsize = int()
        dummy = {}
        step = int()
        if self.size == 64: step = 8
        if self.size == 32: step = 4
        for section in self.shhdr:
            name = self.read_section_name(byte2int(section.sh_name))
            if name == section_name:
                self.so.seek(byte2int(section.sh_offset))
                section_whole = self.so.read(byte2int(section.sh_size))
                size = byte2int(section.sh_size)
                entsize = byte2int(section.sh_entsize)
        for i in range(0, int(size/entsize)):
            dummy["r_offset"] = byte2int(section_whole[i*entsize:i*entsize+step])
            dummy["r_info"] = byte2int(section_whole[i*entsize+step:i*entsize+(step*2)])
            to_pop.append(dummy)
            dummy = {}

    #FIXME-ELF64 only
    def pop_got(self):
        for shhdr in self.shhdr:
            name = self.read_section_name(byte2int(shhdr.sh_name))
            if name == ".got":
                self.so.seek(byte2int(shhdr.sh_offset))
                self.got = self.so.read(byte2int(shhdr.sh_size))
                self.so.seek(byte2int(shhdr.sh_offset))
                for i in range(0, int(byte2int(shhdr.sh_size)/8)):
                    self.got_ents.append(byte2int(self.so.read(8)))

    #FIXME-ELF64 only
    def pop_got_plt(self):
        for shhdr in self.shhdr:
            name = self.read_section_name(byte2int(shhdr.sh_name))
            if name == ".got.plt":
                self.so.seek(byte2int(shhdr.sh_offset))
                self.got_plt = self.so.read(byte2int(shhdr.sh_size))
                self.so.seek(byte2int(shhdr.sh_offset))
                for i in range(0, int(byte2int(shhdr.sh_size)/8)):
                    self.got_plt_ents.append(byte2int(self.so.read(8)))

    def dump_got(self):
        header = ["value"]
        value_list = [entry for entry in self.got_ents]
        lines = ffs(2, header, True, value_list)
        for line in lines:
            print(line)

    def dump_got_plt(self):
        header = ["value"]
        value_list = [entry for entry in self.got_plt_ents]
        lines = ffs(2, header, True, value_list)
        for line in lines:
            print(line)

class obj_loader():
    def __init__(self, bytes):
        self.memory = bytes()

    def load(self, obj):
        for byte in obj:
            self.memory.append(byte)

def ch_so_to_exe(path):
    so = open(path, "r+b")
    so.seek(16)
    so.write(bytes([2]))
    print(Colors.purple + "changed so to exe" + Colors.ENDC)
    so.close

def ch_exe_to_so(path):
    so = open(path, "r+b")
    so.seek(16)
    so.write(bytes([3]))
    print(Colors.purple + "changed exe to so" + Colors.ENDC)
    so.close

def elf_init():
    so = openSO_r(sys.argv[1])
    elf = ELF(so)
    elf.init(64)

def elf_get_func_names():
    so = openSO_r(sys.argv[1])
    elf = ELF(so)
    elf.init(64)
    return elf.dump_symbol_string(ELF_ST_TYPE.STT_FUNC, False)

def elf_get_text_section():
    so = openSO_r(sys.argv[1])
    elf = ELF(so)
    elf.init(64)
    return elf.dump_section(".text", False)

def elf_get_section(name):
    so = openSO_r(sys.argv[1])
    elf = ELF(so)
    elf.init(64)
    return elf.dump_section(name, False)

def elf_get_rodata_section():
    so = openSO_r(sys.argv[1])
    elf = ELF(so)
    elf.init(64)
    return elf.dump_section(".rodata", False)

# obj here means variables or what the C standard means by objects
def elf_get_obj_names():
    so = openSO_r(sys.argv[1])
    elf = ELF(so)
    elf.init(64)
    return elf.dump_symbol_string(ELF_ST_TYPE.STT_OBJECT, False)

# obj here means variables or what the C standard means by objects
def elf_get_obj_sizes():
    so = openSO_r(sys.argv[1])
    elf = ELF(so)
    elf.init(64)
    return elf.dump_obj_size(ELF_ST_TYPE.STT_OBJECT, False)

def elf_get_func_code():
    so = openSO_r(sys.argv[1])
    elf = ELF(so)
    elf.init(64)
    return elf.dump_funcs(False)

def elf_get_func_code_byname():
    so = openSO_r(sys.argv[1])
    arg = openSO_r(sys.argv[2])
    elf = ELF(so)
    elf.init(64)
    counter = 0
    hit = False
    for name in elf.dump_symbol_string(ELF_ST_TYPE.STT_FUNC, False):
        if name == arg:
            code = elf.dump_funcs(False)[counter]
            hit = True
        counter += 1
    return code

class Call_Rewriter(object):
    #def __init__(self, obj_code, arch, mode):
    def __init__(self, obj_code):
        self.obj_code = bytes(obj_code)
        self.md = Cs(CS_ARCH_X86, CS_MODE_64)
        #self.md = Cs(arch, mode)

    def dumpall(self):
        for i in self.md.disasm(self.obj_code, 0x1):
            print("0x%x:\t%s\t%s" %(i.address, i.mnemonic, i.op_str))

    def run(self):
        for i in self.md.disasm(self.obj_code, 0x1):
            if i.mnemonic == "call":
                print("0x%x:\t%s\t%s" %(i.address, i.mnemonic, i.op_str))
                print(i.bytes)

class Global_Rewriter(object):
    def __init__(self):
        pass

class Rewriter(object):
    def __init__(self, path, new_name):
        so = openSO_r(path)
        self.elf = ELF(so)
        self.elf.init(64)
        #shutil.copyfile(path, "/tmp/exe")
        self.magic_section_number = int()
        self.new_name = new_name
        self.shdr_new_size = []
        self.shdr_new_offset = []

    def fix_section_offsets(self, section_name, new_size:int, new_section:bytes):
        file_w = open(self.new_name, "wb")
        magic_number = int()
        for i in range(0, byte2int(self.elf.elfhdr.e_shnum)):
            name = self.elf.read_section_name(byte2int(self.elf.shhdr[i].sh_name))
            if section_name == name:
                self.magic_section_number = i
        print(self.magic_section_number)

        ### copy the sections before magic_number
        ### write in the new section
        ### fix section headers

        end = int()
        #for i in range(self.magic_section_number, byte2int(self.elf.elfhdr.e_shnum) + 1):
        for i in range(0, byte2int(self.elf.elfhdr.e_shnum)):
            if i > self.magic_section_number:
                extra_chunk = end % byte2int(self.elf.shhdr[i].sh_addralign)
                missing_chunk = byte2int(self.elf.shhdr[i].sh_addralign) - extra_chunk
                assert missing_chunk > 0, "missing chunk is negative"
                self.shdr_new_size.append(byte2int(self.elf.shhdr[i].sh_size))
                self.shdr_new_offset.append(end + missing_chunk%byte2int(self.elf.shhdr[i].sh_addralign))
                end = self.shdr_new_offset[-1] + self.shdr_new_size[-1]

            elif i < self.magic_section_number:
                self.shdr_new_size.append(byte2int(self.elf.shhdr[i].sh_size))
                self.shdr_new_offset.append(byte2int(self.elf.shhdr[i].sh_offset))
            elif i == self.magic_section_number:
                self.shdr_new_size.append(new_size)
                self.shdr_new_offset.append(byte2int(self.elf.shhdr[i].sh_offset))
                end = byte2int(self.elf.shhdr[i].sh_offset) + new_size
        for size in self.shdr_new_size: print(repr(i) + " new size is " + repr(size))
        for offset in self.shdr_new_offset: print(repr(i) + " new offset is " + repr(offset))

def premain(argparser):
    so = openSO_r(argparser.args.obj)
    elf = ELF(so)
    elf.init(64)
    if argparser.args.header: elf.dump_header()
    elif argparser.args.symboltable:
        elf.dump_symbol_tb(".strtab", sh_type_e.SHT_STRTAB)
        elf.dump_symbol_tb(".dynstr", sh_type_e.SHT_STRTAB)
    elif argparser.args.phdrs: elf.dump_phdrs()
    elif argparser.args.shdrs: elf.dump_shdrs()
    elif argparser.args.symbolindex: elf.dump_symbol_idx()
    elif argparser.args.stentries: elf.dump_st_entries()
    elif argparser.args.objcode: elf.dump_funcs(True)
    elif argparser.args.funcs: elf.dump_symbol_string(ELF_ST_TYPE.STT_FUNC, True)
    elif argparser.args.objs: elf.dump_symbol_string(ELF_ST_TYPE.STT_OBJECT, True)
    elif argparser.args.dynsym: elf.dump_st_entries_dyn()
    elif argparser.args.dlpath: elf.dump_section(".interp", True)
    elif argparser.args.section: elf.dump_section(argparser.args.section, True)
    elif argparser.args.test2:
        rewriter = Rewriter(argparser.args.obj, "new_exe")
        new_text = bytes()
        rewriter.fix_section_offsets(".text", 1000,  new_text)
    elif argparser.args.dumpfunc:
        counter = 0
        for name in elf.dump_symbol_string(ELF_ST_TYPE.STT_FUNC, False):
            if name == argparser.args.dumpfunc:
                print(Colors.red + Colors.BOLD + name + Colors.ENDC)
                code = elf.dump_funcs(False)[counter]
                print(code)
            counter += 1
    elif argparser.args.dumpfuncasm:
        counter = 0
        hit = False
        for name in elf.dump_symbol_string(ELF_ST_TYPE.STT_FUNC, False):
            if name == argparser.args.dumpfuncasm:
                code = elf.dump_funcs(False)[counter]
                hit = True
            counter += 1
        if hit:
            md = Cs(CS_ARCH_X86, CS_MODE_64)
            for i in md.disasm(bytes(code), 0x0):
                print(hex(i.address).ljust(7), i.mnemonic.ljust(7), i.op_str)
    elif argparser.args.phdynent: elf.dump_ph_dyn_entries()
    elif argparser.args.disass:
        for section in elf.shhdr:
            name = elf.read_section_name(byte2int(section.sh_name))
            if name == argparser.args.disass:
                if byte2int(section.sh_flags) & 0x4 != 0x04:
                    print("section is not executable...but, since you asked, here you go...")
                elf.so.seek(byte2int(section.sh_offset))
                code = elf.so.read(byte2int(section.sh_size))
                md = Cs(CS_ARCH_X86, CS_MODE_64)
                for i in md.disasm(bytes(code), 0x0):
                    print(hex(i.address).ljust(7), i.mnemonic.ljust(7), i.op_str)
    elif argparser.args.disassp != None:
        index = argparser.args.disassp
        # section not executable message
        if byte2int(elf.phdr[index].p_flags) & 0x1 != 1: print("program header section is not executable but since you asked...")
        header_offset = elf.phdr[index].p_offset
        header_size = elf.phdr[index].p_filesz
        elf.so.seek(byte2int(header_offset))
        code = elf.so.read(byte2int(header_size))
        md = Cs(CS_ARCH_X86, CS_MODE_64)
        for i in md.disasm(bytes(code), 0x0):
            print(hex(i.address).ljust(7), i.mnemonic.ljust(7), i.op_str)
    elif argparser.args.textasm:
        md = Cs(CS_ARCH_X86, CS_MODE_64)
        for i in md.disasm(bytes(elf.text_section), 0x0):
            print(hex(i.address).ljust(7), i.mnemonic.ljust(7), i.op_str)
    elif argparser.args.dynsecents: elf.dump_dyn_sec_ents(elf.dyn_section_ents)
    elif argparser.args.reladyn: elf.dump_rela(elf.rela_dyn_ents)
    elif argparser.args.relaplt: elf.dump_rela(elf.rela_plt_ents)
    elif argparser.args.got:
        elf.pop_got()
        elf.dump_got()
    elif argparser.args.gotplt:
        elf.pop_got_plt()
        elf.dump_got_plt()
    else: print("why even bother if you were not gonna type anythng decent in?")

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
