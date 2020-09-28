
//
// Created by William.Hua on 2020/9/28.
//
#include "scope_guard.h"
#include <AudioToolbox/AudioToolbox.h>
#include <iostream>
#include <string>
#include <cmath>
using namespace std;

#define SAMPLE_RATE 44100
#define DURATION 5.0
#define FILENAME_FORMAT "%0.3f-sine.aif"

std::string createOutputFileName(float hz)
{
    char buf[512];
    sprintf(buf, FILENAME_FORMAT, hz);
    return string(buf);
}

CFURLRef createOutputFileURL(const std::string& std_str)
{
    CFStringRef audio_url_str = CFStringCreateWithCString(kCFAllocatorDefault,
                                                          std_str.c_str(),
                                                          kCFStringEncodingUTF8);

    CFURLRef audio_url = CFURLCreateWithString(kCFAllocatorDefault, audio_url_str, NULL);
    CFRelease(audio_url_str);
    return audio_url;
}

int main(int argc, char* argv[])
{
    if(argc < 2){
        cerr << "Usage: tone_file_generator n\n(where n is tone in Hz)\n";
        return -1;
    }

    float hz = atof(argv[1]);
    assert(hz > 0);
    cout << "generating " << hz << " tone\n";

    string file_name = createOutputFileName(hz);
    CFURLRef file_url = createOutputFileURL(file_name);
    ON_SCOPE_EXIT([file_url](){CFRelease(file_url);});

    // Prepare the format
    AudioStreamBasicDescription asbd;
    memset(&asbd, 0, sizeof(asbd));
    asbd.mSampleRate = SAMPLE_RATE;
    asbd.mFormatID = kAudioFormatLinearPCM;
    asbd.mFormatFlags = kAudioFormatFlagIsBigEndian |
                        kAudioFormatFlagIsSignedInteger |
                        kAudioFormatFlagIsPacked;
    asbd.mBitsPerChannel = 16;
    asbd.mChannelsPerFrame = 1; // mono
    asbd.mFramesPerPacket = 1;
    asbd.mBytesPerFrame = 2;
    asbd.mBytesPerPacket = 2;

    // set up the file
    AudioFileID audio_file;
    OSStatus audio_err = noErr;
    audio_err = AudioFileCreateWithURL(file_url, kAudioFileAIFFType, &asbd, kAudioFileFlags_EraseFile, &audio_file);
    assert(audio_err == noErr);
    ON_SCOPE_EXIT([audio_file](){AudioFileClose(audio_file);});

    // start writing samples
    long max_sample_count = SAMPLE_RATE * DURATION;
    long sample_count = 0;
    UInt32 bytes_to_write = 2;
    long wave_length_in_samples = std::lroundf(SAMPLE_RATE / hz);

    while(sample_count < max_sample_count){
        for(int i = 0; i < wave_length_in_samples; ++i){
//            SInt16 sample = CFSwapInt16HostToBig( (i/wave_length_in_samples) * 2 * SHRT_MAX - SHRT_MAX );
              SInt16 sample = CFSwapInt16HostToBig((SInt16)SHRT_MAX * sin(2*M_PI*(1.0f*i/wave_length_in_samples)));
//            if(i < wave_length_in_samples/2){
//                sample = CFSwapInt16BigToHost(SHRT_MAX);
//            }else{
//                sample = CFSwapInt16HostToBig(SHRT_MIN);
//            }
            audio_err = AudioFileWriteBytes(audio_file,false,sample_count*2,&bytes_to_write, &sample);
            assert(audio_err == noErr);
            ++sample_count;
        }
    }

    cout << "wrote " << sample_count << " samples\n";
    cout << bitset<sizeof(SInt16) * 8>(SHRT_MAX) << endl;
    cout << bitset<sizeof(SInt16) * 8>(CFSwapInt16HostToBig(SHRT_MAX)) << endl;

    return 0;
}