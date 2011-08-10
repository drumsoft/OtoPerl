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

#include <stdlib.h>
#include <getopt.h>
#include "otoperld.h"

#define OTOPERLD_DEFAULT_PORT 14609
#define OTOPERLD_DEFAULT_STARTCODE "otoperld-start.pl"

int main(int argc, char **argv, char **env) {
	int port = OTOPERLD_DEFAULT_PORT;
	int perlargc;
	char **perlargv;

	int result;
	while( (result = getopt(argc, argv, "p:")) != -1 ){
		switch(result){
			case 'p':
				port = atoi(optarg);
				break;
		}
	}

	if (optind < argc) {
		perlargc = 1 + argc - optind;
		perlargv = (char **)malloc(perlargc * sizeof(char *));
		perlargv[0] = "";
		int index = 1;
		for (; optind < argc; optind++) {
			perlargv[index++] = argv[optind];
		}
	}else{
		perlargc = 2;
		perlargv = (char **)malloc(perlargc * sizeof(char *));
		perlargv[0] = "";
		perlargv[1] = OTOPERLD_DEFAULT_STARTCODE;
	}

	otoperld_start(port, perlargc, perlargv, env);
	return 0;
}


