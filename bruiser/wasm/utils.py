from OpCodes import *
import numpy as np
import struct as stc

class ParseFlags:
    def __init__(self, wast_path, wasm_path, as_path, disa_path, out_path, dbg, unval, memdump
                 , idxspc, run, metric, gas, entry):
        self.wast_path = wast_path
        self.wasm_path = wasm_path
        self.as_path = as_path
        self.disa_path = disa_path
        self.out_path = out_path
        self.dbg = dbg
        self.unval = unval
        self.memdump = memdump
        self.idxspc = idxspc
        self.run = run
        self.metric = metric
        self.gas = gas
        self.entry = entry

# pretty print
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

# @DEVI-FIXME-MVP-only-we currently inly support consts and get_global
# interprets the init-exprs
def init_interpret(expr):
    offset = 0
    byte, offset, dummy = Read(expr, offset, 'uint8')
    const = int()

    if byte == 65:
        # @DEVI-FIXME-the spec says varint32, obviously we are not doing that
        # since it will return neg values which are meningless and break things
        const, offset, dummy = Read(expr, offset, 'varuint32')
    elif byte == 66:
        const, offset, dummy = Read(expr, offset, 'varint64')
    elif byte == 67:
        const, offset, dummy = Read(expr, offset, 'uint32')
    elif byte == 68:
        const, offset, dummy = Read(expr, offset, 'uint64')
    elif byte == 35:
        pass
    else:
        raise Exception(Colors.red + "illegal opcode for an MVP init expr." + Colors.ENDC)

    block_end, offset, dummy = Read(expr, offset, 'uint8')
    if block_end != 11:
        raise Exception(Colors.red + "init expr has no block end." + Colors.ENDC)

    return(const)

# reads different-typed values from a byte array, takes in the bytearray, the
# current offset the read should be performed from and the kind of value that
# should be read. returns the read value as a decimal number, the updated
# offset and the number of bytes read
def Read(section_byte, offset, kind):
    operand = []
    return_list = int()
    read_bytes = 0

    if kind == 'varuint1' or kind == 'varuint7' or kind == 'varuint32' or kind == 'varuint64':
        while True:
            byte = int(section_byte[offset])
            read_bytes += 1
            offset += 1

            operand.append(byte)

            if byte == 0x80:
                pass
            elif byte & 0x80 != 0:
                pass
            else:
                # we have read the last byte of the operand
                break

        return_list = LEB128UnsignedDecode(operand)
        operand = []
    elif kind == 'uint8' or kind == 'uint16' or kind == 'uint32' or kind == 'uint64':
        byte = section_byte[offset: offset + TypeDic[kind]]
        read_bytes += TypeDic[kind]
        offset += TypeDic[kind]
        operand.append(byte)
        return_list = int.from_bytes(operand[0], byteorder='little', signed=False)
        operand = []
    elif kind == 'varint1' or kind == 'varint7' or kind == 'varint32' or kind == 'varint64':
        while True:
            byte = int(section_byte[offset])
            read_bytes += 1
            offset += 1
            operand.append(byte)
            # @DEVI-what happens when we decode a 56-bit value?
            if byte == 0x80 or byte == 0xff:
                pass
            elif byte & 0x80 != 0:
                pass
            else:
                # we have read the lasy byte of the operand
                break
        return_list = LEB128SignedDecode(operand)
        operand = []
    return return_list, offset, read_bytes

def ror(val, type_length, rot_size):
    rot_size_rem = rot_size % type_length
    return (((val >> rot_size_rem) & (2**type_length - 1)) | ((val & (2**rot_size_rem - 1)) << (type_length - rot_size_rem)))

def rol(val, type_length, rot_size):
    rot_size_rem = rot_size % type_length
    return (((val << rot_size_rem) & (2**type_length - 1)) | ((val & ((2**type_length - 1) - (2**(type_length - rot_size_rem) - 1))) >> (type_length - rot_size_rem)))

# @DEVI-these are here because i wanted to test them to make sure what i thik is
# happening is really happening
def reinterpretf32toi32(val):
    return (stc.unpack("i", stc.pack("f" ,val))[0])

def reinterpretf64toi64(val):
    return (stc.unpack("Q", stc.pack("d", val))[0])

def reinterpreti32tof32(val):
    return (stc.unpack("f", stc.pack("i", val))[0])

def reinterpreti64tof64(val):
    return (stc.unpack("d", stc.pack("Q", val))[0])

# @DEVI-FIXME
def clz(val, _type):
    cnt = int()
    if _type == 'uint32':
        bits = np.uint32(val)
        power = 31
        while power > -1:
            if val & 2**power == 0:
                cnt += 1
            else:
                break
            power -= 1
    elif _type == 'uint64':
        bits = bin(np.uint64(val))
        power = 63
        while power > -1:
            if val & 2**power == 0:
                cnt += 1
            else:
                break
            power -= 1
    else:
        raise Exception(Colors.red + "unsupported type passed to clz." + Colors.ENDC)
    return cnt


# @DEVI-FIXME
def ctz(val, _type):
    cnt = int()
    power = int()
    if _type == 'uint32':
        bits = np.uint32(val)
        while power < 32:
            if val & 2**power == 0:
                cnt += 1
            else:
                break
            power += 1
    elif _type == 'uint64':
        bits = bin(np.uint64(val))
        while power < 64:
            if val & 2**power == 0:
                cnt += 1
            else:
                break
            power += 1
    else:
        raise Exception(Colors.red + "unsupported type passed to ctz." + Colors.ENDC)
    return cnt

# @DEVI-FIXME
def pop_cnt(val, _type):
    cnt = int()
    power = int()
    if _type == 'uint32':
        bits = np.uint32(val)
        while power < 32:
            if val & 2**power != 0:
                cnt += 1
            power += 1
    elif _type == 'uint64':
        bits = bin(np.uint64(val))
        while power < 64:
            if val & 2**power != 0:
                cnt += 1
        power += 1
    else:
        raise Exception(Colors.red + "unsupported type passed to pop_cnt." + Colors.ENDC)
    return cnt

def gen_label(label_stack):
    counter += 1
    label_stack.append(counter)

def dumpprettysections(sections_list, width, section_name):
    line_counter = 0
    str_list = []
    module_counter = 0
    section_offset = 0
    for sections in sections_list:
        print (Colors.cyan + Colors.BOLD + "module " + repr(module_counter) + Colors.ENDC)
        for section in sections.section_list:
            if section_name == "": pass
            else:
                if section_name != SectionID[section[0]]:
                    continue
            print(Colors.green + Colors.BOLD + SectionID[section[0]] + " section" + Colors.ENDC)
            #print(Colors.green + "length: " + Colors.blue + section[1] + Colors.ENDC)
            print(Colors.green + "length: " + Colors.blue + repr(section[2]) + Colors.ENDC)
            print(Colors.green + "is custom section: " + Colors.blue + repr(section[3]) + Colors.ENDC)
            print(Colors.green + "name length: " + Colors.blue + repr(section[4]) + Colors.ENDC)
            print(Colors.green + "name: " + Colors.blue + section[5] + Colors.ENDC)
            print("\t", end="")
            for offset in range(0, width):
                if offset <= 15:
                    print(Colors.yellow + hex(offset) + "  " + Colors.ENDC, end="")
                else:
                    print(Colors.yellow + hex(offset) + " " + Colors.ENDC, end="")
            print()
            print(Colors.yellow + Colors.BOLD + hex(section_offset) + "\t" + Colors.ENDC, end="")
            for byte in section[6]:
                if line_counter == width:
                    section_offset += width
                    #print("\t\t", end="")
                    line_counter = 0
                    for char in str_list:
                        print(Colors.green + "|" + Colors.ENDC, end="")
                        if ord(char) < 32: print(" ", end="")
                        else:  print(char, end="")
                    str_list = []
                    print()
                    print(Colors.yellow + Colors.BOLD + hex(section_offset) + "\t" + Colors.ENDC, end="")
                print(format(byte, '02x') + "   ", end="")
                str_list.append(chr(byte))
                line_counter += 1
            #print("  ", end="")
            for i in range(0, width - line_counter): print("     ", end="")
            for char in str_list:
                if ord(char) < 32: print(" ", end="")
                else:  print(char, end="")
                print(Colors.green + "|" + Colors.ENDC, end="")
            str_list = []
            line_counter = 0
            section_offset = 0
            print()
        str_list = []
        line_counter = 0
        module_counter += 1
        section_offset = 0

def popcnt32(r1):
    temp = r1
    temp = (temp & 0x55555555) + ((temp >> 1) & 0x55555555)
    temp = (temp & 0x33333333) + ((temp >> 2) & 0x33333333)
    temp = (temp & 0x0f0f0f0f) + ((temp >> 4) & 0x0f0f0f0f)
    temp = (temp & 0x00ff00ff) + ((temp >> 8) & 0x00ff00ff)
    temp = (temp & 0x0000ffff) + ((temp >> 16) & 0x0000ffff)
    return temp

def popcnt64(r1):
    temp = r1
    temp = (temp & 0x5555555555555555) + ((temp >> 1) & 0x5555555555555555)
    temp = (temp & 0x3333333333333333) + ((temp >> 2) & 0x3333333333333333)
    temp = (temp & 0x0f0f0f0f0f0f0f0f) + ((temp >> 4) & 0x0f0f0f0f0f0f0f0f)
    temp = (temp & 0x00ff00ff00ff00ff) + ((temp >> 8) & 0x00ff00ff00ff00ff)
    temp = (temp & 0x0000ffff0000ffff) + ((temp >> 16) & 0x0000ffff0000ffff)
    temp = (temp & 0x00000000ffffffff) + ((temp >> 32) & 0x00000000ffffffff)
    return temp

def clz32(r1):
    if r1 == 0: return 32
    temp_r1 = r1
    n = 0
    if temp_r1 & 0xffff0000 == 0:
        n += 16
        temp_r1 = temp_r1 << 16
    if temp_r1 & 0xff000000 == 0:
        n += 8
        temp_r1 = temp_r1 << 8
    if temp_r1 & 0xf0000000 == 0:
        n += 4
        temp_r1 = temp_r1 << 4
    if temp_r1 & 0xc0000000 == 0:
        n += 2
        temp_r1 = temp_r1 << 2
    if temp_r1 & 0x8000000 == 0:
        n += 1
    return n

def clz64(r1):
    if r1 == 0: return 64
    temp_r1 = r1
    n = 0
    if temp_r1 & 0xffffffff00000000 == 0:
        n += 32
        temp_r1 = temp_r1 << 32
    if temp_r1 & 0xffff000000000000 == 0:
        n += 16
        temp_r1 == temp_r1 << 16
    if temp_r1 & 0xff00000000000000 == 0:
        n+= 8
        temp_r1 = temp_r1 << 8
    if temp_r1 & 0xf000000000000000 == 0:
        n += 4
        temp_r1 = temp_r1 << 4
    if temp_r1 & 0xc000000000000000 == 0:
        n += 2
        temp_r1 = temp_r1 << 2
    if temp_r1 & 0x8000000000000000 == 0:
        n += 1
    return n

def ctz32(r1):
    if r1 == 0: return 32
    temp_r1 = r1
    n = 0
    if temp_r1 & 0x0000ffff == 0:
        n += 16
        temp_r1 = temp_r1 >> 16
    if temp_r1 & 0x000000ff == 0:
        n += 8
        temp_r1 = temp_r1 >> 8
    if temp_r1 & 0x0000000f == 0:
        n += 4
        temp_r1 = temp_r1 >> 4
    if temp_r1 & 0x00000003 == 0:
        n += 2
        temp_r1 = temp_r1 >> 2
    if temp_r1 & 0x00000001 == 0:
        n += 1
    return n

def ctz64(r1):
    if r1 == 0: return 64
    temp_r1 = r1
    n = 0
    if temp_r1 & 0x00000000ffffffff == 0:
        n += 32
        temp_r1 = temp_r1 >> 32
    if temp_r1 & 0x000000000000ffff == 0:
        n += 16
        temp_r1 = temp_r1 >> 16
    if temp_r1 & 0x00000000000000ff == 0:
        n += 8
        temp_r1 = temp_r1 >> 8
    if temp_r1 & 0x000000000000000f == 0:
        n += 4
        temp_r1 = temp_r1 >> 4
    if temp_r1 & 0x0000000000000003 == 0:
        n += 2
        temp_r1 = temp_r1 >> 2
    if temp_r1 & 0x0000000000000001 == 0:
        n += 1
    return n
