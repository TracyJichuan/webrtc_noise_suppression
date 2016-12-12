//
//  webrtcns.c
//
//  Created by Jc on 16/12/12.
//  Copyright © 2016年 jichuan. All rights reserved.
//

#include "webrtcns.h"

int webrtcns_init(webrtcns_context **context, int sampleRate, int mode)
{
    *context = malloc(sizeof(webrtcns_context));
    
    if (*context != NULL) {
        
        if (WebRtcNs_Create(&(*context)->ns_handle) != 0) {
            free(*context);
            return -1;
        }
        
        if (WebRtcNs_Init((*context)->ns_handle, sampleRate) != 0) {
            free(*context);
            return -1;
        }
        
        if (WebRtcNs_set_policy((*context)->ns_handle, mode) != 0) {
            free(*context);
            return -1;
        }
        
        return 0;
        
    } else {
        return -1;
    }
}

int webrtcns_destory(webrtcns_context *context)
{
    if (context != NULL) {
        free(context);
        context = NULL;
    }
    
    return 0;
}

