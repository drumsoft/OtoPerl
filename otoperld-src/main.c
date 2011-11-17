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
#include <string.h>
#include <stdio.h>
#include "otoperld.h"

#define OTOPERLD_DEFAULT_STARTCODE "otoperld-start.pl"

const char options_short[] = "p:vc:r:a:o:";
const struct option options_long[] = {
	{ "port"   , required_argument, NULL, 'p' },
	{ "verbose",       no_argument, NULL, 'v' },
	{ "channel", required_argument, NULL, 'c' },
	{ "rate"   , required_argument, NULL, 'r' },
	{ "allow"  , required_argument, NULL, 'a' },
	{ "output" , required_argument, NULL, 'o' }
};

char errortext[256];

// -------------------------------------------- private functions
long options_integer(const char *arg, long min, long max, const char *text);
void die(const char *text);


// -------------------------------------------- main
int main(int argc, char **argv, char **env) {
	otoperld_options options = OTOPERLD_OPTIONS_DEFAULTS;

	int perlargc;
	char **perlargv;

	int result, length;
	while( (result = getopt_long(argc, argv, options_short, options_long, NULL)) != -1 ){
		switch(result){
			case 'p':
				options.port
				 = options_integer(optarg, 0, 65535, "-p, --port");
				break;
			case 'v':
				options.verbose = true;
				break;
			case 'c':
				options.channel
				 = options_integer(optarg, 1, 128, "-c, --channel");
				break;
			case 'r':
				options.sample_rate
				 = options_integer(optarg, 1, 192000, "-r, --rate");
				break;
			case 'a':
				length = strlen(optarg);
				if ( length <= 0 || 39 < length)
					die("-a, --allow parameter is like '192.168.1.3' or '192.168.2.0/255.255.255.0'.");
				options.allow_pattern = (char *)malloc(length+1);
				if ( ! options.allow_pattern ) die( "malloc faild (options.allow)." );
				strcpy(options.allow_pattern, optarg);
				break;
			case 'o':
				length = strlen(optarg);
				options.output = (char *)malloc(length+1);
				if ( ! options.output ) die( "malloc faild (options.output)." );
				strcpy(options.output, optarg);
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

	otoperld_start(&options, perlargc, perlargv, env);
	return 0;
}


// ----------------------------------------------- functions
long options_integer(const char *arg, long min, long max, const char *text) {
	char *err;
	long number = strtol(arg, &err, 0);
	
	if (*err != '\0') {
		sprintf(errortext, "%s parameter must be a number.", text);
		die(errortext);
	}else if( number < min || max < number ) {
		sprintf(errortext, "%s parameter must be in %ld - %ld.", text, min, max);
		die(errortext);
	}
	return number;
}

void die(const char *text) {
	if (text) perror(text);
	exit(-1);
}

