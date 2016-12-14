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

int webrtcns_init(webrtcns_context **context, int sample_rate, int bit_depth, int mode, int float_ns)
{
    *context = malloc(sizeof(webrtcns_context));
    
    if (*context != NULL) {
        
        (*context)->sample_rate = sample_rate;
        (*context)->bit_depth = bit_depth;
        (*context)->mode = mode;
        (*context)->float_ns = float_ns;
        
        memset((*context)->qmf.analysis_state1, 0, sizeof((*context)->qmf.analysis_state1));
        memset((*context)->qmf.analysis_state2, 0, sizeof((*context)->qmf.analysis_state2));
        memset((*context)->qmf.synthesis_state1, 0, sizeof((*context)->qmf.synthesis_state1));
        memset((*context)->qmf.synthesis_state2, 0, sizeof((*context)->qmf.synthesis_state2));
        
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

int webrtcns_process_frame_8k_16k(webrtcns_context *context, void *in_frame, void *out_frame)
{
    if (context && in_frame && out_frame && (context->sample_rate == 8000 || context->sample_rate == 16000)) {
        
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

int webrtcns_process_frame_32k(webrtcns_context *context, void *in_frame, void *out_frame)
{
    if (context && in_frame && out_frame && context->sample_rate == 32000) {
        
        short in_low_band[160] = {0};
        short in_high_band[160] = {0};
        short out_low_band[160] = {0};
        short out_high_band[160] = {0};

        WebRtcSpl_AnalysisQMF(in_frame, 320, in_low_band, in_high_band, context->qmf.analysis_state1, context->qmf.analysis_state2);

        if (context->float_ns) {
            if (WebRtcNs_Process(context->ns_handle, in_low_band, in_high_band, out_low_band, out_high_band) != 0) {
                return -1;
            }
        } else {
            if (WebRtcNsx_Process(context->ns_handle, in_low_band, in_high_band, out_low_band, out_high_band) != 0) {
                return -1;
            }
        }
        
        WebRtcSpl_SynthesisQMF(out_low_band, out_high_band, 160, out_frame, context->qmf.synthesis_state1, context->qmf.synthesis_state2);
    }
    
    return 0;
}

int webrtcns_process_frame(webrtcns_context *context, void *in_frame, void *out_frame)
{
    if (context->sample_rate == 8000 || context->sample_rate == 16000) {
        return webrtcns_process_frame_8k_16k(context, in_frame, out_frame);
    } else if (context->sample_rate == 32000) {
        return webrtcns_process_frame_32k(context, in_frame, out_frame);
    } else {
        return -1;
    }
}

int webrtcns_process_buffer(webrtcns_context *context, unsigned int in_bytes, const void *in_buffer, void **out_buffer, unsigned int *out_bytes, void **remain_buffer, unsigned int *remain_bytes)
{
    unsigned int samples_10ms = context->sample_rate * 0.01;
    unsigned int bytes_10ms = samples_10ms * (context->bit_depth/8);
    unsigned target_bytes = (in_bytes / bytes_10ms) * bytes_10ms;

    *out_buffer = malloc(target_bytes);
    memset(*out_buffer, 0, target_bytes);
    
    for (int i = 0; i < in_bytes; i += bytes_10ms) {
        
        if (in_bytes - i >= bytes_10ms) {
            
            void *in_frame = malloc(bytes_10ms);
            memset(in_frame, 0, bytes_10ms);
            void *out_frame = malloc(bytes_10ms);
            memset(out_frame, 0, bytes_10ms);
            
            memcpy(in_frame, in_buffer + i, bytes_10ms);
            
            if (webrtcns_process_frame(context, in_frame, out_frame) == 0) {
                memcpy(*out_buffer + i, out_frame, bytes_10ms);
                
                free(in_frame);
                free(out_frame);
            } else {
                free(*out_buffer);
                *out_buffer = NULL;
                
                free(in_frame);
                free(out_frame);
                return -1;
            }
        }
    }
    
    *out_bytes = target_bytes;
    *remain_bytes = in_bytes - target_bytes;
    if (*remain_bytes > 0) {
        *remain_buffer = malloc(*remain_bytes);
        memset(*remain_buffer, 0, *remain_bytes);
        memcpy(*remain_buffer, in_buffer + target_bytes, *remain_bytes);
    }
    
    return 0;
}
