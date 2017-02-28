
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
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
/**********************************************************************************************************************/
int main(void)
{
  /*getting a process ID*/
  pid_t pid;
  /*getting a session ID*/
  pid_t sid;

  /*fork off the parent process*/
  pid = fork();

  if (pid < 0)
  {
    exit(EXIT_FAILURE);
  }

  if (pid > 0)
  {
    exit(EXIT_SUCCESS);
  }

  umask(0);

  /*create a new session ID for the child process*/
  sid = setsid();
  if (sid < 0)
  {
    exit(EXIT_FAILURE);
  }

  /*change the current working directory*/
  if (chdir("/") < 0)
  {
    exit(EXIT_FAILURE);
  }

  /*close the standard file descriptors*/
  close(STDIN_FILENO) ;
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  /*deamon loop*/
  while(1)
  {
    sleep(30);
  }

}
/*last line intentionally left blank*/

