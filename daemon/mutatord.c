
/***************************************************Project Mutator****************************************************/
/*first line intentionally left blank.*/
/*the source code for the static checks(Misra-C,...)*/
/*Copyright (C) 2017 Farzad Sadeghi
This source file contains mutator's daemon.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.*/
/**********************************************************************************************************************/
/*inclusion directives*/
#include "mutatord.h"
/*library headers*/
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>
/**********************************************************************************************************************/
void time_n_date(FILE* infile)
{
  time_t tnd = time(NULL);
  fprintf(infile, "%s", ctime(&tnd));
}

void signal_callback_handler(int signum)
{
  exit(signum);
}

int main(void)
{
  //signal(SIGINT, signal_callback_handler);

  /*getting a process ID*/
  pid_t pid;
  /*getting a session ID*/
  pid_t sid;

  FILE *mut_log;
  mut_log = fopen("mutatord-log", "w");
  
  /*printing out time n date*/
  fprintf(mut_log, "%s", "daemon started on ");
  time_n_date(mut_log);

  /*fork off the parent process*/
  pid = fork();

  if (pid < 0)
  {
    //fprintf(mut_log, "%s", "failed to get a pid.\n");
    exit(EXIT_FAILURE);
  }

  if (pid > 0)
  {
    fprintf(mut_log, "%s%d%s", "successfully got a pid:", pid, "\n");
    exit(EXIT_SUCCESS);
  }

  umask(0);
  fprintf(mut_log, "%s", "set umask to 0.\n");
  
  /*create a new session ID for the child process*/
  sid = setsid();
  if (sid < 0)
  {
    fprintf(mut_log, "%s", "failed to get an sid.\n");
    exit(EXIT_FAILURE);
  }
  else
  {
    fprintf(mut_log, "%s%d%s", "got an sid:", sid, "\n");
  }

  /*change the current working directory*/
  if ((chdir("/")) < 0)
  {
    fprintf(mut_log, "%s", "failed to change to root dir.\n");
    exit(EXIT_FAILURE);
  }
  else
  {
    fprintf(mut_log, "%s", "changed to root dir.\n");
  }

  /*close the standard file descriptors*/
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
  fprintf(mut_log, "%s", "closed standard file descriptors..\n");

  /*deamon loop*/
  while(1)
  {
    fprintf(mut_log, "%s", "mutatord is running fine.\n");
    sleep(1);
  }

  fclose(mut_log);
  exit(EXIT_SUCCESS);

}
/*last line intentionally left blank*/

