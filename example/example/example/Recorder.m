//
//  Recorder.m
//  example
//
//  Created by Jc on 16/12/13.
//  Copyright © 2016年 jichuan. All rights reserved.
//

#import "Recorder.h"

static void HandleInputBuffer (void                                  *aqData,
                               AudioQueueRef                         inAQ,
                               AudioQueueBufferRef                   inBuffer,
                               const AudioTimeStamp                  *inStartTime,
                               UInt32                                inNumPackets,
                               const AudioStreamPacketDescription    *inPacketDesc
                               )
{
    NSData *data = [NSData dataWithBytes:inBuffer->mAudioData length:inBuffer->mAudioDataByteSize];

    Recorder *recorder = (__bridge Recorder *)(aqData);
    
    dispatch_async(recorder.processQueue, ^{
    
        [recorder.bufferData appendData:data];
        
        void *outBuffer = NULL;
        void *remainBuffer = NULL;
        unsigned int outBytes = 0;
        unsigned int remainBytes = 0;
        webrtcns_process_buffer(recorder.nsContext, (unsigned int)recorder.bufferData.length, recorder.bufferData.bytes, &outBuffer, &outBytes, &remainBuffer, &remainBytes);
        
        UInt32 in_num_packets = outBytes / 2;
        if (recorder.recorderState.mAudioFile) {
            AudioFileWritePackets(recorder.recorderState.mAudioFile,
                                  false,
                                  outBytes,
                                  inPacketDesc,
                                  recorder.recorderState.mCurrentPacket,
                                  &in_num_packets,
                                  outBuffer);
            [recorder addNumPackets:in_num_packets];
        }

        if (remainBuffer) {
            recorder.bufferData = [NSMutableData dataWithBytes:remainBuffer length:remainBytes];
        } else {
            recorder.bufferData = [NSMutableData data];
        }
        
        free(outBuffer);
        free(remainBuffer);
    });
    
    if (recorder.recorderState.mIsRunning) {
        AudioQueueEnqueueBuffer(recorder.recorderState.mQueue,
                                inBuffer,
                                0,
                                NULL);
    }
}

void DeriveBufferSize (AudioQueueRef                audioQueue,
                       AudioStreamBasicDescription  asbd,
                       Float64                      seconds,
                       UInt32                       *outBufferSize
                       )
{
    static const int maxBufferSize = 0x50000;
    int maxPacketSize = asbd.mBytesPerPacket;
    if (maxPacketSize == 0) {
        UInt32 maxVBRPacketSize = sizeof(maxPacketSize);
        AudioQueueGetProperty(audioQueue,
                              kAudioQueueProperty_MaximumOutputPacketSize,
                              &maxPacketSize,
                              &maxVBRPacketSize);
        
    }
    Float64 numBytesForTime = asbd.mSampleRate * seconds * maxPacketSize;
    *outBufferSize = (UInt32)(numBytesForTime < maxBufferSize ? numBytesForTime : maxBufferSize);
}

void SetupAudioStreamBasicDescription (AudioStreamBasicDescription *asbd) {
    asbd->mFormatID = kAudioFormatLinearPCM;
    asbd->mSampleRate = 16000;
    asbd->mChannelsPerFrame = 1;
    asbd->mBitsPerChannel = 16;
    asbd->mBytesPerPacket = asbd->mBytesPerFrame = sizeof(SInt16);
    asbd->mFramesPerPacket = 1;
    asbd->mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
}

@implementation Recorder

@synthesize recorderState, nsContext;

- (instancetype)init
{
    self = [super init];
    if (self) {
        self.processQueue = dispatch_queue_create("webrtc_ns_queue", DISPATCH_QUEUE_SERIAL);
    }
    return self;
}

- (void)addNumPackets:(UInt32)numPackets
{
    recorderState.mCurrentPacket += numPackets;
}

#pragma mark - Public API
- (BOOL)startRecording:(NSString *)filePath
{
    webrtcns_init(&nsContext, 16000, 16, 1, 0);

    self.bufferData = [NSMutableData data];

    recorderState.mIsRunning = true;
    recorderState.mCurrentPacket = 0;
    
    SetupAudioStreamBasicDescription(&recorderState.mDataFormat);
    
    AudioQueueNewInput(&recorderState.mDataFormat,
                       HandleInputBuffer,
                       (__bridge void * _Nullable)(self),
                       NULL,
                       kCFRunLoopCommonModes,
                       0,
                       &recorderState.mQueue);
    
    UInt32 dataFormatSize = sizeof(recorderState.mDataFormat);
    AudioQueueGetProperty(recorderState.mQueue,
                          kAudioQueueProperty_StreamDescription,
                          &recorderState.mDataFormat,
                          &dataFormatSize);
    
    if (filePath.length) {
        CFURLRef audioFileURL = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault,
                                                                        (const UInt8 *)[filePath UTF8String],
                                                                        strlen([filePath UTF8String]),
                                                                        false);
        AudioFileCreateWithURL(audioFileURL,
                               kAudioFileWAVEType,
                               &recorderState.mDataFormat,
                               kAudioFileFlags_EraseFile,
                               &recorderState.mAudioFile);
        CFRelease(audioFileURL);
    }
    
    DeriveBufferSize(recorderState.mQueue,
                     recorderState.mDataFormat,
                     0.5,
                     &recorderState.mBufferByteSize);
    
    for (int i = 0; i < kRecorderNumberBuffers; i++) {
        AudioQueueAllocateBuffer(recorderState.mQueue,
                                 recorderState.mBufferByteSize,
                                 &recorderState.mBuffers[i]);
        AudioQueueEnqueueBuffer(recorderState.mQueue,
                                recorderState.mBuffers[i],
                                0,
                                NULL);
    }
    
    AudioQueueStart(recorderState.mQueue, NULL);
    return YES;
}

- (void)stopRecording
{
    if (recorderState.mIsRunning) {
        [self stopRecordingMerely];
    }
}

- (void)stopRecordingMerely
{
    if (recorderState.mIsRunning) {
        recorderState.mIsRunning = false;
        recorderState.mCurrentPacket = 0;
        
        AudioQueueStop(recorderState.mQueue, true);
        AudioQueueDispose(recorderState.mQueue, true);
        
        if (recorderState.mAudioFile) {
            AudioFileClose(recorderState.mAudioFile);
            recorderState.mAudioFile = NULL;
        }
        
        dispatch_sync(self.processQueue, ^{
            webrtcns_destory(nsContext);
        });
    }
}

- (BOOL)isRecording
{
    return recorderState.mIsRunning;
}

@end
