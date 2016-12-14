//
//  webrtcns.h
//
//  Created by Jc on 16/12/12.
//  Copyright © 2016年 jichuan. All rights reserved.
//

#ifndef webrtcns_h
#define webrtcns_h

#endif /* webrtcns_h */

typedef struct webrtcns_context {
    void *ns_handle;
    unsigned int sample_rate;
    unsigned int bit_depth;
    unsigned int mode;
    unsigned int float_ns;
} webrtcns_context;

int webrtcns_init(webrtcns_context **context, int sample_rate, int bit_depth, int mode, int float_ns);

int webrtcns_destory(webrtcns_context *context);

int webrtcns_process_buffer(webrtcns_context *context, unsigned int in_bytes, const void *in_buffer, void **out_buffer, unsigned int *out_bytes, void **remain_buffer, unsigned int *remain_bytes);




