//
//  webrtcns.h
//
//  Created by Jc on 16/12/12.
//  Copyright © 2016年 jichuan. All rights reserved.
//

#ifndef webrtcns_h
#define webrtcns_h

#include <stdio.h>
#include <stdlib.h>
#include "noise_suppression.h"

#endif /* webrtcns_h */

typedef struct webrtcns_context {
    NsHandle *ns_handle;
} webrtcns_context;

int webrtcns_init(webrtcns_context **context, int sampleRate, int mode);

int webrtcns_destory(webrtcns_context *context);
