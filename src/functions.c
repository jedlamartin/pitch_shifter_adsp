#include "functions.h"

size_t corr(const fract* arr1, const fract* arr2) {
    size_t max_index = 0;
    int32_t max_value = 0;
    for(size_t k = 0; k < L; ++k) {
        int32_t accu = 0;
        for(size_t n = 0; n < L - k; ++n) {
            accu += (int32_t) arr1[n] * arr2[n + k];
        }
        if(accu > max_value) {
            max_value = accu;
            max_index = k;
        }
    }
    return max_index;
}

void apply_fade(fract* arr1, fract* arr2, size_t fade_length) {
    fract dx = (fract) (FRACT_ONE / fade_length);
    for(size_t i = 0; i < fade_length; ++i) {
        fract mul = (fract) (((int32_t) i * dx) >> 15);
        arr1[block_length - 1 - fade_length + i] *= mul;
        arr2[i] *= mul;
    }
}

#define FRACT_ONE 0x7FFF

float cubic_interp(float y0, float y1, float y2, float y3, float x) {
    float a = y3 - y2 - y0 + y1;
    float b = y0 - y1 - a;
    float c = y2 - y0;
    float d = y1;

    return d + x * (c + x * (b + x * a));
}

void resample_spline(const fract* input_buffer,
                     size_t N_in,
                     fract* output_buffer,
                     size_t N_out) {
    // Safety check
    if(input_buffer == NULL || output_buffer == NULL || N_in == 0 ||
       N_out == 0) {
        return;
    }

    const float ratio = (float) N_in / (float) N_out;

    // Index boundary check
    const float max_index = (float) N_in - 1.0;

    for(size_t n = 0; n < N_out; ++n) {
        // 2. Determine the source index
        float source_index = (float) n * ratio;

        if(source_index >= max_index) {
            output_buffer[n] = input_buffer[N_in - 1];
            continue;
        }

        // Find the integer part (i) and fractional part (x) of the source
        // index
        size_t i = (size_t) floor(source_index);
        float x = source_index - i;

        // y1 and y2 are the main points (i and i+1)
        float y1_fp = (float) input_buffer[i] / FRACT_ONE;
        float y2_fp = (float) input_buffer[i + 1] / FRACT_ONE;

        // y0 is the point before i. Clamp and convert.
        float y0_fp;
        if(i > 0) {
            y0_fp = (float) input_buffer[i - 1] / FRACT_ONE;
        } else {
            y0_fp = (float) input_buffer[0] / FRACT_ONE;
        }

        // y3 is the point after i+1. Clamp and convert.
        float y3_fp;
        if(i + 2 < N_in) {
            y3_fp = (float) input_buffer[i + 2] / FRACT_ONE;
        } else {
            y3_fp = (float) input_buffer[N_in - 1] / FRACT_ONE;
        }

        float result_fp = cubic_interp(y0_fp, y1_fp, y2_fp, y3_fp, x);

        fract result_fract = (fract) round(result_fp * FRACT_ONE);

        output_buffer[n] = result_fract;
    }
}