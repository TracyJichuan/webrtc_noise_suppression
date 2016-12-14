//
//  ViewController.m
//  example
//
//  Created by Jc on 16/12/12.
//  Copyright © 2016年 jichuan. All rights reserved.
//

#import "ViewController.h"
#import "Recorder.h"
@import AVFoundation;

@interface ViewController ()
@property (nonatomic, strong) Recorder *recorder;
@property (nonatomic, strong) NSString *audioPath;
@property (nonatomic, strong) AVAudioPlayer *audioPlayer;
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    self.recorder = [[Recorder alloc] init];
    
    [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryPlayAndRecord
                                     withOptions:AVAudioSessionCategoryOptionDefaultToSpeaker | AVAudioSessionCategoryOptionAllowBluetooth
                                           error:NULL];
    [[AVAudioSession sharedInstance] setActive:YES error:NULL];
}

- (IBAction)pressRecordButton:(UIButton *)sender
{
    if (self.audioPlayer.isPlaying) {
        return;
    }
    
    if (![self.recorder isRecording]) {
        NSString *randomName = [NSString stringWithFormat:@"%@.wav", @(arc4random()/100000)];
        self.audioPath = [NSTemporaryDirectory() stringByAppendingPathComponent:randomName];
        NSLog(@"audio path is = %@", self.audioPath);
        
        [self.recorder startRecording:self.audioPath];
        
        [sender setTitle:@"停止录音" forState:UIControlStateNormal];
    } else {
        [self.recorder stopRecording];
        
        [sender setTitle:@"开始录音" forState:UIControlStateNormal];
    }
}

- (IBAction)pressAudioButton:(UIButton *)sender
{
    if (!self.audioPath.length || [self.recorder isRecording]) {
        return;
    }
    
    if (!self.audioPlayer) {
        self.audioPlayer = [[AVAudioPlayer alloc] initWithContentsOfURL:[NSURL fileURLWithPath:self.audioPath] error:NULL];
        [self.audioPlayer prepareToPlay];
        [self.audioPlayer play];
        
        [sender setTitle:@"停止播放" forState:UIControlStateNormal];
    } else {
        [self.audioPlayer stop];
        self.audioPlayer = nil;
        
        [sender setTitle:@"播放录音" forState:UIControlStateNormal];
    }
}

@end
