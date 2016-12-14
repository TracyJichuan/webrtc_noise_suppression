//
//  Recorder.h
//  example
//
//  Created by Jc on 16/12/13.
//  Copyright © 2016年 jichuan. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "webrtcns.h"
@import AudioToolbox;

static const int kRecorderNumberBuffers = 3;
typedef struct {
    AudioStreamBasicDescription mDataFormat;
    AudioQueueRef               mQueue;
    AudioQueueBufferRef         mBuffers[kRecorderNumberBuffers];
    AudioFileID                 mAudioFile;
    UInt32                      mBufferByteSize;
    SInt64                      mCurrentPacket;
    bool                        mIsRunning;
} RecorderState;

@interface Recorder : NSObject
@property (nonatomic) webrtcns_context *nsContext;
@property (nonatomic, assign) RecorderState recorderState;
@property (nonatomic, strong) NSMutableData *bufferData;
@property (nonatomic, strong) dispatch_queue_t processQueue;

- (BOOL)startRecording:(NSString *)filePath;
- (void)stopRecording;
- (BOOL)isRecording;
- (void)addNumPackets:(UInt32)numPackets;
@end
