from utils import Colors, init_interpret, ParseFlags
from opcodes import WASM_OP_Code
from section_structs import Code_Section, Func_Body, WASM_Ins, Resizable_Limits, Memory_Section
from execute import *
import datetime as dti
import os
import sys
import signal


# McCabe cyclomatic complexity metric
class Metric():
    def __init__(self, code_section):
        self.code_section = code_section
        self.metric = []
        self.soc = []

    def mccabe(self):
        soc = 0
        Edges = 1
        Nodes = 1
        for funcs in self.code_section.func_bodies:
            for ins in funcs.code:
                soc += 1
                #print(repr(ins.opcodeint))
                if ins.opcodeint == 4 or ins.opcodeint == 5 or ins.opcodeint == 12 \
                    or ins.opcodeint == 13 or ins.opcodeint == 14:
                    Nodes += 2
                    Edges += 4
                elif ins.opcode == 3:
                    Nodes += 2
                    Edges += 3
                else:
                    pass

            self.metric.append(Edges - Nodes + 1)
            self.soc.append(soc)
            soc = 0
            Edges = 1
            Nodes = 1

    def getMcCabe(self):
        return self.metric

    def getSOC(self):
        return self.soc


# handles the debug option --memdump. dumps the contents of linear memories.
def DumpLinearMems(linear_memories, threshold):
    count = int()
    strrep = []
    linmem_cnt = int()
    for lin_mem in linear_memories:
        print('-----------------------------------------')
        print(Colors.blue + Colors.BOLD + 'Linear Memory '+ repr(linmem_cnt)+ ' :' + Colors.ENDC)
        for byte in lin_mem:
            if count >= threshold:
                break
            if count%16 == 0:
                for ch in strrep:
                    # @DEVI-line feed messes the pretty format up
                    if ord(ch) != 10:
                        print(Colors.green + ' ' + ch + Colors.ENDC, end = '')
                    else:
                        pass
                print()
                strrep = []
                print(Colors.cyan + hex(count), ':\t' + Colors.ENDC, end='')
                strrep.append(str(chr(byte)))
                print(Colors.blue + format(byte, '02x') + ' ' + Colors.ENDC, end='')
            else:
                strrep += str(chr(byte))
                print(Colors.blue + format(byte, '02x') + ' ' + Colors.ENDC, end='')
            count += 1
        count = 0
    print()


# handles the debug options --idxspc. dumps the index spaces.
def DumpIndexSpaces(machinestate):
    print('-----------------------------------------')
    print(Colors.green + 'Function Index Space: ' + Colors.ENDC)
    for iter in machinestate.Index_Space_Function:
        print(Colors.blue + repr(iter) + Colors.ENDC)

    print('-----------------------------------------')
    print(Colors.green + 'Globa Index Space: ' + Colors.ENDC)
    for iter in machinestate.Index_Space_Global:
        print(Colors.blue + repr(iter) + Colors.ENDC)

    print('-----------------------------------------')
    print(Colors.green + 'Linear Memory Index Space: ' + Colors.ENDC)
    for iter in machinestate.Index_Space_Linear:
        print(Colors.blue + repr(iter) + Colors.ENDC)

    print('-----------------------------------------')
    print(Colors.green + 'Table Index Space: ' + Colors.ENDC)
    for iter in machinestate.Index_Space_Table:
        print(Colors.blue + repr(iter) + Colors.ENDC)
    print('-----------------------------------------')


# WIP-the Truebit Machine class
class TBMachine():
    def __init__(self):
        # bytearray of size PAGE_SIZE
        self.Linear_Memory = []
        self.Stack_Label = list()
        self.Stack_Label_Height = int()
        self.Stack_Control_Flow = list()
        self.Stack_Call = list()
        self.Stack_Value = list()
        self.Stack_Omni = list()
        self.Vector_Globals = list()
        self.Index_Space_Function = list()
        self.Index_Space_Global = list()
        self.Index_Space_Linear = list()
        self.Index_Space_Table = list()
        self.Index_Space_Locals = list()
        self.Index_Space_Label = list()


# handles the initialization of the WASM machine
class TBInit():
    def __init__(self, module, machinestate):
        self.module = module
        self.machinestate = machinestate

    # a convenience function that runs the methods of the class. all methods
    # can be called separately manually as well.
    def run(self):
        self.InitFuncIndexSpace()
        self.InitGlobalIndexSpace()
        self.InitLinearMemoryIndexSpace()
        self.InitTableIndexSpace()
        self.InitializeLinearMemory()

    def InitFuncIndexSpace(self):
        if self.module.import_section is not None:
            for iter in self.module.import_section.import_entry:
                if iter.kind == 0:
                    name = str()
                    for i in iter.field_str:
                        name += str(chr(i))
                    self.machinestate.Index_Space_Function.append(name)

        if self.module.function_section is not None:
            for iter in self.module.function_section.type_section_index:
                self.machinestate.Index_Space_Function.append(iter)

    def InitGlobalIndexSpace(self):
        if self.module.import_section is not None:
            for iter in self.module.import_section.import_entry:
                if iter.kind == 3:
                    name = str()
                    for i in iter.field_str:
                        name += str(chr(i))
                    self.machinestate.Index_Space_Global.append(name)

        if self.module.global_section is not None:
            for iter in self.module.global_section.global_variables:
                self.machinestate.Index_Space_Global.append(iter.init_expr)

    def InitLinearMemoryIndexSpace(self):
        if self.module.import_section is not None:
            for iter in self.module.import_section.import_entry:
                if iter.kind == 2:
                    name = str()
                    for i in iter.field_str:
                        name += str(chr(i))
                    self.machinestate.Index_Space_Linear.append(name)

        if self.module.memory_section is not None:
            for iter in self.module.memory_section.memory_types:
                self.machinestate.Index_Space_Linear.append(iter.initial)

    def InitTableIndexSpace(self):
        if self.module.import_section is not None:
            for iter in self.module.import_section.import_entry:
                if iter.kind == 1:
                    name = str()
                    for i in iter.field_str:
                        name += str(chr(i))
                    self.machinestate.Index_Space_Table.append(name)

        if self.module.table_section is not None:
            for iter in self.module.table_section.table_types:
                self.machinestate.Index_Space_Table.append(iter.element_type)

    def InitializeLinearMemory(self):
        # @DEVI-we could try to pack the data in the linear memory ourselve to
        # decrease the machinestate size
        if self.module.memory_section is None:
            rsz_limits = Resizable_Limits()
            self.module.memory_section = Memory_Section()
            self.module.memory_section.memory_types = [rsz_limits]
            self.module.memory_section.count = 1
        for iter in self.module.memory_section.memory_types:
            self.machinestate.Linear_Memory.append(bytearray(
                WASM_OP_Code.PAGE_SIZE))
        if self.module.data_section is not None:
            for iter in self.module.data_section.data_segments:
                count = int()
                for byte in iter.data:
                    self.machinestate.Linear_Memory[iter.index][init_interpret(iter.offset) + count] = byte
                    count += 1

    # returns the machinestate
    def getInits(self):
        return(self.machinestate)


# WIP-holds the run-rime data structures for a wasm machine
class RTE():
    def __init__(self):
        Stack_Control_Flow = list()
        Stack_Value = list()
        Vector_Locals = list()
        Current_Position = int()
        Local_Stacks = list()

    def genFuncLocalStack(func_body):
        pass


# palceholder for the class that holds the validation functions
class ModuleValidation():
    def __init__(self, module):
        self.module = module

    def TypeSection(self):
        pass

    def ImportSection(self):
        pass

    def FunctionSection(self):
        pass

    def TableSection(self):
        pass

    def MemorySection(self):
        pass

    def GlobalSection(self):
        pass

    def ExportSection(self):
        pass

    def StartSection(self):
        pass

    def ElementSection(self):
        pass

    def CodeSection(self):
        pass

    def DataSection(self):
        pass

    def TBCustom(self):
        pass

    def ValidateAll(self):
        self.TypeSection()
        self.ImportSection()
        self.FunctionSection()
        self.TableSection()
        self.MemorySection()
        self.GlobalSection()
        self.ExportSection()
        self.StartSection()
        self.ElementSection()
        self.CodeSection()
        self.DataSection()
        self.TBCustom()

        return(True)


# a convinience class that handles the initialization of the wasm machine and
# interpretation of the code.
class VM():
    def __init__(self, modules):
        self.modules = modules
        self.machinestate = TBMachine()
        # @DEVI-FIXME- the first implementation is single-module only
        self.init = TBInit(self.modules[0], self.machinestate)
        self.init.run()
        self.machinestate = self.init.getInits()
        self.start_function = Func_Body()
        self.ins_cache = WASM_Ins()
        self.executewasm = Execute(self.machinestate)
        self.totGas = int()
        self.metric = Metric(modules[0].code_section)
        self.parseflags = None

    def setFlags(self, parseflags):
        self.parseflags = parseflags

    def getState(self):
        return(self.machinestate)

    def initLocalIndexSpace(self, local_count):
        for i in range(0, local_count):
            self.machinestate.Index_Space_Locals.append(0)

    def getStartFunctionIndex(self):
        if self.modules[0].start_section is None:
            if self.parseflags.entry is None:
                raise Exception(Colors.red + "module does not have a start section. no function index was provided with the --entry option.quitting..." + Colors.ENDC)
            else:
                start_index = int(self.parseflags.entry)
        else:
            print(Colors.green + "found start section: " + Colors.ENDC, end = '')
            start_index = self.modules[0].start_section.function_section_index

        print(Colors.blue + Colors.BOLD + "running function at index " + repr(start_index) + Colors.ENDC)
        if (start_index > len(self.modules[0].code_section.func_bodies) - 1):
            raise Exception(Colors.red + "invalid function index: the function index does not exist." + Colors.ENDC)
        return(start_index)

    def getStartFunctionBody(self):
        start_index = self.getStartFunctionIndex()
        if isinstance(start_index, int):
            self.start_function = self.modules[0].code_section.func_bodies[start_index]
        elif isinstance(start_index, str):
            # we have to import the function from another module/library. we
            # assume sys calls are not present.:w
            pass
        else:
            raise Exception(Colors.red + "invalid entry for start function index" + Colors.ENDC)

    def execute(self):
        print(Colors.blue + Colors.BOLD + 'running module with code: ' + Colors.ENDC)
        for ins in self.start_function.code:
            print(Colors.purple + repr(ins.opcode) + ' ' + repr(ins.operands) + Colors.ENDC)
        for ins in self.start_function.code:
            self.executewasm.getInstruction(ins.opcodeint, ins.operands)
            self.executewasm.callExecuteMethod()
            self.getState()

    # pre-execution hook
    def startHook(self):
        if self.parseflags.metric:
            for mem in self.modules[0].memory_section.memory_types:
                self.executewasm.chargeGasMem(mem.initial)

            self.metric.mccabe()
            print(Colors.red + "mccabe: " + repr(self.metric.getMcCabe()) + Colors.ENDC)
            print(Colors.red + "soc: " + repr(self.metric.getSOC()) + Colors.ENDC)

    # post-execution hook
    def endHook(self):
        if self.parseflags.gas:
            self.totGas = self.executewasm.getOPGas()
            print(Colors.red + "total gas cost: " + repr(self.totGas) + Colors.ENDC)
        if self.machinestate.Stack_Omni:
            print(Colors.green + "stack top: " + repr(self.machinestate.Stack_Omni.pop()) + Colors.ENDC)

    # a convinience method
    def run(self):
        self.startHook()
        self.getStartFunctionBody()
        self.initLocalIndexSpace(self.start_function.local_count)
        self.execute()
        self.endHook()


# a wrapper class for VM. it timeouts instructions that take too long to
# execute.
class Judicator():
    def __int__(self, op_time_table, module):
        self.op_time_table = op_time_table
        self.vm = VM(modules)
        self.vm.getStartFunctionBody()

    def overseer():
        # @DEVI- forking introduces a new source of non-determinism
        pid = os.fork()
        # child process
        if pid == 0:
            sys.stdout = open('./jstdout', 'w')
            sys.stderr = open('./jstderr', 'w')
            self.vm.execute()
            sys.exit()
        # parent process
        if pid > 0:
            cpid, status = os.waitpid(pid, 0)
            if status == 0:
                print('overseer child exited successfully.')
            else:
                print('overseer child exited with non-zero.')
        # pid < 0
        else:
            raise Exception(Colors.red + 'could not fork judicator overseer.' + Colors.ENDC)

    def setup(self):
        signal.signal(signal.SIGALRM, self.to_sighandler)

    def set_alarm(t):
        signal.alaram(t)

    def to_sighandler(signum, frame):
        print(Colors.red + "execution time out..." + Colors.ENDC)
        raise Exception(Colors.red + "execution time out" + Colors.ENDC)

    def run(self):
        self.setup()
        self.set_alaram(10)
        self.overseer()
