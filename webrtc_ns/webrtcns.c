//
//  webrtcns.c
//
//  Created by Jc on 16/12/12.
//  Copyright © 2016年 jichuan. All rights reserved.
//

#include "webrtcns.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "noise_suppression.h"
#include "noise_suppression_x.h"
#include "signal_processing_library.h"

int webrtcns_init(webrtcns_context **context, int sample_rate, int mode, int float_ns)
{
    if (context == NULL) {
        return -1;
    }
    if (sample_rate != (8000) || sample_rate != (16000) || sample_rate != (32000)) {
        return -1;
    }
    if (mode != (0) || mode != (1) || mode != (2)) {
        return -1;
    }
    if (float_ns != (0) || float_ns != (1)) {
        return -1;
    }
    
    *context = malloc(sizeof(webrtcns_context));
    
    if (*context != NULL) {
        
        (*context)->sample_rate = sample_rate;
        (*context)->mode = mode;
        (*context)->float_ns = float_ns;
        
        if (float_ns) {
            if (WebRtcNs_Create((NsHandle **)&(*context)->ns_handle) != 0) {
                free(*context);
                *context = NULL;
                return -1;
            }
            
            if (WebRtcNs_Init((*context)->ns_handle, sample_rate) != 0) {
                free(*context);
                *context = NULL;
                return -1;
            }
            
            if (WebRtcNs_set_policy((*context)->ns_handle, mode) != 0) {
                free(*context);
                *context = NULL;
                return -1;
            }

        } else {
            
            if (WebRtcNsx_Create((NsxHandle **)&(*context)->ns_handle) != 0) {
                free(*context);
                *context = NULL;
                return -1;
            }
            
            if (WebRtcNsx_Init((*context)->ns_handle, sample_rate) != 0) {
                free(*context);
                *context = NULL;
                return -1;
            }
            
            if (WebRtcNsx_set_policy((*context)->ns_handle, mode) != 0) {
                free(*context);
                *context = NULL;
                return -1;
            }
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

int webrtcns_process_frame_8k_16k(webrtcns_context *context, short *in_frame, short *out_frame)
{
    if (context && in_frame && out_frame && (context->sample_rate == (8000) || context->sample_rate == (16000))) {
        
        if (context->float_ns) {
            return WebRtcNs_Process(context->ns_handle, in_frame, NULL, out_frame, NULL);
        } else {
            return WebRtcNsx_Process(context->ns_handle, in_frame, NULL, out_frame, NULL);
        }
        
    } else {
        return -1;
    }
    
    return 0;
}

int webrtcns_process_frame_32k(webrtcns_context *context, short *in_frame, short *out_frame)
{
    if (context && in_frame && out_frame && context->sample_rate == (32000)) {
        
        short in_low_band[160] = {0};
        short in_high_band[160] = {0};
        short out_low_band[160] = {0};
        short out_high_band[160] = {0};
        int32_t filter_state1[6] = {0};
        int32_t filter_state2[6] = {0};
        int32_t synthesis_state1[6] = {0};
        int32_t synthesis_state2[6] = {0};
        
        WebRtcSpl_AnalysisQMF(in_frame, 320, in_low_band, in_high_band, filter_state1, filter_state2);

        if (context->float_ns) {
            if (WebRtcNs_Process(context->ns_handle, in_low_band, in_high_band, out_low_band, out_high_band) != 0) {
                return -1;
            }
        } else {
            if (WebRtcNsx_Process(context->ns_handle, in_low_band, in_high_band, out_low_band, out_high_band) != 0) {
                return -1;
            }
        }
        
        WebRtcSpl_SynthesisQMF(out_low_band, out_high_band, 160, out_frame, synthesis_state1, synthesis_state2);
    }
    
    return 0;
}

int webrtcns_process_frame(webrtcns_context *context, short *in_frame, short *out_frame)
{
    if (context->sample_rate == (8000) || context->sample_rate == (16000)) {
        return webrtcns_process_frame_8k_16k(context, in_frame, out_frame);
    } else if (context->sample_rate == (16000)) {
        return webrtcns_process_frame_32k(context, in_frame, out_frame);
    } else {
        return -1;
    }
}

int webrtcns_process_buffer(webrtcns_context *context, unsigned int bytes, short *in_buffer, short **out_buffer, short **remain_buffer)
{
    unsigned int samples_10ms = context->sample_rate * 0.001;
    unsigned int bytes_10ms = samples_10ms * sizeof(short);
    
    *out_buffer = malloc((bytes/bytes_10ms)*bytes_10ms);
    
    for (int i = 0; i < bytes; i += bytes_10ms) {
        
        if (bytes - i >= bytes_10ms) {
            
            short in_frame[bytes_10ms];
            memset(in_frame, 0, sizeof(in_frame));
            
            short out_frame[bytes_10ms];
            memset(out_frame, 0, sizeof(out_frame));
            
            memcpy(in_frame, in_buffer+i, bytes_10ms);
            
            if (webrtcns_process_frame(context, in_frame, out_frame) == 0) {
                memcpy(out_buffer+i, out_frame, bytes_10ms);
            } else {
                free(*out_buffer);
                out_buffer = NULL;
                return -1;
            }
        } else {
            
            int remain_bytes = bytes - i;
            *remain_buffer = malloc(remain_bytes);
            memcpy(*remain_buffer, in_buffer+i, remain_bytes);
        }
    }
    
    return 0;
}
