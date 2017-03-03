
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
#include "mutatorserver.h"
/*standard headers*/
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
/**********************************************************************************************************************/
int main (int argc, char *argv[])
{
  int socket_desc, client_sock, c, read_size;
  struct sockaddr_in server, client;

  char client_message[2000];

  /*create socket*/
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_desc < 0)
  {
    printf("could not create socket.");
  }

  puts ("socket created.");

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(8888);

  /*Bind*/
  if (bind(socket_desc, (struct sockaddr*)&server, sizeof(server)) < 0)
  {
    perror("bind failed.error.");
    return 1;
  }

  puts("bind done.");

  /*Listen*/
  listen(socket_desc, 3);

  /*Accept incoming connection*/
  puts("Waiting for incoming connections...");
  c = sizeof(struct sockaddr_in);

  /*accept incoming connection from client*/
  client_sock = accept(socket_desc, (struct sockaddr*)&client, (socklen_t*)&c);

  if (client_sock < 0)
  {
    perror("could not accept incoming client.");
    return 1;
  }

  puts("connection accpeted.");

  /*recieve a message from client*/
  while((read_size = recv(client_sock, client_message, 2000, 0)) > 0)
  {
    /*send the message back to the client*/
    write(client_sock, client_message, strlen(client_message));
  }

  if (read_size  == 0)
  {
    puts("client disconnected");
    fflush(stdout);
  }
  else if (read_size == -1)
  {
    perror("recv failed.");
  }
  else
  {
    /*intentionally left blank*/
  }

  return 0;
}
/**********************************************************************************************************************/
/*last line intentionally left blank*/

