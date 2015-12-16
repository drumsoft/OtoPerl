// OtoPerl - live sound programming environment with Perl.
// OtoPerl::otoperld - sound processing server for OtoPerl.
// Otoperl::otoperld::coreaudioutilities - Core Audio Utilities.
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

#include <CoreFoundation/CoreFoundation.h>
#include <CoreAudio/CoreAudio.h>
#include "coreaudioutilities.h"

#define NOERR(t, l, r) if (t) { printf ("%s=%4.4s, %ld\n", l, (char*)&t, (long int)t); return r; }
#define CFSTRING2STR(x) CFStringGetCStringPtr(x, kCFStringEncodingMacRoman)
#define CFNUMBER2INT(x) CFNumberGetCInt(x)

CFNumberRef CFNumberFromInt(int value) {
	CFNumberRef result = CFNumberCreate(NULL, kCFNumberIntType , &value);
	assert(result != NULL);
	return result;
}

int CFNumberGetCInt(CFNumberRef number) {
	int value;
	Boolean r = CFNumberGetValue(number, kCFNumberIntType, &value);
	assert(r);
	return value;
};

CFDictionaryRef getAudioSubDeviceDescription(AudioObjectID objectID, int enableIn, int enableOut, int compensateDrift);

// --------------------------------------------------------------------

// set sampling rate to audio device.
void setSamplingRateToDevice(AudioObjectID objectID, Float64 samplingRate) {
	UInt32 size = sizeof(Float64);
	AudioObjectPropertyAddress address = {
		kAudioDevicePropertyNominalSampleRate,
		kAudioObjectPropertyScopeGlobal,
		kAudioObjectPropertyElementMaster
	};
	OSErr err = AudioObjectSetPropertyData(objectID, &address, 0, NULL, size, &samplingRate);
	if (err) {
		printf ("sampling rate is not supported. (%4.4s, %ld)\n", (char*)&err, (long int)err);
		exit(-1);
	}
}

// get default input/output device ID.
AudioObjectID getDefaultDeviceID(bool isInput) {
	AudioObjectID result;
	UInt32 size = sizeof(result);
	AudioObjectPropertyAddress address = {
		isInput ? kAudioHardwarePropertyDefaultInputDevice : kAudioHardwarePropertyDefaultOutputDevice,
		kAudioObjectPropertyScopeGlobal,
		kAudioObjectPropertyElementMaster
	};
	OSErr err = AudioObjectGetPropertyData(kAudioObjectSystemObject, &address, 0, NULL, &size, &result);
	if (err) {
		printf ("input device not found. (%4.4s, %ld)\n", (char*)&err, (long int)err);
		exit(-1);
	}
	return result;
}

// aggregate input/output devices.
AudioObjectID getInputOutputDevice(AudioObjectID inputDevice, AudioObjectID outputDevice, int channels) {
	OSStatus err = noErr;
	if (inputDevice == outputDevice) { // in+out is in the single device.
		return outputDevice;
	}
	
	// aggregate
	AudioObjectID aggregatedDevice;
	CFNumberRef trueRef = CFNumberFromInt(1);
	CFNumberRef falseRef = CFNumberFromInt(0);
	CFDictionaryRef inputDeviceDescription  = getAudioSubDeviceDescription(inputDevice, channels, 0, 0);
	CFDictionaryRef outputDeviceDescription = getAudioSubDeviceDescription(outputDevice, 0, channels, 1);
	void * keys[6] = {
		(void *)CFSTR(kAudioAggregateDeviceUIDKey),
		(void *)CFSTR(kAudioAggregateDeviceNameKey),
		(void *)CFSTR(kAudioAggregateDeviceSubDeviceListKey),
		(void *)CFSTR(kAudioAggregateDeviceMasterSubDeviceKey),
		(void *)CFSTR(kAudioAggregateDeviceIsPrivateKey),
		(void *)CFSTR(kAudioAggregateDeviceIsStackedKey)
	};
	void * list[2] = {
		(void *)inputDeviceDescription,
		(void *)outputDeviceDescription
	};
	CFArrayRef subDeviceList = CFArrayCreate(NULL, (void *)list, (CFIndex)2, NULL);
	assert(subDeviceList != NULL);
	void * values[6] = {
		(void *)CFSTR("OtoperlDefaultInputOutputAggregated"), // UID
		(void *)CFSTR("Otoperl Default Input+Output Aggregated"), // Name
		(void *)subDeviceList,
		(void *)CFDictionaryGetValue(inputDeviceDescription, CFSTR(kAudioSubDeviceUIDKey)), // MasterSubDeviceUID
		(void *)trueRef, // isPrivate
		(void *)falseRef // isStacked
	};
	CFDictionaryRef inDescription = CFDictionaryCreate (NULL,
			(void *)keys,
			(void *)values,
			(CFIndex)6,
			NULL,
			NULL);
	assert(inDescription != NULL);
	err = AudioHardwareCreateAggregateDevice(inDescription, &aggregatedDevice);
	NOERR(err, "AudioHardwareCreateAggregateDevice", 0);
	CFRelease(inputDeviceDescription);
	CFRelease(outputDeviceDescription);
	CFRelease(subDeviceList);
	CFRelease(trueRef);
	CFRelease(falseRef);
	CFRelease(inDescription);
	return aggregatedDevice;
}

CFDictionaryRef getAudioSubDeviceDescription(AudioObjectID objectID, int channelsIn, int channelsOut, int compensateDrift) {
	OSStatus err = noErr;
	UInt32 size;
	
	AudioObjectPropertyAddress address = {  kAudioDevicePropertyDeviceUID,
											kAudioObjectPropertyScopeGlobal,
											kAudioObjectPropertyElementMaster };
	CFStringRef deviceUID;
	size = sizeof(deviceUID);
	err = AudioObjectGetPropertyData(objectID, &address, 0, NULL, &size, &deviceUID);
	NOERR(err, "AudioObjectGetPropertyData-deviceUID", NULL);
	
	address.mSelector = kAudioObjectPropertyName;
	CFStringRef deviceName;
	size = sizeof(deviceName);
	err = AudioObjectGetPropertyData(objectID, &address, 0, NULL, &size, &deviceName);
	NOERR(err, "AudioObjectGetPropertyData-deviceUID", NULL);
	
	CFNumberRef in = CFNumberFromInt(channelsIn);
	CFNumberRef out = CFNumberFromInt(channelsOut);
	CFNumberRef cDrift = CFNumberFromInt(compensateDrift);
	
	void * keys[5] = {
		(void *)CFSTR(kAudioSubDeviceUIDKey),
		(void *)CFSTR(kAudioSubDeviceNameKey),
		(void *)CFSTR(kAudioSubDeviceInputChannelsKey),
		(void *)CFSTR(kAudioSubDeviceOutputChannelsKey),
		(void *)CFSTR(kAudioSubDeviceDriftCompensationKey)
	};
	void * values[5] = {
		(void *)deviceUID,
		(void *)deviceName,
		(void *)in,
		(void *)out,
		(void *)cDrift
	};
	CFDictionaryRef result = CFDictionaryCreate(NULL,
			(void *)keys, (void *)values, (CFIndex)5,
			NULL, NULL);
	assert(result != NULL);
	CFRelease(deviceUID);
	CFRelease(deviceName);
	CFRelease(in);
	CFRelease(out);
	CFRelease(cDrift);
	return result;
	/*
		{
			kAudioSubDeviceUIDKey: "device uid",
			kAudioSubDeviceNameKey: "device name",
			kAudioSubDeviceInputChannelsKey: CFNumber(inputs number),
			kAudioSubDeviceOutputChannelsKey: CFNumber(outputs number),
			kAudioSubDeviceExtraInputLatencyKey: CFNumber(extra input latency in frames),
			kAudioSubDeviceExtraOutputLatencyKey: CFNumber(extra output latency in frames),
			kAudioSubDeviceDriftCompensationKey: CFNumber(drift compensation enabled when non-zero),
			kAudioSubDeviceDriftCompensationQualityKey: CFNumber(quality of drifty compensation),
		}
	*/
}

AudioBufferList * allocAudioBufferList(UInt32 channels, UInt32 frames) {
	UInt32 bufferSizeBytes = frames * sizeof(Float32);
	AudioBufferList *bufferList = (AudioBufferList *)malloc(offsetof(AudioBufferList, mBuffers[0]) + sizeof(AudioBuffer) * channels);
	bufferList->mNumberBuffers = channels;
	for(UInt32 i = 0; i < channels; i++) {
		bufferList->mBuffers[i].mNumberChannels = 1;
		bufferList->mBuffers[i].mDataByteSize = bufferSizeBytes;
		bufferList->mBuffers[i].mData = malloc(bufferSizeBytes);
		memset(bufferList->mBuffers[i].mData, 0, bufferSizeBytes);
	}
	return bufferList;
}

void deallocAudioBufferList(AudioBufferList *bufferList) {
	for(UInt32 i = 0; i < bufferList->mNumberBuffers; i++) {
		free(bufferList->mBuffers[i].mData);
	}
	free(bufferList);
}
