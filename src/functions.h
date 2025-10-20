#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stddef.h>

#include "frame.h"

#define N            2048    // block length
#define Sa           256     //
#define alpha        1
#define Ss           (alpha * Sa)
#define L            (Sa * alpha / 2)
#define block_num    (N / (Sa * 2))
#define block_length (Sa * 2)

size_t corr(const fract* arr1, const fract* arr2);

void apply_fade(fract* arr1, fract* arr2, size_t fade_length);

void resample_spline(const fract* input_buffer,
                     size_t N_in,
                     fract* output_buffer,
                     size_t N_out);

#endif
