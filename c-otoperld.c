// c-otoperld

#include "c-otoperld.h"

pthread_mutex_t mutex_for_perl = PTHREAD_MUTEX_INITIALIZER;

// THESE values can be read from your data source
// they're used to tell the DefaultOutputUnit what you're giving it
const Float64			sSampleRate = 48000;
const int				sNumChannels = 2;
const int				sWhichFormat = 32;

const double			sAmplitude = 0.8;
const double			sToneFrequency = 880.0;

const UInt32 theFormatID = kAudioFormatLinearPCM;
// these are set based on which format is chosen
const UInt32 theFormatFlags = kAudioFormatFlagsNativeFloatPacked | kAudioFormatFlagIsNonInterleaved;
const UInt32 theBytesInAPacket = 4;
const UInt32 theBitsPerChannel = 32;
const UInt32 theBytesPerFrame = 4;
// these are the same regardless of format
const UInt32 theFramesPerPacket = 1; // this shouldn't change'

AudioUnit	gOutputUnit;

static PerlInterpreter *my_perl;
//void	*perl_context;



OSStatus	MyRenderer_Perl(void 				*inRefCon, 
				AudioUnitRenderActionFlags 	*ioActionFlags, 
				const AudioTimeStamp 		*inTimeStamp, 
				UInt32 						inBusNumber, 
				UInt32 						inNumberFrames, 
				AudioBufferList 			*ioData) {
	UInt32 frame;
	UInt32 channel;
	UInt32 inChannel = ioData->mNumberBuffers;
	int count;
	
	pthread_mutex_lock( &mutex_for_perl );
	
	dSP;
	ENTER;
	SAVETMPS;
	
	PUSHMARK(SP);
	XPUSHs(sv_2mortal(newSViv(inNumberFrames)));
	XPUSHs(sv_2mortal(newSViv(inChannel)));
	PUTBACK;
	
	count = call_pv("perl_render", G_ARRAY);
	
	if (count != inNumberFrames * inChannel) croak("Big trouble\n");
	SPAGAIN;
	for (channel = 0; channel < inChannel; channel++)
		for (frame = 0; frame < inNumberFrames; frame++)
			((Float32 *)( ioData->mBuffers[channel].mData ))[frame] = POPn;
	PUTBACK;
	
	FREETMPS;
	LEAVE;

	pthread_mutex_unlock( &mutex_for_perl );

	return noErr;
}

// ---------------------------
void	CreateDefaultAU() {
	OSStatus err = noErr;

	// Open the default output unit
	ComponentDescription desc;
	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_DefaultOutput;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
	
	Component comp = FindNextComponent(NULL, &desc);
	if (comp == NULL) { printf ("FindNextComponent\n"); return; }
	
	err = OpenAComponent(comp, &gOutputUnit);
	if (comp == NULL) { printf ("OpenAComponent=%ld\n", (long int)err); return; }

	// Set up a callback function to generate output to the output unit
    AURenderCallbackStruct input;
	input.inputProc = MyRenderer_Perl;
	input.inputProcRefCon = NULL;

	err = AudioUnitSetProperty (gOutputUnit, 
								kAudioUnitProperty_SetRenderCallback, 
								kAudioUnitScope_Input,
								0, 
								&input, 
								sizeof(input));
	if (err) { printf ("AudioUnitSetProperty-CB=%ld\n", (long int)err); return; }
    
	AudioStreamBasicDescription streamFormat;
	streamFormat.mSampleRate = sSampleRate;		//	the sample rate of the audio stream
	streamFormat.mFormatID = theFormatID;			//	the specific encoding type of audio stream
	streamFormat.mFormatFlags = theFormatFlags;		//	flags specific to each format
	streamFormat.mBytesPerPacket = theBytesInAPacket;	
	streamFormat.mFramesPerPacket = theFramesPerPacket;	
	streamFormat.mBytesPerFrame = theBytesPerFrame;		
	streamFormat.mChannelsPerFrame = sNumChannels;	
	streamFormat.mBitsPerChannel = theBitsPerChannel;	
	
	err = AudioUnitSetProperty (gOutputUnit,
								kAudioUnitProperty_StreamFormat,
								kAudioUnitScope_Input,
								0,
								&streamFormat,
								sizeof(AudioStreamBasicDescription));
	if (err) { printf ("AudioUnitSetProperty-SF=%4.4s, %ld\n", (char*)&err, (long int)err); return; }
	
    // Initialize unit
	err = AudioUnitInitialize(gOutputUnit);
	if (err) { printf ("AudioUnitInitialize=%ld\n", (long int)err); return; }
    
	/*	Float64 outSampleRate;
	 UInt32 size = sizeof(Float64);
	 err = AudioUnitGetProperty (gOutputUnit,
	 kAudioUnitProperty_SampleRate,
	 kAudioUnitScope_Output,
	 0,
	 &outSampleRate,
	 &size);
	 if (err) { printf ("AudioUnitSetProperty-GF=%4.4s, %ld\n", (char*)&err, (long int)err); return; }
	 */
	
	// Start the rendering
	// The DefaultOutputUnit will do any format conversions to the format of the default device
	err = AudioOutputUnitStart (gOutputUnit);
	if (err) { printf ("AudioOutputUnitStart=%ld\n", (long int)err); return; }	
}

void CloseDefaultAU () {
	OSStatus err = noErr;

	// REALLY after you're finished playing STOP THE AUDIO OUTPUT UNIT!!!!!!	'
	// but we never get here because we're running until the process is nuked...	'
	verify_noerr (AudioOutputUnitStop (gOutputUnit));
	
	err = AudioUnitUninitialize (gOutputUnit);
	if (err) { printf ("AudioUnitUninitialize=%ld\n", (long int)err); return; }

	CloseComponent (gOutputUnit);
}

// ---------------------------
void sound_start() {
	printf("start\n");
//	perl_context = PERL_GET_CONTEXT;
	CreateDefaultAU();
}

void sound_stop() {
	printf("stop\n");
	CloseDefaultAU();
}

// ---------------------------
int main(int argc, char **argv, char **env) {

	pthread_mutex_init( &mutex_for_perl , NULL );

	PERL_SYS_INIT3(&argc,&argv,&env);
	my_perl = perl_alloc();
	perl_construct(my_perl);

	int perl_argc = 2;
	char *perl_argv[] = { "", "c-otoperl-start.pl" };
	perl_parse(my_perl, NULL, perl_argc, perl_argv, NULL);
	PL_exit_flags |= PERL_EXIT_DESTRUCT_END;

	dSP;
	PUSHMARK(SP);
	call_pv("perl_render_init", G_DISCARD|G_NOARGS);

	sound_start();

	if (SIG_ERR == signal(SIGINT, terminate)) {
		printf("failed to set signal handler.n");
		terminate(0);
	}

	//             0123456789A
	char code[] = "sub pan{0.5}";
	srand((unsigned) time(NULL));
	while(1){
		CFRunLoopRunInMode(kCFRunLoopDefaultMode, 1, false);
		code[10] = '0' + ( rand()%9 + 1 );
		pthread_mutex_lock( &mutex_for_perl );
		eval_pv(code, TRUE);
		pthread_mutex_unlock( &mutex_for_perl );
	}
	return 0;
}

void terminate ( int sig ) {
	sound_stop();

	perl_destruct(my_perl);
	perl_free(my_perl);
	PERL_SYS_TERM();

	pthread_mutex_destroy( &mutex_for_perl );
	exit(0);
}


