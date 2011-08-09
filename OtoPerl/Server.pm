package OtoPerl::Server;

use strict;
use warnings;
use threads;
use threads::shared;

my $default_LDDLFLAGS;
BEGIN {
	use Config;
	$default_LDDLFLAGS = $Config{lddlflags};
}
use Inline C => Config =>
    LDDLFLAGS => $default_LDDLFLAGS
     . ' -framework CoreServices -framework CoreAudio -framework AudioUnit';
use Inline 'C' => <<'END_OF_C_CODE';





//------------------------------------------------------------------------
#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include <unistd.h>

#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/CoreAudio.h>
 

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
void	*perl_context;



OSStatus	MyRenderer(void 				*inRefCon, 
				AudioUnitRenderActionFlags 	*ioActionFlags, 
				const AudioTimeStamp 		*inTimeStamp, 
				UInt32 						inBusNumber, 
				UInt32 						inNumberFrames, 
				AudioBufferList 			*ioData) {
	UInt32 frame;
	UInt32 channel;
	UInt32 inChannel = ioData->mNumberBuffers;
	int count;
	
	PERL_SET_CONTEXT(perl_context);
	dSP;
	ENTER;
	SAVETMPS;
	
	PUSHMARK(SP);
	XPUSHs(sv_2mortal(newSViv(inNumberFrames)));
	XPUSHs(sv_2mortal(newSViv(inChannel)));
	PUTBACK;
	
	count = call_pv("OtoPerl::Server::_render_callback", G_ARRAY);
	
	if (count != inNumberFrames * inChannel) croak("Big trouble\n");
	SPAGAIN;
	for (channel = 0; channel < inChannel; channel++)
		for (frame = 0; frame < inNumberFrames; frame++)
			((Float32 *)( ioData->mBuffers[channel].mData ))[frame] = POPn;
	PUTBACK;
	
	FREETMPS;
	LEAVE;

	return noErr;
}

OSStatus	no_MyRenderer(void 				*inRefCon, 
				AudioUnitRenderActionFlags 	*ioActionFlags, 
				const AudioTimeStamp 		*inTimeStamp, 
				UInt32 						inBusNumber, 
				UInt32 						inNumberFrames, 
				AudioBufferList 			*ioData) {
	UInt32 frame;
	UInt32 channel;
	UInt32 inChannel = ioData->mNumberBuffers;
	int count;

	for (channel = 0; channel < inChannel; channel++)
		for (frame = 0; frame < inNumberFrames; frame++)
			((Float32 *)( ioData->mBuffers[channel].mData ))[frame] = 
				0.8 * sin( 3.1415 * 2 * frame * 440 / 48000 );

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
	input.inputProc = MyRenderer;
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
	perl_context = PERL_GET_CONTEXT;
	CreateDefaultAU();
}

void sound_stop() {
	printf("stop\n");
	CloseDefaultAU();
}



END_OF_C_CODE
#------------------------------------------------------------

use HTTP::Daemon;

my $OP;
my $flag_stop : shared = 0;
my $thread : shared;

my $render : shared = sub {
	my $self = shift;
	my $size = shift;
	my $channels = shift;
	my $frame = $self->{frame};
	my (@w, $i, $v);
	for ($i = $size-1; $i >= 0; $i--) {
		$v = 0.8 * sin( 3.1415 * 2 * $frame * 440 / 48000 );
		$w[$i + $size*0] = $v;
		$w[$i + $size*1] = $v;
		$frame++;
	}
	$self->{frame} = $frame;
	return \@w;
};


sub new {
	my $class = shift;
	my $options = shift;

	my $daemon = HTTP::Daemon->new(
		LocalPort => $options->{-port}
	) or die $!;

	return bless {
		daemon => $daemon
	}, $class;
}

sub start {
	my $self = shift;
	$OP = $self;
	$| = 1;

	my $thread = threads->new(\&_start_server_loop);

	_audio_loop();
}

sub _start_server_loop {
	my $self = $OP;
	my ($c, $req, $header, $res);
		print "0\n";
	while ( $c = $self->{daemon}->accept ) {
		$flag_stop and exit(0);
		print "1\n";
		while ( $req = $c->get_request ) {
			$flag_stop and exit(0);
		print "2\n";
			$header = HTTP::Headers->new('Content-Type' => 'text/plain');
		print "3\n";
			eval($req->content);
		print "4\n";
			if( $@ ){
				$res = HTTP::Response->new(400, 'eval failed', $header, $@);
			}else{
				$res = HTTP::Response->new(200, 'OK', $header, 'ok');
			}
			$c->send_response($res);
		}
		print "5\n";
		$c->close;
		undef($c);
		print "6\n";
	}
		print "7\n";
}

sub _audio_loop {
	print "[audio start]..";
	$OP->{frame} = 0;
	sound_start();
	print "[ok]\n";

	$SIG{INT} = sub {
		print "caught SIGINT in main thread\n";

		print "[audio stop]..";
		sound_stop();
		print "[ok]\n";

		$flag_stop = 1;
		$thread->join;
		exit(0);
	};

	while(1) {
		sleep(10);
	}
}

sub _render_callback {
	my $w = $render->($OP, @_);
	return @$w;
}

1;
