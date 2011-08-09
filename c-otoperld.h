#include <CoreServices/CoreServices.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/CoreAudio.h>

#include <EXTERN.h>
#include <perl.h>

OSStatus	MyRenderer_Perl(void 				*inRefCon, 
				AudioUnitRenderActionFlags 	*ioActionFlags, 
				const AudioTimeStamp 		*inTimeStamp, 
				UInt32 						inBusNumber, 
				UInt32 						inNumberFrames, 
				AudioBufferList 			*ioData);

void CreateDefaultAU();
void CloseDefaultAU();

void sound_start();
void sound_stop();

int main(int argc, char **argv, char **env);
void terminate ( int sig );

