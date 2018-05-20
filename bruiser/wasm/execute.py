from OpCodes import *
from utils import Colors, ror, rol
import numpy as np
import math


class Label():
    def __init__(self, arity, name):
        self.arity = arity
        self.name = name


class Frame():
    def __init__(self, arity, local_indices, self_ref):
        self.arity = arity
        self.local_indices = local_indices
        self.self_ref = self_ref


# takes the machinestate, opcode and operand to run. updates the machinestate
class Execute(): # pragma: no cover
    def __init__(self, machinestate):
        self.machinestate = machinestate
        self.opcodeint = ''
        self.immediates = []
        self.op_gas = int()
        self.stack_top = []

    def getOPGas(self):
        return self.op_gas

    def chargeGasMem(self, mem_size_page):
        factor = 64
        self.op_gas += 64 * mem_size_page

    def chargeGas(self, opcodeint):
        if opcodeint != 64:
            self.op_gas += 1
        else:
            chargeGasMem()
            pass

    def getInstruction(self, opcodeint, immediates):
        self.opcodeint = opcodeint
        dummy = []
        #FIXME-why is it being cast to int?
        for i in immediates:
            dummy.append(int(i))
        self.immediates = dummy

    def callExecuteMethod(self):
        runmethod = self.instructionUnwinder(self.opcodeint, self.immediates, self.machinestate)
        #print (repr(hex(self.opcodeint)) + ' ' + repr(self.immediates))
        try:
            runmethod(self.opcodeint, self.immediates)
        except IndexError:
            # trap
            print(Colors.red + 'bad stack access.' + Colors.ENDC)
            val2 = self.machinestate.Stack_Omni.pop()


    def instructionUnwinder(self, opcodeint, immediates, machinestate):
        self.chargeGas(opcodeint)

        if opcodeint == 0:
            return(self.run_unreachable)
        elif opcodeint == 1:
            return(self.run_nop)
        elif opcodeint == 2:
            return(self.run_block)
        elif opcodeint == 3:
            return(self.run_loop)
        elif opcodeint == 4:
            return(self.run_if)
        elif opcodeint == 5:
            return(self.run_else)
        elif opcodeint == 11:
            return(self.run_end)
        elif opcodeint == 12:
            return(self.run_br)
        elif opcodeint == 13:
            return(self.run_br_if)
        elif opcodeint == 14:
            return(self.run_br_table)
        elif opcodeint == 15:
            return(self.run_return)
        elif opcodeint == 16:
            return(self.run_call)
        elif opcodeint == 17:
            return(self.run_call_indirect)
        elif opcodeint == 26:
            return(self.run_drop)
        elif opcodeint == 27:
            return(self.run_select)
        elif opcodeint == 32:
            return(self.run_getlocal)
        elif opcodeint == 33:
            return(self.run_setlocal)
        elif opcodeint == 34:
            return(self.run_teelocal)
        elif opcodeint == 35:
            return(self.run_getglobal)
        elif opcodeint == 36:
            return(self.run_setglobal)
        elif opcodeint >= 40 and opcodeint <= 53:
            return(self.run_load)
        elif opcodeint >= 54 and opcodeint <= 62:
            return(self.run_store)
        elif opcodeint == 63:
            return(self.run_current_memory)
        elif opcodeint == 64:
            self.chargeGasMem(immediates[0])
            return(self.run_grow_memory)
        elif opcodeint >= 65 and opcodeint <= 68:
            return(self.run_const)
        elif opcodeint == 69 or opcodeint == 80:
            return(self.run_eqz)
        elif opcodeint == 70 or opcodeint == 81 or opcodeint == 91 or opcodeint == 97:
            return(self.run_eq)
        elif opcodeint == 71 or opcodeint == 82 or opcodeint == 92 or opcodeint == 98:
            return(self.run_ne)
        elif opcodeint == 72 or opcodeint == 83:
            return(self.run_lt_s)
        elif opcodeint == 73 or opcodeint == 84:
            return(self.run_lt_u)
        elif opcodeint == 74 or opcodeint == 85:
            return(self.run_gt_s)
        elif opcodeint == 75 or opcodeint == 86:
            return(self.run_gt_u)
        elif opcodeint == 76 or opcodeint == 87:
            return(self.run_le_s)
        elif opcodeint == 77 or opcodeint == 88:
            return(self.run_le_u)
        elif opcodeint == 78 or opcodeint == 89:
            return(self.run_ge_s)
        elif opcodeint == 79 or opcodeint == 90:
            return(self.run_ge_u)
        elif opcodeint == 93 or opcodeint == 99:
            return(self.run_lt)
        elif opcodeint == 94 or opcodeint == 100:
            return(self.run_gt)
        elif opcodeint == 95 or opcodeint == 101:
            return(self.run_le)
        elif opcodeint == 96 or opcodeint == 102:
            return(self.run_ge)
        elif opcodeint == 103 or opcodeint == 121:
            return(self.run_clz)
        elif opcodeint == 104 or opcodeint == 122:
            return(self.run_ctz)
        elif opcodeint == 105 or opcodeint == 123:
            return(self.run_popcnt)
        elif opcodeint == 106 or opcodeint == 124 or opcodeint == 146 or opcodeint == 160:
            return(self.run_add)
        elif opcodeint == 107 or opcodeint == 125 or opcodeint == 147 or opcodeint == 161:
            return(self.run_sub)
        elif opcodeint == 108 or opcodeint == 126 or opcodeint == 148 or opcodeint == 162:
            return(self.run_mul)
        elif opcodeint == 109 or opcodeint == 127:
            return(self.run_div_s)
        elif opcodeint == 110 or opcodeint == 128:
            return(self.run_div_u)
        elif opcodeint == 111 or opcodeint == 129:
            return(self.run_rem_s)
        elif opcodeint == 112 or opcodeint == 130:
            return(self.run_rem_u)
        elif opcodeint == 113 or opcodeint == 131:
            return(self.run_and)
        elif opcodeint == 114 or opcodeint == 132:
            return(self.run_or)
        elif opcodeint == 115 or opcodeint == 133:
            return(self.run_xor)
        elif opcodeint == 116 or opcodeint == 134:
            return(self.run_shl)
        elif opcodeint == 117 or opcodeint == 135:
            return(self.run_shr_s)
        elif opcodeint == 118 or opcodeint == 136:
            return(self.run_shr_u)
        elif opcodeint == 119 or opcodeint == 137:
            return(self.run_rotl)
        elif opcodeint == 120 or opcodeint == 138:
            return(self.run_rotr)
        elif opcodeint == 139 or opcodeint == 153:
            return(self.run_abs)
        elif opcodeint == 140 or opcodeint == 154:
            return(self.run_neg)
        elif opcodeint == 141 or opcodeint == 155:
            return(self.run_ceil)
        elif opcodeint == 142 or opcodeint == 156:
            return(self.run_floor)
        elif opcodeint == 143 or opcodeint == 157:
            return(self.run_trunc)
        elif opcodeint == 144 or opcodeint == 158:
            return(self.run_nearest)
        elif opcodeint == 145 or opcodeint == 159:
            return(self.run_sqrt)
        elif opcodeint == 149 or opcodeint == 163:
            return(self.run_div)
        elif opcodeint == 150 or opcodeint == 164:
            return(self.run_min)
        elif opcodeint == 151 or opcodeint == 165:
            return(self.run_max)
        elif opcodeint == 152 or opcodeint == 166:
            return(self.run_copysign)
        elif opcodeint == 167:
            return(self.run_i32wrapi64)
        elif opcodeint == 168:
            return(self.run_i32trunc_sf32)
        elif opcodeint == 169:
            return(self.run_i32trunc_uf32)
        elif opcodeint == 170:
            return(self.run_i32trunc_sf64)
        elif opcodeint == 171:
            return(self.run_i32trunc_uf64)
        elif opcodeint == 172:
            return(self.run_i64extend_si32)
        elif opcodeint == 173:
            return(self.run_i64extend_ui3o)
        elif opcodeint == 174:
            return(self.run_i64trunc_sf32)
        elif opcodeint == 175:
            return(self.run_i64trunc_uf32)
        elif opcodeint == 176:
            return(self.run_i64trunc_sf64)
        elif opcodeint == 177:
            return(self.run_i64trunc_uf64)
        elif opcodeint == 178:
            return(self.run_f32convert_si32)
        elif opcodeint == 179:
            return(self.run_f32convert_ui32)
        elif opcodeint == 180:
            return(self.run_f32convert_si64)
        elif opcodeint == 181:
            return(self.run_f32convert_ui64)
        elif opcodeint == 182:
            return(self.run_f32demotef64)
        elif opcodeint == 183:
            return(self.run_f64convert_si32)
        elif opcodeint == 184:
            return(self.run_f64convert_ui32)
        elif opcodeint == 185:
            return(self.run_f64convert_si64)
        elif opcodeint == 186:
            return(self.run_f64convert_ui64)
        elif opcodeint == 187:
            return(self.run_f64promotef32)
        elif opcodeint == 188:
            return(self.run_i32reinterpretf32)
        elif opcodeint == 189:
            return(self.run_i64reinterpretf64)
        elif opcodeint == 190:
            return(self.run_f32reinterpreti32)
        elif opcodeint == 191:
            return(self.run_f64reinterpreti64)
        else:
            raise Exception(Colors.red + 'unknown opcode' + Colors.ENDC)

    def run_unreachable(self, opcodeint, immediates):
        # trap
        raise Exception(Colors.red + "trapped." + Colors.ENDC)

    def run_nop(self, opcodeint, immediates):
        # literally do nothing
        pass

    def run_block(self, opcodeint, immediates):
        self.machinestate.Stack_Label.append(self.machinestate.Stack_Label_Height)
        self.machinestate.Stack_Label_Height += 1

    def run_loop(self, opcodeint, immediates):
        # assertion
        if not self.machinestate.Stack_Omni:
            print(Colors.red + "entered a loop. stack is empty." + Colors.ENDC)
            # exit 1
        self.machinestate.Stack_Label.append(self.machinestate.Stack_Label_Height)
        self.machinestate.Stack_Label_Height += 1
        val = self.machinestate.Stack_Omni.pop()
        if val != 0:
            pass
        else:
            pass

    def run_if(self, opcodeint, immediates):
        pass

    def run_else(self, opcodeint, immediates):
        pass

    def run_end(self, opcodeint, immediates):
        #self.machinestate.Stack_Label.pop()
        pass

    def run_br(self, opcodeint, immediates):
        if self.machinestate.Stack_Label_Height >= immediates[0] + 1:
            print(Colors.red + "label stack does not have enough labels." + Colors.ENDC)
            # exit 1
        if len(self.machinestate.Stack_Omni) < 1:
            print(Colors.red + "the value stack does not have enough values." + Colors.ENDC)
            # exit 1
        val = self.machinestate.Stack_Omni.pop()
        label = self.machinestate.Stack_Label.pop()

    def run_br_if(self, opcodeint, immediates):
        val = self.machinestate.Stack_Omni.pop()
        if val:
            pass
        else:
            self.run_br(dummy, immediates[0])

    def run_br_table(self, opcodeint, immediates):
        pass

    def run_return(self, opcodeint, immediates):
        pass

    def run_call(self, opcodeint, immediates):
        pass

    def run_call_indirect(self, opcodeint, immediates):
        pass

    def run_drop(self, opcodeint, immediates):
        self.machinestate.Stack_Omni.pop()

    def run_select(self, opcodeint, immediates):
        pass

    def run_getlocal(self, opcodeint, immediates):
        local = self.machinestate.Index_Space_Locals[int(immediates[0])]
        self.machinestate.Stack_Omni.append(local)

    def run_setlocal(self, opcodeint, immediates):
        self.machinestate.Index_Space_Locals[int(immediates[0])] = self.machinestate.Stack_Omni.pop()

    def run_teelocal(self, dummy, immediates):
        # @DEVI-we dont pop and push
        self.machinestate.Index_Space_Locals[int(immediates[0])] = self.machinestate.Stack_Omni[-1]

    def run_getglobal(self, opcodeint, immediates):
        val = self.machinestate.Index_Space_Global[immediates[0]]
        self.machinestate.Stack_Omni.append(val)

    def run_setglobal(self, opcodeint, immediates):
        val = self.machinestate.Stack_Omni.pop()
        self.machinestate.Index_Space_Global = val

    # currently only one linear memory is allowed so thats the default.
    def run_load(self, opcodeint, immediates):
        if opcodeint == 40:
            bytes = self.machinestate.Linear_Memory[0][int(immediates[1]):int(immediates) + 4]
            self.machinestate.Stack_Omni.append(np.int32(bytes))
        elif opcodeint == 41:
            bytes = self.machinestate.Linear_Memory[0][int(immediates[1]):int(immediates) + 8]
            self.machinestate.Stack_Omni.append(np.int64(bytes))
        elif opcodeint == 42:
            bytes = self.machinestate.Linear_Memory[0][int(immediates[1]):int(immediates) + 4]
            self.machinestate.Stack_Omni.append(np.float32(bytes))
        elif opcodeint == 43:
            bytes = self.machinestate.Linear_Memory[0][int(immediates[1]):int(immediates) + 8]
            self.machinestate.Stack_Omni.append(np.float64(bytes))
        elif opcodeint == 44:
            temp = np.int8(self.machinestate.Linear_Memory[0][int(immediates[1])])
            temp2 = (temp & 0x0000007f) | ((temp & 0x80) << 24)
            self.machinestate.append(np.int32(tmep2))
        elif opcodeint == 45:
            temp = np.int8(self.machinestate.Linear_Memory[0][int(immediates[1])])
            temp2 = temp & 0x000000ff
            self.machinestate.append(np.uint32(tmep2))
        elif opcodeint == 46:
            temp = np.int8(self.machinestate.Linear_Memory[0][int(immediates[1]):int(immediates[1] + 2)])
            temp2 = (temp & 0x00007fff) | ((temp & 0x8000) << 16)
            self.machinestate.append(np.int32(tmep2))
        elif opcodeint == 47:
            temp = np.int8(self.machinestate.Linear_Memory[0][int(immediates[1]):int(immediates[1] + 2)])
            temp2 = temp & 0x0000ffff
            self.machinestate.append(np.uint32(tmep2))
        elif opcodeint == 48:
            temp = np.int8(self.machinestate.Linear_Memory[0][int(immediates[1])])
            temp2 = (temp & 0x000000000000007f) | ((temp & 0x80) << 56)
            self.machinestate.append(np.int64(tmep2))
        elif opcodeint == 49:
            temp = np.uint8(self.machinestate.Linear_Memory[0][int(immediates[1])])
            self.machinestate.append(np.uint64(tmep))
        elif opcodeint == 50:
            temp = np.int8(self.machinestate.Linear_Memory[0][int(immediates[1]):int(immediates[1] + 2)])
            temp2 = (temp & 0x0000000000007fff) | ((temp & 0x8000) << 48)
            self.machinestate.append(np.int64(tmep2))
        elif opcodeint == 51:
            temp = np.uint8(self.machinestate.Linear_Memory[0][int(immediates[1]):int(immediates[1] + 2)])
            self.machinestate.append(np.uint64(tmep))
        elif opcodeint == 52:
            temp = np.int8(self.machinestate.Linear_Memory[0][int(immediates[1]):int(immediates[1] + 4)])
            temp2 = (temp & 0x000000007fffffff) | ((temp & 0x80000000) << 32)
            self.machinestate.append(np.int64(tmep2))
        elif opcodeint == 53:
            temp = np.uint8(self.machinestate.Linear_Memory[0][int(immediates[1]):int(immediates[1] + 4)])
            self.machinestate.append(np.uint64(tmep))
        else:
            raise Exception(Colors.red + 'invalid load instruction.' + Colors.ENDC)

    # currently only one linear memory is allowed so thats the default.
    def run_store(self, opcodeint, immediates):
        if opcodeint == 54:
            val = self.machinestate.Stack_Omni.pop()
            self.machinestate.Linear_Memory[0][int(immediates[1]) + 0] = val & 0x000000ff
            self.machinestate.Linear_Memory[0][int(immediates[1]) + 1] = val & 0x0000ff00 >> 8
            self.machinestate.Linear_Memory[0][int(immediates[1]) + 2] = val & 0x00ff0000 >> 16
            self.machinestate.Linear_Memory[0][int(immediates[1]) + 3] = val & 0xff000000 >> 24
        elif opcodeint == 55:
            val = self.machinestate.Stack_Omni.pop()
            self.machinestate.Linear_Memory[0][int(immediates[1]) + 0] = val & 0x00000000000000ff
            self.machinestate.Linear_Memory[0][int(immediates[1]) + 1] = val & 0x000000000000ff00 >> 8
            self.machinestate.Linear_Memory[0][int(immediates[1]) + 2] = val & 0x0000000000ff0000 >> 16
            self.machinestate.Linear_Memory[0][int(immediates[1]) + 3] = val & 0x00000000ff000000 >> 24
            self.machinestate.Linear_Memory[0][int(immediates[1]) + 4] = val & 0x000000ff00000000 >> 32
            self.machinestate.Linear_Memory[0][int(immediates[1]) + 5] = val & 0x0000ff0000000000 >> 40
            self.machinestate.Linear_Memory[0][int(immediates[1]) + 6] = val & 0x00ff000000000000 >> 48
            self.machinestate.Linear_Memory[0][int(immediates[1]) + 7] = val & 0xff00000000000000 >> 56
        # @DEVI-FIXME-needs reinterpret cast
        elif opcodeint == 56:
            pass
        # @DEVI-FIXME-needs reinterpret cast
        elif opcodeint == 57:
            pass
        elif opcodeint == 58:
            val = self.machinestate.Stack_Omni.pop()
            self.machinestate.Linear_Memory[0][int(immediates[1])] = np.in8(val & 0x000000ff)
        elif opcodeint == 59:
            val = self.machinestate.Stack_Omni.pop()
            self.machinestate.Linear_Memory[0][int(immediates[1]) + 0] = np.in8(val & 0x000000ff)
            self.machinestate.Linear_Memory[0][int(immediates[1]) + 1] = np.in8(val & 0x0000ff00 >> 8)
        elif opcodeint == 60:
            val = self.machinestate.Stack_Omni.pop()
            self.machinestate.Linear_Memory[0][int(immediates[1])] = np.in8(val & 0x00000000000000ff)
        elif opcodeint == 61:
            val = self.machinestate.Stack_Omni.pop()
            self.machinestate.Linear_Memory[0][int(immediates[1]) + 0] = np.in8(val & 0x00000000000000ff)
            self.machinestate.Linear_Memory[0][int(immediates[1]) + 1] = np.in8(val & 0x000000000000ff00 >> 8)
        elif opcodeint == 62:
            val = self.machinestate.Stack_Omni.pop()
            self.machinestate.Linear_Memory[0][int(immediates[1]) + 0] = np.in8(val & 0x00000000000000ff)
            self.machinestate.Linear_Memory[0][int(immediates[1]) + 1] = np.in8(val & 0x000000000000ff00 >> 8)
            self.machinestate.Linear_Memory[0][int(immediates[1]) + 2] = np.in8(val & 0x0000000000ff0000 >> 16)
            self.machinestate.Linear_Memory[0][int(immediates[1]) + 3] = np.in8(val & 0x00000000ff000000 >> 24)
        else:
            raise Exception(Colors.red + 'invalid store instruction' + Colors.ENDC)

    def run_current_memory(self, opcodeint, immediates):
        pass

    def run_grow_memory(self, opcodeint, immediates):
        pass

    def run_const(self, opcodeint, immediates):
        if opcodeint == 65:
            self.machinestate.Stack_Omni.append(immediates[0])
        elif opcodeint == 66:
            self.machinestate.Stack_Omni.append(immediates[0])
        elif opcodeint == 67:
            self.machinestate.Stack_Omni.append(immediates[0])
        elif opcodeint == 68:
            self.machinestate.Stack_Omni.append(immediates[0])
        else:
            raise Exception(Colors.red + 'invalid const instruction' + Colors.ENDC)

    def run_eqz(self, opcodeint, immediates):
        if opcodeint == 69 or opcodeint == 80:
            val = self.machinestate.Stack_Omni.pop()
            if val == 0:
                self.machinestate.Stack_Omni.append(1)
            else:
                self.machinestate.Stack_Omni.append(0)
        else:
            raise Exception(Colors.red + 'invalid eqz instruction' + Colors.ENDC)

    def run_eq(self, opcodeint, immediates):
        if opcodeint == 70 or opcodeint == 81 or opcodeint == 91 or opcodeint == 97:
            val2 = self.machinestate.Stack_Omni.pop()
            val1 = self.machinestate.Stack_Omni.pop()
            if val1 == val2:
                self.machinestate.Stack_Omni.append(1)
            else:
                self.machinestate.Stack_Omni.append(0)
        else:
            raise Exception(Colors.red + 'invalid eq instruction' + Colors.ENDC)

    def run_ne(self, opcodeint, immediates):
        if opcodeint == 71 or opcodeint == 82 or opcodeint == 92 or opcodeint == 98:
            val2 = self.machinestate.Stack_Omni.pop()
            val1 = self.machinestate.Stack_Omni.pop()
            if val1 != val2:
                self.machinestate.Stack_Omni.append(1)
            else:
                self.machinestate.Stack_Omni.append(0)
        else:
            raise Exception(Colors.red + 'invalid ne instruction' + Colors.ENDC)

    def run_lt_s(self, opcodeint, immediates):
        val2 = self.machinestate.Stack_Omni.pop()
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 72:
            if np.int32(val1) < np.int32(val2):
                self.machinestate.Stack_Omni.append(1)
            else:
                self.machinestate.Stack_Omni.append(0)
        elif opcodeint == 83:
            if np.int64(val1) < np.int64(val2):
                self.machinestate.Stack_Omni.append(1)
            else:
                self.machinestate.Stack_Omni.append(0)
        else:
            raise Exception(Colors.red + 'invalid lt_s instruction' + Colors.ENDC)

    def run_lt_u(self, opcodeint, immediates):
        val2 = self.machinestate.Stack_Omni.pop()
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 73:
            if np.uint32(val1) < np.uint32(val2):
                self.machinestate.Stack_Omni.append(1)
            else:
                self.machinestate.Stack_Omni.append(0)
        elif opcodeint == 84:
            if np.uint64(val1) < np.uint64(val2):
                self.machinestate.Stack_Omni.append(1)
            else:
                self.machinestate.Stack_Omni.append(0)
        else:
            raise Exception(Colors.red + 'invalid lt_u instruction' + Colors.ENDC)

    def run_gt_s(self, opcodeint, immediates):
        val2 = self.machinestate.Stack_Omni.pop()
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 74:
            if np.int32(val1) > np.int32(val2):
                self.machinestate.Stack_Omni.append(1)
            else:
                self.machinestate.Stack_Omni.append(0)
        elif opcodeint == 85:
            if np.int64(val1) > np.int64(val2):
                self.machinestate.Stack_Omni.append(1)
            else:
                self.machinestate.Stack_Omni.append(0)
        else:
            raise Exception(Colors.red + 'invalid gt_s instruction' + Colors.ENDC)

    def run_gt_u(self, opcodeint, immediates):
        val2 = self.machinestate.Stack_Omni.pop()
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 75:
            if np.uint32(val1) > np.uint32(val2):
                self.machinestate.Stack_Omni.append(1)
            else:
                self.machinestate.Stack_Omni.append(0)
        elif opcodeint == 86:
            if np.uint64(val1) > np.uint64(val2):
                self.machinestate.Stack_Omni.append(1)
            else:
                self.machinestate.Stack_Omni.append(0)
        else:
            raise Exception(Colors.red + 'invalid gt_u instruction' + Colors.ENDC)

    def run_le_s(self, opcodeint, immediates):
        val2 = self.machinestate.Stack_Omni.pop()
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 76:
            if np.int32(val1) <= np.int32(val2):
                self.machinestate.Stack_Omni.append(1)
            else:
                self.machinestate.Stack_Omni.append(0)
        elif opcodeint == 87:
            if np.int64(val1) <= np.int64(val2):
                self.machinestate.Stack_Omni.append(1)
            else:
                self.machinestate.Stack_Omni.append(0)
        else:
            raise Exception(Colors.red + 'invalid le_s instruction' + Colors.ENDC)

    def run_le_u(self, opcodeint, immediates):
        val2 = self.machinestate.Stack_Omni.pop()
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 77:
            if np.uint32(val1) <= np.uint32(val2):
                self.machinestate.Stack_Omni.append(1)
            else:
                self.machinestate.Stack_Omni.append(0)
        elif opcodeint == 88:
            if np.uint64(val1) <= np.uint64(val2):
                self.machinestate.Stack_Omni.append(1)
            else:
                self.machinestate.Stack_Omni.append(0)
        else:
            raise Exception(Colors.red + 'invalid le_u instruction' + Colors.ENDC)

    def run_ge_s(self, opcodeint, immediates):
        val2 = self.machinestate.Stack_Omni.pop()
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 78:
            if np.int32(val1) >= np.int32(val2):
                self.machinestate.Stack_Omni.append(1)
            else:
                self.machinestate.Stack_Omni.append(0)
        elif opcodeint == 89:
            if np.int64(val1) >= np.int64(val2):
                self.machinestate.Stack_Omni.append(1)
            else:
                self.machinestate.Stack_Omni.append(0)
        else:
            raise Exception(Colors.red + 'invalid ge_s instruction' + Colors.ENDC)

    def run_ge_u(self, opcodeint, immediates):
        val2 = self.machinestate.Stack_Omni.pop()
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 79:
            if np.uint32(val1) >= np.uint32(val2):
                self.machinestate.Stack_Omni.append(1)
            else:
                self.machinestate.Stack_Omni.append(0)
        elif opcodeint == 90:
            if np.uint64(val1) >= np.uint64(val2):
                self.machinestate.Stack_Omni.append(1)
            else:
                self.machinestate.Stack_Omni.append(0)
        else:
            raise Exception(Colors.red + 'invalid ge_u instruction' + Colors.ENDC)

    def run_lt(self, opcodeint, immediates):
        val2 = self.machinestate.Stack_Omni.pop()
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 93:
            if np.float32(v1) < np.float32(v2):
                self.machinestate.Stack_Omni.append(1)
            else:
                self.machinestate.Stack_Omni.append(0)
        elif opcodeint == 99:
            if np.float64(v1) < np.float64(v2):
                self.machinestate.Stack_Omni.append(1)
            else:
                self.machinestate.Stack_Omni.append(0)
        else:
            raise Exception(Colors.red + 'invalid lt instruction' + Colors.ENDC)

    def run_gt(self, opcodeint, immediates):
        val2 = self.machinestate.Stack_Omni.pop()
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 94:
            if np.float32(v1) > np.float32(v2):
                self.machinestate.Stack_Omni.append(1)
            else:
                self.machinestate.Stack_Omni.append(0)
        elif opcodeint == 100:
            if np.float64(v1) > np.float64(v2):
                self.machinestate.Stack_Omni.append(1)
            else:
                self.machinestate.Stack_Omni.append(0)
        else:
            raise Exception(Colors.red + 'invalid gt instruction' + Colors.ENDC)

    def run_le(self, opcodeint, immediates):
        val2 = self.machinestate.Stack_Omni.pop()
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 95:
            if np.float32(v1) <= np.float32(v2):
                self.machinestate.Stack_Omni.append(1)
            else:
                self.machinestate.Stack_Omni.append(0)
        elif opcodeint == 101:
            if np.float64(v1) <= np.float64(v2):
                self.machinestate.Stack_Omni.append(1)
            else:
                self.machinestate.Stack_Omni.append(0)
        else:
            raise Exception(Colors.red + 'invalid le instruction' + Colors.ENDC)

    def run_ge(self, opcodeint, immediates):
        val2 = self.machinestate.Stack_Omni.pop()
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 96:
            if np.float32(v1) >= np.float32(v2):
                self.machinestate.Stack_Omni.append(1)
            else:
                self.machinestate.Stack_Omni.append(0)
        elif opcodeint == 102:
            if np.float64(v1) >= np.float64(v2):
                self.machinestate.Stack_Omni.append(1)
            else:
                self.machinestate.Stack_Omni.append(0)
        else:
            raise Exception(Colors.red + 'invalid ge instruction' + Colors.ENDC)

    def run_clz(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 103:
            self.machinestate.Stack_Omni.append(clz(val, 'uint32'))
        elif opcodeint == 121:
            self.machinestate.Stack_Omni.append(clz(val, 'uint64'))
        else:
            raise Exception(Colors.red + 'invalid clz instruction' + Colors.ENDC)

    def run_ctz(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 104:
            self.machinestate.Stack_Omni.append(ctz(val, 'uint32'))
        elif opcodeint == 122:
            self.machinestate.Stack_Omni.append(ctz(val, 'uint64'))
        else:
            raise Exception(Colors.red + 'invalid ctz instruction' + Colors.ENDC)

    def run_popcnt(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 105:
            self.machinestate.Stack_Omni.append(pop_cnt(val, 'uint32'))
        elif opcodeint == 123:
            self.machinestate.Stack_Omni.append(pop_cnt(val, 'uint64'))
        else:
            raise Exception(Colors.red + 'invalid popcnt instruction' + Colors.ENDC)

    def run_add(self, opcodeint, immediates):
        val2 = self.machinestate.Stack_Omni.pop()
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 106:
            self.machinestate.Stack_Omni.append(np.uint32(val1 + val2))
        elif opcodeint == 124:
            self.machinestate.Stack_Omni.append(np.uint64(val1 + val2))
        elif opcodeint == 146:
            self.machinestate.Stack_Omni.append(np.float32(val1 + val2))
        elif opcodeint == 160:
            self.machinestate.Stack_Omni.append(np.float64(val1 + val2))
        else:
            raise Exception(Colors.red + 'invalid add instruction' + Colors.ENDC)

    def run_sub(self, opcodeint, immediates):
        val2 = self.machinestate.Stack_Omni.pop()
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 107:
            self.machinestate.Stack_Omni.append(np.uint32(val1 - val2))
        elif opcodeint == 125:
            self.machinestate.Stack_Omni.append(np.uint64(val1 - val2))
        elif opcodeint == 147:
            self.machinestate.Stack_Omni.append(np.float32(val1 - val2))
        elif opcodeint == 161:
            self.machinestate.Stack_Omni.append(np.float64(val1 - val2))
        else:
            raise Exception(Colors.red + 'invalid sub instruction' + Colors.ENDC)

    def run_mul(self, opcodeint, immediates):
        val2 = self.machinestate.Stack_Omni.pop()
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 108:
            self.machinestate.Stack_Omni.append(np.uint32(val1 * val2))
        elif opcodeint == 126:
            self.machinestate.Stack_Omni.append(np.uint64(val1 * val2))
        elif opcodeint == 148:
            self.machinestate.Stack_Omni.append(np.float32(val1 * val2))
        elif opcodeint == 162:
            self.machinestate.Stack_Omni.append(np.float64(val1 * val2))
        else:
            raise Exception(Colors.red + 'invalid mul instruction' + Colors.ENDC)

    def run_div_s(self, opcodeint, immediates):
        val2 = self.machinestate.Stack_Omni.pop()
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 109:
            self.machinestate.Stack_Omni.append(np.int32(np.int32(val1) / np.int32(val2)))
        elif opcodeint == 127:
            self.machinestate.Stack_Omni.append(np.int64(np.int64(val1) / np.int64(val2)))
        else:
            raise Exception(Colors.red + 'invalid div_s instruction' + Colors.ENDC)

    def run_div_u(self, opcodeint, immediates):
        val2 = self.machinestate.Stack_Omni.pop()
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 110:
            self.machinestate.Stack_Omni.append(np.uint32(np.uint32(val1) / np.uint32(val2)))
        elif opcodeint == 128:
            self.machinestate.Stack_Omni.append(np.uint64(np.uint64(val1) / np.uint64(val2)))
        else:
            raise Exception(Colors.red + 'invalid div_u instruction' + Colors.ENDC)

    def run_rem_s(self, opcodeint, immediates):
        val2 = self.machinestate.Stack_Omni.pop()
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 111:
            self.machinestate.Stack_Omni.append(np.int32(np.int32(val1) % np.int32(val2)))
        elif opcodeint == 129:
            self.machinestate.Stack_Omni.append(np.int64(np.int64(val1) % np.int64(val2)))
        else:
            raise Exception(Colors.red + 'invalid rem_s instruction' + Colors.ENDC)

    def run_rem_u(self, opcodeint, immediates):
        val2 = self.machinestate.Stack_Omni.pop()
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 112:
            self.machinestate.Stack_Omni.append(np.uint32(np.uint32(val1) % np.uint32(val2)))
        elif opcodeint == 130:
            self.machinestate.Stack_Omni.append(np.uint64(np.uint64(val1) % np.uint64(val2)))
        else:
            raise Exception(Colors.red + 'invalid rem_u instruction' + Colors.ENDC)

    def run_and(self, opcodeint, immediates):
        val2 = self.machinestate.Stack_Omni.pop()
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 113:
            self.machinestate.Stack_Omni.append(np.uint32(np.uint32(val1) & np.uint32(val2)))
        elif opcodeint == 131:
            self.machinestate.Stack_Omni.append(np.uint64(np.uint64(val1) & np.uint64(val2)))
        else:
            raise Exception(Colors.red + 'invalid and instruction' + Colors.ENDC)

    def run_or(self, opcodeint, immediates):
        val2 = self.machinestate.Stack_Omni.pop()
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 114:
            self.machinestate.Stack_Omni.append(np.uint32(np.uint32(val1) | np.uint32(val2)))
        elif opcodeint == 132:
            self.machinestate.Stack_Omni.append(np.uint64(np.uint64(val1) | np.uint64(val2)))
        else:
            raise Exception(Colors.red + 'invalid or instruction' + Colors.ENDC)

    def run_xor(self, opcodeint, immediates):
        val2 = self.machinestate.Stack_Omni.pop()
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 115:
            self.machinestate.Stack_Omni.append(np.uint32(np.uint32(val1) ^ np.uint32(val2)))
        elif opcodeint == 133:
            self.machinestate.Stack_Omni.append(np.uint64(np.uint64(val1) ^ np.uint64(val2)))
        else:
            raise Exception(Colors.red + 'invalid xor instruction' + Colors.ENDC)

    def run_shl(self, opcodeint, immediates):
        val2 = self.machinestate.Stack_Omni.pop()
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 116:
            self.machinestate.Stack_Omni.append(np.uint32(np.uint32(val1) << (np.uint32(val2))))
        elif opcodeint == 134:
            self.machinestate.Stack_Omni.append(np.uint64(np.uint64(val1) << (np.uint64(val2))))
        else:
            raise Exception(Colors.red + 'invalid shl instruction' + Colors.ENDC)

    def run_shr_s(self, opcodeint, immediates):
        val2 = self.machinestate.Stack_Omni.pop()
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 117:
            self.machinestate.Stack_Omni.append(np.int32(np.int32(val1) >> (np.int32(val2))))
        elif opcodeint == 135:
            self.machinestate.Stack_Omni.append(np.int64(np.int64(val1) >> (np.int64(val2))))
        else:
            raise Exception(Colors.red + 'invalid shr_s instruction' + Colors.ENDC)

    def run_shr_u(self, opcodeint, immediates):
        val2 = self.machinestate.Stack_Omni.pop()
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 118:
            self.machinestate.Stack_Omni.append(np.uint32(np.uint32(val1) >> (np.uint32(val2))))
        elif opcodeint == 136:
            self.machinestate.Stack_Omni.append(np.uint64(np.uint64(val1) >> (np.uint64(val2))))
        else:
            raise Exception(Colors.red + 'invalid shr_u instruction' + Colors.ENDC)

    def run_rotl(self, opcodeint, immediates):
        val2 = self.machinestate.Stack_Omni.pop()
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 119:
            self.machinestate.Stack_Omni.append(rol(val1, 32, val2))
        elif opcodeint == 137:
            self.machinestate.Stack_Omni.append(rol(val1, 64, val2))
        else:
            raise Exception(Colors.red + 'invalid rotl instruction' + Colors.ENDC)

    def run_rotr(self, opcodeint, immediates):
        val2 = self.machinestate.Stack_Omni.pop()
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 120:
            self.machinestate.Stack_Omni.append(ror(val1, 32, val2))
        elif opcodeint == 138:
            self.machinestate.Stack_Omni.append(ror(val1, 32, val2))
        else:
            raise Exception(Colors.red + 'invalid rotl instruction' + Colors.ENDC)

    def run_abs(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 139 or opcodeint == 153:
            self.machinestate.Stack_Omni.append(abs(val1))
        else:
            raise Exception(Colors.red + 'invalid abs instruction' + Colors.ENDC)

    def run_neg(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 140 or opcodeint == 154:
            self.machinestate.Stack_Omni.append(-val1)
        else:
            raise Exception(Colors.red + 'invalid neg instruction' + Colors.ENDC)

    def run_ceil(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 141 or opcodeint == 155:
            self.machinestate.Stack_Omni.append(math.ceil(val1))
        else:
            raise Exception(Colors.red + 'invalid ceil instruction' + Colors.ENDC)

    def run_floor(self, opcodeint, immediates):
        if opcodeint == 142 or opcodeint == 156:
            self.machinestate.Stack_Omni.append(math.floor(val1))
        else:
            raise Exception(Colors.red + 'invalid floor instruction' + Colors.ENDC)

    def run_trunc(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 143 or opcodeint == 157:
            self.machinestate.Stack_Omni.append(math.trunc(val1))
        else:
            raise Exception(Colors.red + 'invalid trunc instruction' + Colors.ENDC)

    def run_nearest(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 144 or opcodeint == 158:
            self.machinestate.Stack_Omni.append(round(val1))
        else:
            raise Exception(Colors.red + 'invalid nearest instruction' + Colors.ENDC)

    def run_sqrt(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 145 or opcodeint == 159:
            self.machinestate.Stack_Omni.append(math.sqrt(val1))
        else:
            raise Exception(Colors.red + 'invalid sqrt instruction' + Colors.ENDC)

    def run_div(self, opcodeint, immediates):
        val2 = self.machinestate.Stack_Omni.pop()
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 149:
            self.machinestate.Stack_Omni.append(v1 / v2)
        else:
            raise Exception(Colors.red + 'invalid float div instruction' + Colors.ENDC)

    def run_min(self, opcodeint, immediates):
        val2 = self.machinestate.Stack_Omni.pop()
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 150 or opcodeint == 164:
            self.machinestate.Stack_Omni.append(min(val1, val2))
        else:
            raise Exception(Colors.red + 'invalid min instruction' + Colors.ENDC)

    def run_max(self, opcodeint, immediates):
        val2 = self.machinestate.Stack_Omni.pop()
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 151 or opcodeint == 165:
            self.machinestate.Stack_Omni.append(max(val1, val2))
        else:
            raise Exception(Colors.red + 'invalid max instruction' + Colors.ENDC)

    def run_copysign(self, opcodeint, immediates):
        val2 = self.machinestate.Stack_Omni.pop()
        val1 = self.machinestate.Stack_Omni.pop()
        if opcodeint == 152 or opcodeint == 166:
            self.machinestate.Stack_Omni.append(math.copysign(val1, val2))
        else:
            raise Exception(Colors.red + 'invalid max instruction' + Colors.ENDC)

    def run_i32wrapi64(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        self.machinestate.Stack_Omni.append(np.int32(np.float64(val1)))

    def run_i32trunc_sf32(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        self.machinestate.Stack_Omni.append(np.int32(np.float32(val1)))

    def run_i32trunc_uf32(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        self.machinestate.Stack_Omni.append(np.uint32(np.float32(val1)))

    def run_i32trunc_sf64(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        self.machinestate.Stack_Omni.append(np.int32(np.float64(val1)))

    def run_i32trunc_uf64(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        self.machinestate.Stack_Omni.append(np.int32(np.float64(val1)))

    def run_i64extend_si32(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        self.machinestate.Stack_Omni.append(np.float64(np.int32(val1)))

    def run_i64extend_ui32(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        self.machinestate.Stack_Omni.append(np.float64(np.uint32(val1)))

    def run_i64trunc_sf32(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        self.machinestate.Stack_Omni.append(np.int64(np.float32(val1)))

    def run_i64trunc_uf32(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        self.machinestate.Stack_Omni.append(np.uint64(np.float32(val1)))

    def run_i64trunc_sf64(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        self.machinestate.Stack_Omni.append(np.int64(np.float64(val1)))

    def run_i64trunc_uf64(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        self.machinestate.Stack_Omni.append(np.uint64(np.float64(val1)))

    def run_f32convert_si32(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        self.machinestate.Stack_Omni.append(np.float32(np.uint32(val1)))

    def run_f32convert_ui32(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        self.machinestate.Stack_Omni.append(np.float32(np.int32(val1)))

    def run_f32convert_si64(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        self.machinestate.Stack_Omni.append(np.float32(np.int64(val1)))

    def run_f32convert_ui64(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        self.machinestate.Stack_Omni.append(np.float32(np.uint64(val1)))

    def run_f32demotef64(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        self.machinestate.Stack_Omni.append(np.float32(np.float64(val1)))

    def run_f64convert_si32(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        self.machinestate.Stack_Omni.append(np.float64(np.int32(val1)))

    def run_f64convert_ui32(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        self.machinestate.Stack_Omni.append(np.float64(np.uint32(val1)))

    def run_f64convert_si64(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        self.machinestate.Stack_Omni.append(np.float64(np.int64(val1)))

    def run_f64convert_ui64(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        self.machinestate.Stack_Omni.append(np.float64(np.uint64(val1)))

    def run_f64promotef32(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        self.machinestate.Stack_Omni.append(np.float64(np.float32(val1)))

    def run_i32reinterpretf32(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        sel.machinestate.Stack_Omni.append(reinterpretf32toi32(val1))

    def run_i64reinterpretf64(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        sel.machinestate.Stack_Omni.append(reinterpretf64toi64(val1))

    def run_f32reinterpreti32(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        sel.machinestate.Stack_Omni.append(reinterpreti32tof32(val1))

    def run_f64reinterpreti64(self, opcodeint, immediates):
        val1 = self.machinestate.Stack_Omni.pop()
        sel.machinestate.Stack_Omni.append(reinterpreti64tof64(val1))
