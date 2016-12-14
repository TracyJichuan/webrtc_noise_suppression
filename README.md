# webrtc_noise_suppression
This is a webrtc de-noising module encapsulation and noise suppression unit for PCM stream.

## Features
+ Cross-platform
+ Easy-Use

## Install
Download zip and drag the 'webrtc_ns' folder to your project.

## Usage
 

```
Objective-C: 

#import "webrtcns.h"
......

webrtcns_init(&context, 16000, 16, 1, 1);
......

while(1) {
  void *in_buffer =  GetPreviousRemainAudioBuffer() + GetNewAudioBuffer();
  void *out_buffer = NULL;
  void *remain_buffer = NULL;
  unsigned int out_bytes = 0;
  unsigned int remain_bytes = 0;
  
  webrtcns_process_buffer(context, in_bytes, in_buffer, &out_buffer, &out_bytes, &remain_buffer, &remain_bytes);
  
  // handle the out buffer
  
  free(out_buffer);
  free(remain_buffer);
}
......

webrtcns_destory(context);
```
