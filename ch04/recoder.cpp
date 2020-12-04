
//
// Created by William.Hua on 2020/12/3.
//
#include "tool_function.h"
#include <AudioToolbox/AudioToolbox.h>

const int kNumberRecordBuffers = 3;

struct MyRecoder {
    AudioFileID  record_file;
    SInt64       record_packet;
    Boolean      running;
};


static void MyAQInputCallback(
    void * __nullable               inUserData,
    AudioQueueRef                   inQueue,
    AudioQueueBufferRef             inBuffer,
    const AudioTimeStamp *          inStartTime,
    UInt32                          inNumPackets,
    const AudioStreamPacketDescription * inPacketDescs)
{
    auto* recoder = reinterpret_cast<MyRecoder*>(inUserData);
    if(inNumPackets > 0){
        checkError(AudioFileWritePackets(recoder->record_file,
                                         FALSE,
                                         inBuffer->mAudioDataByteSize,
                                         inPacketDescs,
                                         recoder->record_packet,
                                         &inNumPackets,
                                         inBuffer->mAudioData),
                   "AudioFileWritePackets failed");

        recoder->record_packet += inNumPackets;
    }

    if(recoder->running){
        checkError(AudioQueueEnqueueBuffer(inQueue,
                                           inBuffer,
                                           0,
                                           NULL),
                   "AudioQueueEnqueueBuffer failed");
    }
}

static void MyCopyEncoderCookieToFile(AudioQueueRef queue, AudioFileID file)
{
    OSStatus error;
    UInt32 property_size;

    error = AudioQueueGetPropertySize(queue,
                                      kAudioConverterCompressionMagicCookie,
                                      &property_size);

    if(error == noErr && property_size >= 0){
        Byte* magic_cookie = (Byte*)malloc(property_size);
        error = AudioQueueGetProperty(queue,
                                      kAudioQueueProperty_MagicCookie,
                                      magic_cookie,
                                      &property_size);
        checkError(error, "Couldn't get audio queue's magic cookie");

        error = AudioFileSetProperty(file,
                                     kAudioFilePropertyMagicCookieData,
                                     property_size,
                                     magic_cookie);
        checkError(error, "Couldn't set audio file's magic cookie");

        free(magic_cookie);
    }
}

static int MyComputeRecordBufferSize(const AudioStreamBasicDescription* format,
                                     AudioQueueRef queue,
                                     float second)
{
    int packets{0};
    int frames{0};
    int bytes{0};

    frames = (int)ceil(second * format->mSampleRate);
    if(format->mBytesPerFrame > 0){
        bytes = frames * format->mBytesPerFrame;
    }else{
        UInt32 max_packet_size;
        if(format->mBytesPerPacket > 0){
            max_packet_size = format->mBytesPerPacket;
        }else{
            UInt32 property_size = sizeof(max_packet_size);
            checkError(AudioQueueGetProperty(queue,
                                             kAudioConverterPropertyMaximumOutputPacketSize,
                                             &max_packet_size,
                                             &property_size),
                       "Couldn't get queue's maximum output packet size");
        }

        if(format->mFramesPerPacket > 0){
            packets = frames / format->mFramesPerPacket;
        }else{
            packets = frames;
        }

        if(packets == 0){
            packets = 1;
        }

        bytes = packets * max_packet_size;
    }

    return bytes;
}

OSStatus MyGetDefaultInputDeviceSampleRate(Float64 *output_sample_rate)
{
    OSStatus error;
    AudioDeviceID deviceID = 0;

    AudioObjectPropertyAddress property_addr;
    UInt32 property_size;
    property_addr.mSelector = kAudioHardwarePropertyDefaultInputDevice;
    property_addr.mScope = kAudioObjectPropertyScopeGlobal;
    property_addr.mElement = kAudioObjectPropertyElementMaster;
    property_size = sizeof(AudioDeviceID);
    error = AudioObjectGetPropertyData(kAudioObjectSystemObject,
                                                &property_addr,
                                                0,
                                                0,
                                                &property_size,
                                                &deviceID);
    if(error)
        return error;

    property_addr.mSelector = kAudioDevicePropertyNominalSampleRate;
    property_addr.mScope = kAudioObjectPropertyScopeGlobal;
    property_addr.mElement = kAudioObjectPropertyElementMaster;
    property_size = sizeof(Float64);
    error  = AudioObjectGetPropertyData(deviceID,
                                                 &property_addr,
                                                 0,
                                                 0,
                                                 &property_size,
                                                 output_sample_rate);
    return error;
}

int main()
{
    MyRecoder recoder{0};

    AudioStreamBasicDescription recode_format;
    memset(&recode_format, 0, sizeof(recode_format));
    recode_format.mFormatID = kAudioFormatMPEG4AAC;
    recode_format.mChannelsPerFrame = 2;

    MyGetDefaultInputDeviceSampleRate(&recode_format.mSampleRate);
    printf("sample_rate: %lf", recode_format.mSampleRate);

    UInt32 property_size = sizeof(recode_format);
    OSStatus error = AudioFormatGetProperty(kAudioFormatProperty_FormatInfo,
                                            0,
                                            NULL,
                                            &property_size,
                                            &recode_format);
    checkError(error, "AudioFormatGetProperty failed");

    AudioQueueRef queue{0};
    error = AudioQueueNewInput(&recode_format,
                               MyAQInputCallback,
                               &recoder,
                               NULL, NULL, 0,
                               &queue);
    checkError(error, "AudioQueueNewInput failed");

    property_size = sizeof(recode_format);
    error = AudioQueueGetProperty(queue,
                                  kAudioConverterCurrentOutputStreamDescription,
                                  &recode_format,
                                  &property_size);
    checkError(error, "AudioQueueGetProperty failed");

    CFURLRef my_file_url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
                                                         CFSTR("output.caf"),
                                                         kCFURLPOSIXPathStyle,
                                                         false);
    error = AudioFileCreateWithURL(my_file_url,
                                   kAudioFileCAFType,
                                   &recode_format,
                                   kAudioFileFlags_EraseFile,
                                   &recoder.record_file);
    checkError(error, "AudioFileCreateWithURL failed");
    CFRelease(my_file_url);

//    MyCopyEncoderCookieToFile(queue, recoder.record_file);
    int buffer_byte_size = MyComputeRecordBufferSize(&recode_format, queue, 0.5f);

    for(int buffer_index = 0;buffer_index < kNumberRecordBuffers; ++buffer_index){
        AudioQueueBufferRef buffer;
        checkError(AudioQueueAllocateBuffer(queue,
                                            buffer_byte_size,
                                            &buffer),
                   "AudioQueueAllocateBuffer failed");
        checkError(AudioQueueEnqueueBuffer(queue,
                                            buffer,
                                            0,
                                            NULL),
                   "AudioQueueEnqueueBuffer failed");
    }

    recoder.running = TRUE;
    checkError(AudioQueueStart(queue,NULL),
               "AudioQueueStart failed");

    printf("Recording, press <return> to stop:\n");
    getchar();

    printf("recording done \n");
    recoder.running = FALSE;
    checkError(AudioQueueStop(queue, TRUE),
               "AudioQueueStop failed");
//    MyCopyEncoderCookieToFile(queue, recoder.record_file);
    AudioQueueDispose(queue, TRUE);
    AudioFileClose(recoder.record_file);

    return 0;
}