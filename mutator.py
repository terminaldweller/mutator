#!/bin/python3

import argparse
import sys
from subprocess import call

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

class ArgParser(object):
    def __init__(self):
        self.inputs = []
        self.outputs = []
        self.opts = []
        self.copts = []
        self.command = []
        self.pretty = bool
        parser = argparse.ArgumentParser()
        parser.add_argument("-help", action='store_true', help="display the driver's help", default=False)
        parser.add_argument("-f", "--file", type=str, help="run an action file")
        parser.add_argument("-c", "--command", nargs="+", type=str, help="run a command")
        parser.add_argument("-v", "--version", action='store_true', help="display version info", default=False)
        parser.add_argument("-i", "--input", type=str, help="the input file(s)")
        parser.add_argument("-o", "--output", type=str, help="the output file(s)")
        parser.add_argument("-pp", "--print-pretty", action='store_true', help="print pretty simple text reports", default=False)
        parser.add_argument("-t", "--test", type=str, help="runs the tests on the built executables")
        parser.add_argument("-opts", "--options", type=str, help="pass clang options to the executables")
        parser.add_argument("-copts", "--custom-options", type=str, help="runs the tests on the built executables")
        self.args = parser.parse_args()

    def run(self):
        if self.args.file  is not None:
            self.parseActionFile(self.args.file)
            self.runActionFile()
        if self.args.command is not None:
            self.runCommand(self.args.command)

    def runTest(self):
        pass

    def parseActionFile(self, action_file_path):
        line_number = int()
        action_file = open(action_file_path)
        self.inputs = str
        self.outputs = str
        self.opts = str
        self.copts = str
        self.command = str
        self.logfile = str
        self.pretty = bool
        self.exename = str
        self.endaction = str
        self.action_name = str
        for line in action_file:
            line_number += 1
            pos = line.find("action_name:", 0, len(line))
            if pos == 0:
                self.action_name = line[pos+len("action_name:"):-1]
                print(Colors.green + "action name: " + Colors.cyan + self.action_name + Colors.ENDC)
            pos = line.find("executable_name:", 0, len(line))
            if pos == 0:
                self.exename = line[pos+len("executable_name:"):-1]
                print(Colors.green + "executable_name: " + Colors.cyan + self.exename + Colors.ENDC)
            pos = line.find("exec_opts:", 0, len(line))
            if pos == 0:
                self.copts = line[pos+len("exec_opts:"):-1]
                print(Colors.green + "exec options: " + Colors.cyan + self.copts + Colors.ENDC)
            pos = line.find("in_files:", 0, len(line))
            if pos == 0:
                self.inputs = line[pos+len("in_files:"):-1]
                print(Colors.green + "input files: " + Colors.cyan + self.inputs + Colors.ENDC)
            pos = line.find("libtooling_options:", 0, len(line))
            if pos == 0:
                self.opts = line[pos+len("libtooling_options:"):-1]
                print(Colors.green + "libtooling options: " + Colors.cyan + self.opts  + Colors.ENDC)
            pos = line.find("out_files:", 0, len(line))
            if pos == 0:
                self.outputs = line[pos+len("out_files:"):-1]
                print(Colors.green + "output files: " + Colors.cyan + self.outputs + Colors.ENDC)
            pos = line.find("log_files:", 0, len(line))
            if pos == 0:
                self.logfile = line[pos+len("log_files:"):-1]
                print(Colors.green + "log files: " + Colors.cyan + self.logfile + Colors.ENDC)
            pos = line.find("print_pretty:", 0, len(line))
            if pos == 0:
                if line[pos+len("print_pretty:"):-1] == "true": self.pretty = True
                elif line[pos+len("print_pretty:"):-1] == "false": self.pretty = False
                else: raise Exception(Colors.red + repr(line_number) + " : " + "invalid value for a boolean switch. should be true or false." + Colors.ENDC)
                print(Colors.green + "pretty print: " + Colors.cyan + repr(self.pretty) + Colors.ENDC)
            pos = line.find("end_action:", 0, len(line))
            if pos == 0:
                self.endaction = line[pos+len("end_action:"):-1]
                print(Colors.green + "end action: " + Colors.cyan + self.endaction + Colors.ENDC)

    def runActionFile(self):
        pass

    def printVersionInfo(self):
        pass

    def runCommand(self, command):
        if command[0] == "clean":
            call(["make", "clean"])
        elif command[0] == "make":
            call(["make", self.args.command[1]])
        elif command[0] == "install":
            call(["make", "install"])
        elif command[0] == "format":
            pass
        elif command[0] == "test":
            pass
        elif command[0] == "buildall" or command[0] == "makeall":
            call(["make"])
        elif command[0] == "m0":
            pass
        elif command[0] == "m1":
            pass
        elif command[0] == "m2":
            pass
        elif command[0] == "bruiser":
            pass
        elif command[0] == "obfuscator":
            pass
        elif command[0] == "safercpp":
            pass
        else:
            raise Exception(Colors.red + "unknown command. for a list of valid commands run with -h." + Colors.ENDC);


def main():
    argparser = ArgParser()
    argparser.run()

if __name__ == "__main__":
    main()
