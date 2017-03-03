
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
/*inclusion directive*/
#include "mutatorclient.h"
/*standard header libraries*/
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
/**********************************************************************************************************************/
int main(int argc, char *argv[])
{
  int sock;
  struct sockaddr_in server;
  char message[1000];
  char server_reply[2000];

  /*create socket*/
  sock = socket(AF_INET, SOCK_STREAM, 0);

  if (sock == -1)
  {
    printf ("could not create socket");
  }

  puts("created socket");

  server.sin_addr.s_addr = inet_addr("127.0.0.1");
  server.sin_family = AF_INET;
  server.sin_port = htons(8888);

  /*connect to remote server*/
  if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0)
  {
    perror("connect failed.error.");
    return 1;
  }

  puts("connected.");

  /*keep communicating with the server.*/
  while(1)
  {
    printf("enter massage: ");
    scanf("%s", message);

    /*send some data*/
    if (send(sock, message, strlen(message), 0) < 0)
    {
      puts("send fialed.");
      return 1;
    }

    /*recieve a reply from the server*/
    if (recv(sock, server_reply, 2000, 0) < 0)
    {
      puts("recv failed.");
      break;
    }
  
    puts("server reply: ");
    puts(server_reply);

    if (strncmp(server_reply, "end_comm", strlen(server_reply)))
    {
      break;
    }
  }

  close(sock);
  return 0;
}
/**********************************************************************************************************************/
/*last line intentionally left blank*/

