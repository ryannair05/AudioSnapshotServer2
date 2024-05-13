//
//  Tweak.c
//  AudioSnapshotServer2
//
//  Created by Ryan Nair on 8/25/23.
//

#include <AudioToolbox/AudioToolbox.h>
#include <arpa/inet.h>
#include <os/log.h>
#include <substrate.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MULTICAST_ADDR "239.255.0.1"
#define MULTICAST_PORT 44333
#define SEND_INTERVAL 85

static int udpSocket = -1;
static struct sockaddr_in multicastAddr;
static struct msghdr msg;

static void sendDataViaUDP(const void *audioData, UInt32 dataSize) {
    static struct timespec lastSendTime = {0, 0};
    struct timespec currentTime;
    clock_gettime(CLOCK_MONOTONIC, &currentTime);

    long elapsedMs = (currentTime.tv_sec - lastSendTime.tv_sec) * 1000 + (currentTime.tv_nsec - lastSendTime.tv_nsec) / 1000000;
    
    if (audioData && elapsedMs >= SEND_INTERVAL) {
        struct iovec iov;
        iov.iov_base = (void *)audioData;
        iov.iov_len = dataSize;

        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;

        ssize_t sentBytes = sendmsg(udpSocket, &msg, 0);
        if (sentBytes < 0) {
            os_log_error(OS_LOG_DEFAULT, "sendto error");
        } else {
            lastSendTime = currentTime;
        }
    }
}

OSStatus (*orig_AudioUnitRender)(AudioUnit unit, AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inOutputBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData);

OSStatus function_AudioUnitRender(AudioUnit unit, AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inOutputBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData) {
    AudioComponentDescription unitDescription = {0};
    AudioComponentGetDescription(AudioComponentInstanceGetComponent(unit), &unitDescription);

    if (unitDescription.componentSubType == 'mcmx' && inNumberFrames > 64) {
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_LOW, 0), ^(void) {
            sendDataViaUDP(ioData->mBuffers[0].mData, ioData->mBuffers[0].mDataByteSize);
        });
    }

    return orig_AudioUnitRender(unit, ioActionFlags, inTimeStamp, inOutputBusNumber, inNumberFrames, ioData);
}

__attribute__((destructor)) void deinit() {
    if (udpSocket >= 0) {
        close(udpSocket);
        udpSocket = -1;
    }
}

__attribute__((constructor)) void init() {
    udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udpSocket < 0) {
        os_log_error(OS_LOG_DEFAULT, "socket failed to setup");
        return;
    }

    memset(&multicastAddr, 0, sizeof(multicastAddr));
    multicastAddr.sin_family = AF_INET;
    multicastAddr.sin_addr.s_addr = inet_addr(MULTICAST_ADDR);
    multicastAddr.sin_port = htons(MULTICAST_PORT);

    memset(&msg, 0, sizeof(msg));
    msg.msg_name = &multicastAddr;
    msg.msg_namelen = sizeof(multicastAddr);

    MSHookFunction((void *)AudioUnitRender, (void *)&function_AudioUnitRender, (void **)&orig_AudioUnitRender);
}
