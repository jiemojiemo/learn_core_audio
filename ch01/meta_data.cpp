
//
// Created by William.Hua on 2020/9/28.
//

#include "scope_guard.h"
#include <iostream>
#include <string>
#include <AudioToolbox/AudioFile.h>

using namespace std;

int main(int argc, char* argv[])
{
    if(argc < 2){
        cerr << "Usage: meta_data /full/path/to/audiofile\n";
        return -1;
    }

    AudioFileID audio_file;
    ON_SCOPE_EXIT([audio_file]{
        if(audio_file != NULL)
            AudioFileClose(audio_file);
    });

    CFStringRef audio_url_str = CFStringCreateWithCString(kCFAllocatorDefault, argv[1], kCFStringEncodingUTF8);
    ON_SCOPE_EXIT([audio_url_str](){ CFRelease(audio_url_str); });

    CFURLRef audio_url = CFURLCreateWithString(kCFAllocatorDefault, audio_url_str, NULL);
    ON_SCOPE_EXIT([audio_url](){CFRelease(audio_url);});

    auto err = AudioFileOpenURL(audio_url, kAudioFileReadPermission, 0, &audio_file);
    assert(err == noErr);

    UInt32 dictionary_size = 0;
    err = AudioFileGetPropertyInfo(audio_file, kAudioFilePropertyInfoDictionary, &dictionary_size, 0);
    assert(err == noErr);

    CFDictionaryRef dictionary;
    err = AudioFileGetProperty(audio_file, kAudioFilePropertyInfoDictionary, &dictionary_size, &dictionary);
    assert(err == noErr);

    CFShow(dictionary);
    CFRelease(dictionary);

    return 0;
}