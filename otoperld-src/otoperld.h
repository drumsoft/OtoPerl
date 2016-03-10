// OtoPerl - live sound programming environment with Perl.
// OtoPerl::otoperld - sound processing server for OtoPerl.
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

#define PORTFILENAME ".otoperld_port"

typedef struct {
	int port;
	bool findfreeport;
	char *allow_pattern;
	int channel;
	int sample_rate;
	bool verbose;
	char *output;
	bool enable_input;
} otoperld_options;

#define OTOPERLD_OPTIONS_DEFAULTS {\
	14609,\
	false,\
	"127.0.0.1",\
	2,\
	48000,\
	false,\
	NULL,\
	false\
}

void otoperld_start(otoperld_options *options, int perlargc, char **perlargv, char **env);

