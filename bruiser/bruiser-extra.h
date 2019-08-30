
/***************************************************Project Mutator****************************************************/
//-*-c++-*-
/*first line intentionally left blank.*/
/*the header for anything extra for bruiser that needs to get shared between different files.*/
/*Copyright (C) 2017 Farzad Sadeghi

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 3
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.*/
/**********************************************************************************************************************/
/*inclusion guard*/
#ifndef BRUISER_EXTRA_H
#define BRUISER_EXTRA_H
/**********************************************************************************************************************/
/*included modules*/
#include <vector>
#include <signal.h>
#include <string>
/**********************************************************************************************************************/
struct SigNames
{
  int Signal;
  char *SigName;
};

#if 0
std::vector<SigNames> SignalNames = 
{
  {SIGHUP, (char*)"SIGHUP"},
  {SIGINT, (char*)"SIGINT"},
  {SIGQUIT, (char*)"SIGQUIT"},
  {SIGILL, (char*)"SIGILL"},
  {SIGTRAP, (char*)"SIGTRAP"},
  {SIGABRT, (char*)"SIGABRT"},
  {SIGIOT, (char*)"SIGIOT"},
  {SIGBUS, (char*)"SIGBUS"},
  {SIGFPE, (char*)"SIGFPE"},
  {SIGKILL, (char*)"SIGKILL"},
  {SIGUSR1, (char*)"SIGUSR1"},
  {SIGSEGV, (char*)"SIGSEGV"},
  {SIGUSR2, (char*)"SIGUSR2"},
  {SIGPIPE, (char*)"SIGPIPE"},
  {SIGALRM, (char*)"SIGALRM"},
  {SIGTERM, (char*)"SIGTERM"},
  {SIGSTKFLT, (char*)"SIGSTKFLT"},
  {SIGCHLD, (char*)"SIGCHLD"},
  {SIGCONT, (char*)"SIGCONT"},
  {SIGSTOP, (char*)"SIGSTOP"},
  {SIGTSTP, (char*)"SIGTSTP"},
  {SIGTTIN, (char*)"SIGTTIN"},
  {SIGTTOU, (char*)"SIGTTOU"},
  {SIGURG, (char*)"SIGURG"},
  {SIGXCPU, (char*)"SIGXCPU"},
  {SIGXFSZ, (char*)"SIGXFSZ"},
  {SIGVTALRM, (char*)"SIGVTALRM"},
  {SIGPROF, (char*)"SIGPROF"},
  {SIGWINCH, (char*)"SIGWINCH"},
  {SIGIO, (char*)"SIGIO"}
};
#endif

std::vector<std::string> BRUISR_COMMANDS = 
{
  "help",
  "quit",
  "exit",
  "list",
  "list vars",
  "list funcs",
  "list structs",
  "list classes",
  "list unions",
  "list records",
  "history",
  "shell",
  "version",
  "clear",
  "hijack",
  "hijack main",
  "m0"
};

/**********************************************************************************************************************/
#endif
/*last line intentionally left balnk*/

