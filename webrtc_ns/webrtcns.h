//
//  webrtcns.h
//
//  Created by Jichuan on 16/12/12.
//  Copyright © 2016 Jichuan. All rights reserved.
//

#ifndef webrtcns_h
#define webrtcns_h

#endif /* webrtcns_h */

#include <stdio.h>

typedef struct webrtcns_QMF {
    int32_t analysis_state1[6];
    int32_t analysis_state2[6];
    int32_t synthesis_state1[6];
    int32_t synthesis_state2[6];
} webrtcns_QMF;

typedef struct webrtcns_context {
    void            *ns_handle;
    webrtcns_QMF    qmf;
    unsigned int    sample_rate;
    unsigned int    bit_depth;
    unsigned int    mode;
    unsigned int    float_ns;
} webrtcns_context;


/*
 * Init an webrtcns unit and return pointer to created webrtcns_context instance.
 *
 * Input:
 *      - sample_rate   : Support only 8k/16k/32k
 *      - bit_depth     : Supoort only 16 bit
 *      - mode          : Noise suppression mode; Mild(0)/Medium(1)/Aggressive(2)
 *      - float_ns      : Fixed-point or Floating-point
 *
 * Output:
 *      - context       : Pointer to created webrtcns_context instance
 *
 * Return value         : Ok(0) or Error(-1)
 */
int webrtcns_init(webrtcns_context **context, int sample_rate, int bit_depth, int mode, int float_ns);


/*
 * Free the dynamic memory of a specified webrtcns_context instance.
 *
 * Input:
 *      - context       : Pointer to webrtcns_context instance that should be freed
 *
 * Return value         : Ok(0) or Error(-1)
 */
int webrtcns_destory(webrtcns_context *context);


/*
 * Noise Suppression for the input audio buffer.
 * Each processed audio frame is always 10ms; if the last audio frame is less than 10ms,
 * recycle it through remain_buffer and remain_bytes, then append it at the head of next audio buffer
 *
 * Input:
 *      - context       : Pointer to webrtcns_context instance
 *      - in_bytes      : The input audio buffer byte size
 *      - in_buffer     : The input audio buffer
 *
 * Output:
 *      - out_buffer    : Pointer to output audio buffer
 *      - out_bytes     : Pointer to output audio buffer byte size
 *      - remain_buffer : Pointer to remain audio buffer
 *      - remain_bytes  : Pointer to remain audio buffer byte size
 *
 * Return value         : Ok(0) or Error(-1)
 */
int webrtcns_process_buffer(webrtcns_context *context, unsigned int in_bytes, const void *in_buffer, void **out_buffer, unsigned int *out_bytes, void **remain_buffer, unsigned int *remain_bytes);

