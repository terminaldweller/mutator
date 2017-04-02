
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

#define __DBG_2
#if 1
#undef __DBG_2
#endif

#define CLEAN_UP() \
      do{\
      fclose(log_file);\
      fclose(mutator_config);\
      close(client_sock);\
      close(socket_desc);\
      }\
      while(0)
/**********************************************************************************************************************/
/*inclusion directive*/
#include "daemon_aux.h"
/*standard headers*/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
/**********************************************************************************************************************/
bool cleanser(char cleansee[])
{
  bool nullterminated = false;
  bool cleansee_health = true;

  for (int i = 0; i < 2000; ++i)
  {
    if (cleansee[i] == '\0')
    {
      nullterminated = true;
      break;
    }

    if (cleansee[i] == '|')
    {
      cleansee_health = false;
    }

    if (cleansee[i] == ';')
    {
      cleansee_health = false;
    }
  }

  return (cleansee_health && nullterminated);
}

/**********************************************************************************************************************/
int mutator_server(FILE* log_file)
{
  int socket_desc, client_sock, socketlength, read_size;
  struct sockaddr_in server, client;

  char client_message[2000];
  FILE* clientistream;
  FILE* mutator_config;
  char runresponse[4000];
  const char NOOUT[]="command did not return any output. could be an error or not.\n";
  const char BADOUT[]="what are you exactly trying to do?";
  const char STD_OUT[]="stdout returned:\n";
  const char EMPTY_CONFIG[]="error: empty config file.\n";
  const char NFOUND_CONFIG[]="error: cant find config file in the default path.\n";
  const char SERVER_TERM[]="server terminated.\n";

  /*create socket*/
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_desc < 0)
  {
    fprintf(log_file, "%s", "could not create socket.\n");
    exit(EXIT_FAILURE);
  }
  fprintf (log_file, "%s", "socket created.\n");

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(8888);
  memset(server.sin_zero, 0, 8);

  /*Bind*/
  if (bind(socket_desc, (struct sockaddr*)&server, sizeof(server)) < 0)
  {
    close(socket_desc);
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
    close(socket_desc);
    return 1;
  }
  fprintf(log_file, "%s", "connection accpeted.\n");

  /*recieve a message from client*/
  while((read_size = recv(client_sock, client_message, 2000, 0)) > 0)
  {
    fflush(stdin);

    fprintf(log_file, "%s", "got command from client.\n");

    mutator_config = fopen("/home/bloodstalker/devi/hell2/daemon/mutator.config", "r");

    if (mutator_config == NULL)
    {
      write(client_sock, NFOUND_CONFIG, strlen(NFOUND_CONFIG));
      write(client_sock, SERVER_TERM, strlen(SERVER_TERM));
      fprintf(log_file, "%s", NFOUND_CONFIG);
      fprintf(log_file, "%s%d%s", "fopen returned: ", errno, "\n");
      fprintf(log_file, "%s", SERVER_TERM);

      fclose(log_file);
      close(client_sock);
      close(socket_desc);

      return errno;
    }

    char configline[100];
    const char delimiter[2]="=";
    char* token_var;
    const char mutator_home_var[]="MUTATOR_HOME";
    const char driver_name[] = "/mutator.sh ";
    char* full_command;
    char* temp;
    char* dummy;

    /*checking for an empty config-file. could also mean the config file was not found.*/
    if(fgets(configline,sizeof(configline), mutator_config) == NULL)
    {
      write(client_sock, EMPTY_CONFIG, strlen(EMPTY_CONFIG));
      write(client_sock, SERVER_TERM, strlen(SERVER_TERM));
      fprintf(log_file, "%s", EMPTY_CONFIG);
      fprintf(log_file, "%s", SERVER_TERM);
      CLEAN_UP();
      /*@DEVI-return SIGPIPE*/
      return 141;
    }

    rewind(mutator_config);

    while (fgets(configline,sizeof(configline), mutator_config) != NULL)
    {
      temp = strstr(configline, mutator_home_var);

      if (temp != NULL)
      {
        memmove(temp, configline + strlen(mutator_home_var) + 1, strlen(configline) - strlen(mutator_home_var) - 1);

        break;
      }
    }

    /*@DEVI-null-terminating temp*/
    temp[strlen(temp) - strlen(mutator_home_var) - 2] = '\0';
    /*@DEVI-checks whether the line-break char was also sent.if yes, then removes it.*/
    if (client_message[read_size - 1] == '\n')
    {
      client_message[read_size - 1] = '\0';
    }

    full_command = malloc(strlen(temp) + read_size + strlen(driver_name) + 1);

    strcpy(full_command,temp);
    strcat(full_command, driver_name);
    /*@DEVI-client_message is not null-terminated but strcat takes care of that.*/
    strcat(full_command, client_message);

#if defined(__DBG_2)
    fprintf(log_file, "%s%s%s", "temp is: ", temp, "\n");
    fprintf(log_file, "%s%s%s", "driver_name is: ",driver_name, "\n");
#endif
    fprintf(log_file, "%s%s%s", "full_command is: ", full_command, "\n");

    if (cleanser(client_message) == true)
    {
#ifndef __DBG
      clientistream = popen(full_command, "r");
#endif

#if defined(__DBG)
      /*open pipe, run command*/
      clientistream = popen(client_message, "r");
      //clientistream = popen(full_command, "r");
#endif
    }
    else
    {
      fprintf(log_file, "%s", "what are you trying to do exactly?");
      write(client_sock, BADOUT, strlen(BADOUT));
      free(full_command);
      fclose(mutator_config);
      continue;
    }

    fprintf(log_file, "%s", "freeing memory reserved for command.\n");
    free(full_command);
    fclose(mutator_config);

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
      fprintf(log_file, "%s", "command returned no stdout.\n");
      write(client_sock, NOOUT, strlen(NOOUT));
    }
    else
    {
      fprintf(log_file, "%s", "command returned stdout.\n");
      write(client_sock, STD_OUT, strlen(STD_OUT));
    }

    rewind(clientistream);

    while (fgets(runresponse, sizeof(runresponse), clientistream) != NULL)
    {
#if defined(__DBG)
      fprintf(log_file, "%s", "command stdout:\n");
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

  close(client_sock);
  close(socket_desc);
  return 0;
}
/**********************************************************************************************************************/
/*last line intentionally left blank*/

