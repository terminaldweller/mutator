
/***************************************************Project Mutator****************************************************/
/*first line intentionally left blank.*/
/*Copyright (C) 2017 Farzad Sadeghi
This is the header for the daemon.

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
#ifndef __MUTATOR_D_H
#define __MUTATOR_D_H
/**********************************************************************************************************************/
/*headers*/
#include <stdio.h>
/**********************************************************************************************************************/
/*function prototypes*/
void sigint_callback_handler(int signum);

void sigterm_callbacl_handler(int signum);

void time_n_date(FILE* infile);

#endif
/*last line intentionally left blank*/

