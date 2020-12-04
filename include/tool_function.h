
//
// Created by William.Hua on 2020/9/28.
//

#ifndef LEARN_CORE_AUDIO_INCLUDE_TOOL_FUNCTION_H
#define LEARN_CORE_AUDIO_INCLUDE_TOOL_FUNCTION_H

#include "scope_guard.h"
#include <AudioToolbox/AudioToolbox.h>
#include <string>

static void checkError(OSStatus error, const char* operation)
{
    if(error == noErr)
        return;

    char error_str[20];
    *(UInt32 *)(error_str + 1) = CFSwapInt32HostToBig(error);
    if(isprint(error_str[1] && isprint(error_str[2])
                   && isprint(error_str[3]) && isprint(error_str[4]))){
        error_str[0] = error_str[5] = '\'';
        error_str[6] = '\0';
    }else{
        sprintf(error_str, "%d", (int)(error));
    }

    fprintf(stderr, "Error: %s (%s)\n", operation, error_str);
    exit(1);

}


CFStringRef createCFStringUTF8WithStdString(const std::string& str)
{
    CFStringRef cf_str = CFStringCreateWithCString(kCFAllocatorDefault,
                                                          str.c_str(),
                                                          kCFStringEncodingUTF8);
    return cf_str;
}

CFURLRef createCFURLWithStdString(const std::string& str)
{
    CFStringRef cf_str = createCFStringUTF8WithStdString(str);

    CFURLRef urf_ref = CFURLCreateWithString(kCFAllocatorDefault, cf_str, NULL);
    CFRelease(cf_str);

    return urf_ref;
}

#endif //LEARN_CORE_AUDIO_INCLUDE_TOOL_FUNCTION_H
