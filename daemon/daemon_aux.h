
/***************************************************Project Mutator****************************************************/
/*first line intentionally left blank.*/
/*Copyright (C) 2017 Farzad Sadeghi
This header is for the server called by the daemon.

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
#ifndef __MUTATOR_AUX_SERVER_H
#define __MUTATOR_AUX_SERVER_H
/**********************************************************************************************************************/
/*headers*/
#include <stdio.h>
/**********************************************************************************************************************/
typedef int bool;
#define true 1
#define false 0
/**********************************************************************************************************************/
/*function prototypes*/
bool cleanser(char cleansee[]);

int mutator_server(FILE* log_file);

#endif
/*last line intentionally left blank*/

