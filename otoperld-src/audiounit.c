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

#include <CoreAudio/CoreAudio.h>
#include "audiounit.h"
#include "coreaudioutilities.h"

#define UNUSED(x) (void)(x)

// ----------------------------------------------------- private functions
OSStatus	MyRenderer_Perl(void 				*inRefCon, 
				AudioUnitRenderActionFlags 	*ioActionFlags, 
				const AudioTimeStamp 		*inTimeStamp, 
				UInt32 						inBusNumber, 
				UInt32 						inNumberFrames, 
				AudioBufferList 			*ioData);
OSStatus	MyRenderer_Perl2(void 				*inRefCon, 
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
AudioBufferList *inputBuffer;

void (*audiounit_callback)(AudioBuffer *outbuf, UInt32 frames, UInt32 channels);

OSStatus	MyRenderer_Perl(void 				*inRefCon, 
				AudioUnitRenderActionFlags 	*ioActionFlags, 
				const AudioTimeStamp 		*inTimeStamp, 
				UInt32 						inBusNumber, 
				UInt32 						inNumberFrames, 
				AudioBufferList 			*ioData) {
	UNUSED(inRefCon);
	UNUSED(ioData);
	
	OSStatus err = AudioUnitRender(gOutputUnit,
		ioActionFlags,
		inTimeStamp,
		inBusNumber,
		inNumberFrames,
		inputBuffer);
	if (err) {
		printf ("AudioUnitRender failed. (%4.4s, %ld)\n", (char*)&err, (long int)err);
		return err;
	}
	
	audiounit_callback(inputBuffer->mBuffers
		, inNumberFrames, inputBuffer->mNumberBuffers);
	
	return err;
}

OSStatus	MyRenderer_Perl2(void 				*inRefCon, 
				AudioUnitRenderActionFlags 	*ioActionFlags, 
				const AudioTimeStamp 		*inTimeStamp, 
				UInt32 						inBusNumber, 
				UInt32 						inNumberFrames, 
				AudioBufferList 			*ioData) {
	OSStatus err = noErr;
	
	UNUSED(inRefCon);
	UNUSED(ioActionFlags);
	UNUSED(inTimeStamp);
	UNUSED(inBusNumber);
	
	for (UInt32 b = 0; b < inputBuffer->mNumberBuffers; b++) {
		memcpy(
			ioData->mBuffers[b].mData,
			inputBuffer->mBuffers[b].mData,
			inNumberFrames * sizeof(Float32)
		);
	}
	
	return err;
}


void	CreateDefaultAU( int channel, int sample_rate ) {
	OSStatus err = noErr;

	// Open the default output unit
	AudioComponentDescription desc;
	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_HALOutput;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
	
	AudioComponent comp = AudioComponentFindNext(NULL, &desc);
	if (comp == NULL) { printf ("AudioComponentFindNext\n"); return; }
	
	err = AudioComponentInstanceNew(comp, &gOutputUnit);
	if (comp == NULL) { printf ("AudioComponentInstanceNew=%ld\n", (long int)err); return; }

	// enable input
	UInt32 enableIO = 1;
	err = AudioUnitSetProperty (gOutputUnit,
								kAudioOutputUnitProperty_EnableIO,
								kAudioUnitScope_Input,
								1,
								&enableIO,
								sizeof(enableIO));
	if (err) { printf ("AudioUnitSetProperty-EnableInput=%4.4s, %ld\n", (char*)&err, (long int)err); return; }

	// set current input device
	AudioObjectID inputDevice = getDefaultDeviceID(true);
	AudioObjectID outputDevice = getDefaultDeviceID(false);
	AudioObjectID inOutDevice = getInputOutputDevice(inputDevice, outputDevice, channel);
	
	err = AudioUnitSetProperty (gOutputUnit,
								kAudioOutputUnitProperty_CurrentDevice,
								kAudioUnitScope_Global,
								0,
								&inOutDevice,
								sizeof(inOutDevice));
	if (err) { printf ("AudioUnitSetProperty-CurrentDevice-0=%ld\n", (long int)err); return; }
	err = AudioUnitSetProperty (gOutputUnit,
								kAudioOutputUnitProperty_CurrentDevice,
								kAudioUnitScope_Global,
								1,
								&inOutDevice,
								sizeof(inOutDevice));
	if (err) { printf ("AudioUnitSetProperty-CurrentDevice-1=%ld\n", (long int)err); return; }

	// Set up a callback function to generate output to the output unit
	AURenderCallbackStruct output;
	output.inputProc = MyRenderer_Perl2;
	output.inputProcRefCon = NULL;
	err = AudioUnitSetProperty (gOutputUnit, 
								kAudioUnitProperty_SetRenderCallback, 
								kAudioUnitScope_Input,
								0, 
								&output, 
								sizeof(output));
	if (err) { printf ("AudioUnitSetProperty-CallbackOut=%ld\n", (long int)err); return; }

	// Set up a callback function to obtain inputs
	AURenderCallbackStruct input;
	input.inputProc = MyRenderer_Perl;
	input.inputProcRefCon = NULL;
	err = AudioUnitSetProperty (gOutputUnit, 
								kAudioOutputUnitProperty_SetInputCallback, 
								kAudioUnitScope_Global,
								1, 
								&input, 
								sizeof(input));
	if (err) { printf ("AudioUnitSetProperty-CallbackIn=%ld\n", (long int)err); return; }

	// Set stream format
	AudioStreamBasicDescription streamFormat;
	streamFormat.mSampleRate = sample_rate;      //	the sample rate of the audio stream
	streamFormat.mFormatID = theFormatID;        //	the specific encoding type of audio stream
	streamFormat.mFormatFlags = theFormatFlags;  //	flags specific to each format
	streamFormat.mBytesPerPacket = theBytesInAPacket;
	streamFormat.mFramesPerPacket = theFramesPerPacket;
	streamFormat.mBytesPerFrame = theBytesPerFrame;
	streamFormat.mChannelsPerFrame = channel;
	streamFormat.mBitsPerChannel = theBitsPerChannel;
	
	setSamplingRateToDevice(inputDevice, sample_rate);
	
	err = AudioUnitSetProperty (gOutputUnit,
								kAudioUnitProperty_StreamFormat,
								kAudioUnitScope_Output,
								1,
								&streamFormat,
								sizeof(AudioStreamBasicDescription));
	if (err) { printf ("AudioUnitSetProperty-FromInputFormat=%4.4s, %ld\n", (char*)&err, (long int)err); return; }

	err = AudioUnitSetProperty (gOutputUnit,
								kAudioUnitProperty_StreamFormat,
								kAudioUnitScope_Input,
								0,
								&streamFormat,
								sizeof(AudioStreamBasicDescription));
	if (err) { printf ("AudioUnitSetProperty-IntoOutputFormat=%4.4s, %ld\n", (char*)&err, (long int)err); return; }

	// alloc bufferList
	UInt32 bufferSizeFrames;
	UInt32 size = sizeof(bufferSizeFrames);
	err = AudioUnitGetProperty (gOutputUnit, 
								kAudioDevicePropertyBufferFrameSize, kAudioUnitScope_Global,
								1,
								&bufferSizeFrames,
								&size);
	if (err) { printf ("AudioUnitGetProperty-BufferFrameSize=%4.4s, %ld\n", (char*)&err, (long int)err); return; }
	inputBuffer = allocAudioBufferList(channel, bufferSizeFrames);

	// Initialize unit
	err = AudioUnitInitialize(gOutputUnit);
	if (err) { printf ("AudioUnitInitialize=%ld\n", (long int)err); return; }

	// Start the rendering
	// The DefaultOutputUnit will do any format conversions to the format of the default device
	err = AudioOutputUnitStart (gOutputUnit);
	if (err) { printf ("AudioOutputUnitStart=%ld\n", (long int)err); return; }	
}

void CloseDefaultAU () {
	OSStatus err = noErr;

	err = AudioOutputUnitStop (gOutputUnit);
	if (err) { printf ("AudioOutputUnitStop=%ld\n", (long int)err); }

	err = AudioUnitUninitialize (gOutputUnit);
	if (err) { printf ("AudioUnitUninitialize=%ld\n", (long int)err); }

	AudioComponentInstanceDispose (gOutputUnit);

	deallocAudioBufferList(inputBuffer);
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
