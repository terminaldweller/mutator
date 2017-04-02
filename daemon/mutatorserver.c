
/** @file mutatorserver.c*/
/**
 * @brief The test file for the server. Runs the server as a stand-alone.
 */

/** @warning Deprecated*/
/***************************************************Project Mutator****************************************************/
/*first line intentionally left blank.*/
/*Copyright (C) 2017 Farzad Sadeghi
This source file contains mutator's server as a standalone, mainly for testing purposes.

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
/*macros*/
#define __DBG
#if 0
#undef __DBG
#endif
/**********************************************************************************************************************/
/*inclusion directive*/
#include "mutatorserver.h"
#include "daemon_aux.h"
/*standard headers*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
/**********************************************************************************************************************/
int main (int argc, char *argv[])
{
  int socket_desc, client_sock, socketlength, read_size;
  struct sockaddr_in server, client;

  char client_message[2000];
  FILE* clientistream;
  char runresponse[4000];
  char NOOUT[]="command did not return any output. could be an error or not.";

  /*create socket*/
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_desc < 0)
  {
    printf("could not create socket.");
    exit(EXIT_FAILURE);
  }
  puts ("socket created.");

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(8888);
  memset(server.sin_zero, 0, 8);

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
  socketlength = sizeof(struct sockaddr_in);

  /*accept incoming connection from client*/
  client_sock = accept(socket_desc, (struct sockaddr*)&client, (socklen_t*)&socketlength);

  if (client_sock < 0)
  {
    perror("could not accept incoming client.");
    return 1;
  }
  puts("connection accpeted.");

  /*recieve a message from client*/
  while((read_size = recv(client_sock, client_message, 2000, 0)) > 0)
  {
    fflush(stdin);

    puts("got command from client.");

    if (cleanser(client_message))
    {
      /*open pipe, run command*/
      clientistream = popen(client_message, "r");
    }
    else
    {
      puts("what are you trying to do exactly?");
      continue;
    }

    if (clientistream == NULL)
    {
      perror("client command did not run successfully.");
    }
    puts ("task completed.");

    for (int i = 0; i < 2000; ++i)
    {
      client_message[i] = 0;
    }

    if (fgets(runresponse, sizeof(runresponse), clientistream) == NULL)
    {
      /*say there was nothing on stdout to send.*/
      write(client_sock, NOOUT, strlen(NOOUT));
    }

    rewind(clientistream);

    while (fgets(runresponse, sizeof(runresponse), clientistream) != NULL)
    {
#if defined(__DBG)
      puts("command stdout:");
      puts(runresponse);
#endif
      write(client_sock, runresponse, strlen(runresponse));
    }

    puts("response sent to client.");

    fflush(stdout);
    /*close pipe*/
    pclose(clientistream);

#if defined(__DBG)
    puts("checkpoint 1");
#endif
  }

#if defined(__DBG)
    puts("checkpoint 10");
#endif

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

