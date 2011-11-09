// OtoPerl - live sound programming environment with Perl.
// OtoPerl::otoperld - sound processing server for OtoPerl.
// Otoperl::otoperld::audiounit - run AudioUnit for otoperld.
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

#include <CoreServices/CoreServices.h>

#include "audiounit.h"

// ----------------------------------------------------- private functions
OSStatus	MyRenderer_Perl(void 				*inRefCon, 
				AudioUnitRenderActionFlags 	*ioActionFlags, 
				const AudioTimeStamp 		*inTimeStamp, 
				UInt32 						inBusNumber, 
				UInt32 						inNumberFrames, 
				AudioBufferList 			*ioData);

void CreateDefaultAU( int channel, int sample_rate );
void CloseDefaultAU();
// -------------------------------------------- audiounit implimentation

const UInt32 theFormatID = kAudioFormatLinearPCM;
// these are set based on which format is chosen
const UInt32 theFormatFlags = kAudioFormatFlagsNativeFloatPacked | kAudioFormatFlagIsNonInterleaved;
const UInt32 theBytesInAPacket = 4;
const UInt32 theBitsPerChannel = 32;
const UInt32 theBytesPerFrame = 4;
// these are the same regardless of format
const UInt32 theFramesPerPacket = 1; // this shouldn't change'

AudioUnit	gOutputUnit;

void (*audiounit_callback)(AudioBuffer *outbuf, UInt32 frames, UInt32 channels);

OSStatus	MyRenderer_Perl(void 				*inRefCon, 
				AudioUnitRenderActionFlags 	*ioActionFlags, 
				const AudioTimeStamp 		*inTimeStamp, 
				UInt32 						inBusNumber, 
				UInt32 						inNumberFrames, 
				AudioBufferList 			*ioData) {
	
	audiounit_callback(ioData->mBuffers
		, inNumberFrames, ioData->mNumberBuffers);
	
	return noErr;
}


void	CreateDefaultAU( int channel, int sample_rate ) {
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
	streamFormat.mSampleRate = sample_rate;      //	the sample rate of the audio stream
	streamFormat.mFormatID = theFormatID;        //	the specific encoding type of audio stream
	streamFormat.mFormatFlags = theFormatFlags;  //	flags specific to each format
	streamFormat.mBytesPerPacket = theBytesInAPacket;
	streamFormat.mFramesPerPacket = theFramesPerPacket;
	streamFormat.mBytesPerFrame = theBytesPerFrame;
	streamFormat.mChannelsPerFrame = channel;
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

	verify_noerr (AudioOutputUnitStop (gOutputUnit));

	err = AudioUnitUninitialize (gOutputUnit);
	if (err) { printf ("AudioUnitUninitialize=%ld\n", (long int)err); return; }

	CloseComponent (gOutputUnit);
}

// ---------------------------
void audiounit_start( int channel, int sample_rate, void (*callback)(AudioBuffer *outbuf, UInt32 frames, UInt32 channels) ) {
	audiounit_callback = callback;
	CreateDefaultAU( channel, sample_rate );
	printf("audiounit start.\n");
}

void audiounit_stop() {
	CloseDefaultAU();
	printf("audiounit stop.\n");
}



