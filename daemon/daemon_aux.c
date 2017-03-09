
/***************************************************Project Mutator****************************************************/
/*first line intentionally left blank.*/
/*Copyright (C) 2017 Farzad Sadeghi
This source file contains mutator's server called by the daemon.

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
#if 1
#undef __DBG
#endif
/**********************************************************************************************************************/
/*inclusion directive*/
#include "daemon_aux.h"
/*standard headers*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
/**********************************************************************************************************************/
int mutator_server(FILE* log_file)
{
  int socket_desc, client_sock, socketlength, read_size;
  struct sockaddr_in server, client;

  char client_message[2000];
  FILE* clientistream;
  char runresponse[4000];
  char NOOUT[]="command did not return any output. could be an error or not.\n";

  /*create socket*/
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_desc < 0)
  {
    fprintf(log_file, "%s", "could not create socket.\n");
  }
  fprintf (log_file, "%s", "socket created.\n");

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(8888);

  /*Bind*/
  if (bind(socket_desc, (struct sockaddr*)&server, sizeof(server)) < 0)
  {
    perror("bind failed.error.\n");
    return 1;
  }

  fprintf(log_file, "%s", "bind done.\n");

  /*Listen*/
  listen(socket_desc, 3);

  /*Accept incoming connection*/
  fprintf(log_file, "%s", "Waiting for incoming connections...\n");
  socketlength = sizeof(struct sockaddr_in);

  /*accept incoming connection from client*/
  client_sock = accept(socket_desc, (struct sockaddr*)&client, (socklen_t*)&socketlength);

  if (client_sock < 0)
  {
    perror("could not accept incoming client.");
    return 1;
  }
  fprintf(log_file, "%s", "connection accpeted.\n");

  /*recieve a message from client*/
  while((read_size = recv(client_sock, client_message, 2000, 0)) > 0)
  {
    fflush(stdin);

    fprintf(log_file, "%s","got command from client.\n");

    /*open pipe, run command*/
    clientistream = popen(client_message, "r");

    if (clientistream == NULL)
    {
      perror("client command did not run successfully.");
    }
    fprintf(log_file, "%s", "task completed.\n");

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
      fscanf(log_file, "%s", "command stdout:");
      fscanf(log_file, "%s", runresponse);
#endif
      write(client_sock, runresponse, strlen(runresponse));
      fprintf(log_file, "%s", runresponse);
    }

    fprintf(log_file, "%s", "response sent to client.\n");

    fflush(stdout);
    /*close pipe*/
    pclose(clientistream);

#if defined(__DBG)
    fprintf(log_file, "%s", "checkpoint 1\n");
#endif
  }

#if defined(__DBG)
    fprintf(log_file, "%s", "checkpoint 10\n");
#endif

  if (read_size  == 0)
  {
    fprintf(log_file, "%s", "client disconnected\n");
    fflush(stdout);
    fprintf(log_file, "%s", "closing log file\n");
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

