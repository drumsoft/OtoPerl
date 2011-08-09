#!/usr/bin/perl

use strict;
use warnings;

my $default_LDDLFLAGS;
BEGIN {
	use Config;
	$default_LDDLFLAGS = $Config{lddlflags};
}

use Inline C => Config =>
    LDDLFLAGS => $default_LDDLFLAGS
     . ' -framework CoreServices -framework CoreAudio -framework AudioUnit';
use Inline 'C';


my $frame = 0;
my @c;
sub render_callback {
	my $size = shift;
	my $channels = shift;
	my (@w, $i, $v);
	for ($i = $size-1; $i >= 0; $i--) {
		my $pan = 0.5 + 0.5 * sin( 3.1415 * 2 * $frame * 1 / 48000 );
		my $fm = 0.5 + 0.5 * sin( 3.1415 * 2 * $frame * 100 / 48000 );
		$v = 0.8 * sin( 3.1415 * 2 * ($frame+5*$fm) * 880 / 48000 );
#		$v = rand(2) - 1;
#		$v = int($frame / 50) % 2 ? 0.8 : -0.8;
		$w[$i + $size*0] = $v * $pan;
		$w[$i + $size*1] = $v * (1-$pan);
		$frame++;
	}
	return @w;
}

sound_start();
sleep(1);
sound_stop();

__END__
__C__
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
const UInt32 theFramesPerPacket = 1; // this shouldn't change

AudioUnit	gOutputUnit;



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

	dSP;
	ENTER;
	SAVETMPS;
	
	PUSHMARK(SP);
	XPUSHs(sv_2mortal(newSViv(inNumberFrames)));
	XPUSHs(sv_2mortal(newSViv(inChannel)));
	PUTBACK;
	
	count = call_pv("main::render_callback", G_ARRAY);
	
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

	// REALLY after you're finished playing STOP THE AUDIO OUTPUT UNIT!!!!!!	
	// but we never get here because we're running until the process is nuked...	verify_noerr (AudioOutputUnitStop (gOutputUnit));
	
	err = AudioUnitUninitialize (gOutputUnit);
	if (err) { printf ("AudioUnitUninitialize=%ld\n", (long int)err); return; }

	CloseComponent (gOutputUnit);
}

// ---------------------------
void sound_start() {
	printf("start\n");
	CreateDefaultAU();
}

void sound_stop() {
	printf("stop\n");
	CloseDefaultAU();
}
