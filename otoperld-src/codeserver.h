// OtoPerl - live sound programming environment with Perl.
// OtoPerl::otoperld - sound processing server for OtoPerl.
// Otoperl::otoperld::codeserver - mini http server for otoperld.
/*
    OtoPerl - live sound programming environment with Perl.
    Copyright (C) 2011- Haruka Kataoka

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdbool.h>
#include <pthread.h>

typedef struct {
	int port;
	bool running;
	pthread_t thread;
	char *(*callback)(char *code);
} codeserver;

codeserver *codeserver_init(int port, char *(*callback)(char *code));
void codeserver_start(codeserver *self);
void codeserver_stop(codeserver *self);

