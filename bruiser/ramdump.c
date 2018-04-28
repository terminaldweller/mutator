
/***************************************************Project Mutator****************************************************/
/*first line intentionally left blank.*/
/*bruiser's ram dump module*/
/*Copyright (C) 2018 Farzad Sadeghi

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
/***********************************************************************************************************/
//#include "ramdump.h"
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>
/***********************************************************************************************************/
void dump_memory_region(FILE* pMemFile, uint64_t start_address, uint64_t length, FILE* out_file) {
  uint64_t address;
  int pageLength = 4096;
  fseeko(pMemFile, start_address, SEEK_SET);
  unsigned char page[pageLength];

  for (address=start_address; address < start_address + length; address += pageLength) {
    fread(&page, 1, pageLength, pMemFile);
    fwrite(&page, 1, pageLength, out_file);
  }
}

void dump_ram(unsigned int pid, FILE* out_file) {
  uint64_t ptraceResult = ptrace(PTRACE_ATTACH, pid, NULL, NULL);
  if (ptraceResult < 0) {
    printf("ramdump: unable to attach to the pid specified\n");
    return;
  }
  wait(NULL);

  char mapsFilename[1024];
  char proc_str[6];
  sprintf(proc_str, "%d", pid);
  sprintf(mapsFilename, "/proc/%s/maps", proc_str);
  FILE* pMapsFile = fopen(mapsFilename, "r");
  char memFilename[1024];
  sprintf(memFilename, "/proc/%s/mem", proc_str);
  FILE* pMemFile = fopen(memFilename, "r");
  char line[256];
  while (fgets(line, 256, pMapsFile) != NULL) {
    uint64_t start_address;
    uint64_t end_address;
    sscanf(line, "%08lx-%08lx\n", &start_address, &end_address);
    dump_memory_region(pMemFile, start_address, end_address - start_address, out_file);
  }

  fclose(pMapsFile);
  fclose(pMemFile);

  ptrace(PTRACE_CONT, pid, NULL, NULL);
  ptrace(PTRACE_DETACH, pid, NULL, NULL);
}

#pragma weak main
int main(int argc, char **argv) {
  if (argc != 2) {
    printf("what happened to the pid?\n");
    return 1;
  }
  FILE* out_file = fopen("/tmp/ramdump", "w");
  int pid = atoi(argv[1]);
  dump_ram(pid, out_file);
  fclose(out_file);
  return 0;
}
/***********************************************************************************************************/
/*last line is intentionally left blank*/

