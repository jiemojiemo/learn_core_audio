
//
// Created by William.Hua on 2020/9/28.
//

#include <iostream>
#include <string>
#include <AudioToolbox/AudioFile.h>

using namespace std;

static void printKeys (const void* key, const void* value, void* context) {
    CFShow(key);
    CFShow(value);
}

int main(int argc, char* argv[])
{
    if(argc < 2){
        cerr << "Usage: ch01_main /full/path/to/audiofile\n";
        return -1;
    }

    AudioFileID audio_file;
    CFStringRef audio_url_str = CFStringCreateWithCString(kCFAllocatorDefault, argv[1], kCFStringEncodingUTF8);
    CFURLRef audio_url = CFURLCreateWithString(kCFAllocatorDefault, audio_url_str, NULL);

    auto err = AudioFileOpenURL(audio_url, kAudioFileReadPermission, 0, &audio_file);
    assert(err == noErr);

    UInt32 dictionary_size = 0;
    err = AudioFileGetPropertyInfo(audio_file, kAudioFilePropertyInfoDictionary, &dictionary_size, 0);
    assert(err == noErr);

    CFDictionaryRef dictionary;
    err = AudioFileGetProperty(audio_file, kAudioFilePropertyInfoDictionary, &dictionary_size, &dictionary);
    assert(err == noErr);

    CFShow(dictionary);

    return 0;
}