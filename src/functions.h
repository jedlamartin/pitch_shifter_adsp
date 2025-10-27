#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stddef.h>

#include "frame.h"

#define N            2048    // block length
#define Sa           128     //
#define alpha        0.5f
#define Ss           (alpha * Sa)
#define L            (Sa * alpha / 2)
#define block_num    (N / Sa - 1)
#define block_length (Sa * 2)

extern volatile bool process_start;

size_t corr(const fract* arr1, const fract* arr2);

void apply_fade(fract* arr1, fract* arr2, size_t fade_length);

void resample_spline(const fract* input_buffer,
                     size_t N_in,
                     volatile fract* output_buffer,
                     size_t N_out);

#endif
