
//
// Created by William.Hua on 2020/9/29.
//
#include <AudioToolbox/AudioToolbox.h>
#include <iostream>
using namespace std;

int main(int argc, char* argv[])
{
    AudioFileTypeAndFormatID  file_type_and_format_id;
    file_type_and_format_id.mFileType = kAudioFileCAFType;
    file_type_and_format_id.mFormatID = kAudioFormatMPEG4AAC;

    OSStatus err = noErr;
    UInt32 info_size = 0;

    err = AudioFileGetGlobalInfoSize(kAudioFileGlobalInfo_AvailableStreamDescriptionsForFormat,
                                     sizeof(file_type_and_format_id),
                                     &file_type_and_format_id,
                                     &info_size);
    assert(err == noErr);
    cout << "info size: " << info_size << endl;

    auto  *asbds = (AudioStreamBasicDescription*)malloc(info_size);
    err = AudioFileGetGlobalInfo(kAudioFileGlobalInfo_AvailableStreamDescriptionsForFormat,
                                 sizeof(file_type_and_format_id),
                                 &file_type_and_format_id,
                                 &info_size,
                                 asbds);
    assert(err == noErr);

    int asbd_count = info_size / sizeof(AudioStreamBasicDescription);
    cout << "asbd count: " << asbd_count << endl;

    for(int i = 0; i < asbd_count; ++i){
        UInt32 format4cc = CFSwapInt32HostToBig(asbds[i].mFormatID);
        cout << i << ": mFormatId: "
             << (char*)(&format4cc) << ", mFormatFlags: "
             << asbds[i].mFormatFlags << ", mBitsPerChannel: "
             << asbds[i].mBitsPerChannel << endl;
    }

    free(asbds);
    return 0;
}