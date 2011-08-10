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

#include <pthread.h>

#include <EXTERN.h>
#include <perl.h>

#include <CoreServices/CoreServices.h>

#include "otoperld.h"
#include "codeserver.h";
#include "audiounit.h";

// ------------------------------------------------------ private functions
void perl_audio_callback(AudioBuffer *outbuf, UInt32 frames, UInt32 channels);
char *perl_code_liveeval(char *code);

// ------------------------------------------------ otoperld implimentation
codeserver *cs;

pthread_mutex_t mutex_for_perl;
pthread_cond_t cond_for_perl;

PerlInterpreter *my_perl;
bool int_perl_runtime_error;

void otoperld_start(int port, int perlargc, char **perlargv, char **env) {
	printf("otoperld - OtoPerl sound server - start with %s.\n", perlargv[1]);
	printf("***WARNING*** otoperld port is %d. You must not expose it to network. Blocking the port with firewall is strongly recommended.\n", port);

	pthread_mutex_init( &mutex_for_perl , NULL );
	pthread_cond_init( &cond_for_perl, NULL );

	PERL_SYS_INIT3(&perlargc, &perlargv, &env);
	my_perl = perl_alloc();
	perl_construct(my_perl);
	perl_parse(my_perl, NULL, perlargc, perlargv, NULL);
	PL_exit_flags |= PERL_EXIT_DESTRUCT_END;

	dSP;
	PUSHMARK(SP);
	call_pv("perl_render_init", G_DISCARD|G_NOARGS);

	int_perl_runtime_error = false;
	audiounit_start(perl_audio_callback);

	cs = codeserver_init(port, perl_code_liveeval);
	codeserver_start(cs);

	if (SIG_ERR == signal(SIGINT, otoperld_stop)) {
		printf("failed to set signal handler.n");
		otoperld_stop(0);
	}

	while(1){
		if (!cs->running) otoperld_stop(0);
		CFRunLoopRunInMode(kCFRunLoopDefaultMode, 2, false);
	}
}

void otoperld_stop ( int sig ) {
	codeserver_stop(cs);

	audiounit_stop();

	perl_destruct(my_perl);
	perl_free(my_perl);
	PERL_SYS_TERM();

	pthread_mutex_destroy( &mutex_for_perl );
	pthread_cond_destroy( &cond_for_perl );

	printf("otoperld - OtoPerl sound server - stop.\n");

	exit(0);
}

void perl_audio_callback(AudioBuffer *outbuf, UInt32 frames, UInt32 channels) {
	pthread_mutex_lock( &mutex_for_perl );
	
	dSP;
	ENTER;
	SAVETMPS;
	
	PUSHMARK(SP);
	XPUSHs(sv_2mortal(newSViv(frames)));
	XPUSHs(sv_2mortal(newSViv(channels)));
	PUTBACK;
	
	int count = call_pv("perl_render", G_ARRAY|G_EVAL);
	
	SPAGAIN;
	if (SvTRUE(ERRSV)) {
		if ( ! int_perl_runtime_error ) {
			int_perl_runtime_error = true;
			STRLEN n_a;
			printf ("perl runtime error: %s", SvPV(ERRSV, n_a));
		}
	} else {
		int_perl_runtime_error = false;
		if (count != frames * channels) croak("Big trouble\n");
		UInt32 channel, frame;
		for (channel = 0; channel < channels; channel++)
			for (frame = 0; frame < frames; frame++)
				((Float32 *)( outbuf[channel].mData ))[frame] = POPn;
	}
	PUTBACK;
	
	FREETMPS;
	LEAVE;

	pthread_mutex_unlock( &mutex_for_perl );
	pthread_cond_signal( &cond_for_perl );
}

char *perl_code_liveeval(char *code) {
	char * rettext = NULL;

	pthread_mutex_lock( &mutex_for_perl );
	pthread_cond_wait( &cond_for_perl, &mutex_for_perl );

	eval_pv(code, FALSE);

	if (SvTRUE(ERRSV)){
		STRLEN len;
		char *err = SvPV(ERRSV, len);
		rettext = (char *)malloc(len+1);
		strcpy(rettext, err);
	}

	pthread_mutex_unlock( &mutex_for_perl );

	return rettext;
}

