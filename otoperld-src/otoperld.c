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
#include "codeserver.h"
#include "audiounit.h"
#include "aiffrecorder.h"

// ------------------------------------------------------ private functions
void perl_audio_callback(AudioBuffer *outbuf, UInt32 frames, UInt32 channels);
char *perl_code_liveeval(char *code);

EXTERN_C void xs_init (pTHX);

// ------------------------------------------------ otoperld implimentation
codeserver *cs;
AiffRecorder *ar;
float *recordBuffer;

pthread_mutex_t mutex_for_perl;
pthread_cond_t cond_for_perl;

PerlInterpreter *my_perl;
bool int_perl_runtime_error;

void otoperld_start(otoperld_options *options, int perlargc, char **perlargv, char **env) {
	printf("otoperld - OtoPerl sound server - start with %s.\n", perlargv[1]);
	printf("otoperld port: %d, allowed clients: %s\n", options->port, options->allow_pattern);
	printf("*WARNING* don't expose otoperl port to network.\n");

	if (options->output) {
		printf("recording: %s\n", options->output);
		recordBuffer = (float *)malloc(options->channel);
		ar = AiffRecorder_create(options->channel, 32, options->sample_rate);
		AiffRecorder_open(ar, options->output);
	}else{
		ar = NULL;
	}

	pthread_mutex_init( &mutex_for_perl , NULL );
	pthread_cond_init( &cond_for_perl, NULL );

	PERL_SYS_INIT3(&perlargc, &perlargv, &env);
	my_perl = perl_alloc();
	perl_construct(my_perl);
	perl_parse(my_perl, xs_init, perlargc, perlargv, NULL);
	PL_exit_flags |= PERL_EXIT_DESTRUCT_END;

	dSP; // ---------------- perl call perl_render_init
	ENTER;
	SAVETMPS;
	PUSHMARK(SP);
	XPUSHs(sv_2mortal(newSViv(options->channel)));
	XPUSHs(sv_2mortal(newSViv(options->sample_rate)));
	PUTBACK;
	call_pv("perl_render_init", G_DISCARD);
	FREETMPS;
	LEAVE;

	int_perl_runtime_error = false;
	audiounit_start(options->channel, options->sample_rate, perl_audio_callback);

	cs = codeserver_init(options->port, options->allow_pattern, options->verbose, perl_code_liveeval);
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

	if (ar) {
		AiffRecorder_close(ar);
		AiffRecorder_destroy(ar);
		free(recordBuffer);
	}

	perl_destruct(my_perl);
	perl_free(my_perl);
	PERL_SYS_TERM();

	pthread_mutex_destroy( &mutex_for_perl );
	pthread_cond_destroy( &cond_for_perl );

	printf("otoperld - stopped (signal %d)\n", sig);

	exit(0);
}

void perl_audio_callback(AudioBuffer *outbuf, UInt32 frames, UInt32 channels) {
	UInt32 channel, frame;
	pthread_mutex_lock( &mutex_for_perl );
	
	dSP;
	ENTER;
	SAVETMPS;
	
	PUSHMARK(SP);
	XPUSHs(sv_2mortal(newSViv(frames)));
	XPUSHs(sv_2mortal(newSViv(channels)));
	for (channel = 0; channel < channels; channel++) {
		for (frame = 0; frame < frames; frame++) {
			XPUSHs(sv_2mortal(newSVnv( ((Float32 *)( outbuf[channel].mData ))[frame] )));
		}
	}
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
		int i = 0;
		for (channel = channels - 1; ; channel--) {
			for (frame = frames - 1; ; frame--) {
				((Float32 *)( outbuf[channel].mData ))[frame] = POPn;
				if (++i >= count || frame == 0) break;
			}
			if (i >= count || channel == 0) break;
		}
		if (ar) {
			for (frame = 0; frame < frames; frame++) {
				for (channel = 0; channel < channels; channel++) {
					recordBuffer[channel] = ((Float32 *)( outbuf[channel].mData ))[frame];
				}
				AiffRecorder_write32bit(ar, (uint32_t *)recordBuffer, 1);
			}
		}
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

	dSP;
	ENTER;
	SAVETMPS;
	
	PUSHMARK(SP);
	XPUSHs(sv_2mortal(newSVpv(code, 0)));
	PUTBACK;

	call_pv("perl_eval", G_DISCARD|G_EVAL);

	if (SvTRUE(ERRSV)){
		STRLEN len;
		char *err = SvPV(ERRSV, len);
		rettext = (char *)malloc(len+1);
		strcpy(rettext, err);
	}

	FREETMPS;
	LEAVE;

	int_perl_runtime_error = false;
	pthread_mutex_unlock( &mutex_for_perl );

	return rettext;
}

