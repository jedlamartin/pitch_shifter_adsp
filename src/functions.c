#include "functions.h"

#define FRACT_MAX    ((fract) 0x7FFF)
#define FRACT_MIN    ((fract) 0x8000)
#define ACC_SCALE    15
#define FRACT_Q_MASK FRACT_MAX

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
    fract dx = (fract) (FRACT_MAX / fade_length);
    int32_t mul_acc = 0;
    for(size_t i = 0; i < fade_length; ++i) {
        fract mul = (fract) (mul_acc >> ACC_SCALE);
        arr1[block_length - 1 - fade_length + i] *= mul;
        arr2[i] *= mul;
    }
}

fract cubic_interp_fixed(fract y0, fract y1, fract y2, fract y3, fract x) {
    int32_t x_32 = (int32_t) x;
    int32_t y0_32 = (int32_t) y0;
    int32_t y1_32 = (int32_t) y1;
    int32_t y2_32 = (int32_t) y2;
    int32_t y3_32 = (int32_t) y3;

    // x^2, x^3
    int32_t x2_32 = (int32_t) (((int64_t) x_32 * (int64_t) x_32) >> ACC_SCALE);
    int32_t x3_32 = (int32_t) (((int64_t) x_32 * (int64_t) x2_32) >> ACC_SCALE);

    int32_t a_32 = y3_32 - y2_32 - y0_32 + y1_32;
    int32_t b_32 = y0_32 - y1_32 - a_32;
    int32_t c_32 = y2_32 - y0_32;
    int32_t d_32 = y1_32;

    // Calculate terms (term = coefficient * power)
    int32_t term_a =
        (int32_t) (((int64_t) a_32 * (int64_t) x3_32) >> ACC_SCALE);
    int32_t term_b =
        (int32_t) (((int64_t) b_32 * (int64_t) x2_32) >> ACC_SCALE);
    int32_t term_c = (int32_t) (((int64_t) c_32 * (int64_t) x_32) >> ACC_SCALE);

    // Sum the terms using 32-bit addition
    int32_t result_32 = term_a + term_b + term_c + d_32;

    if(result_32 > FRACT_MAX) {
        return FRACT_MAX;
    }
    if(result_32 < FRACT_MIN) {
        return FRACT_MIN;
    }

    return (fract) result_32;
}

void resample_spline(const fract* input_buffer,
                     size_t N_in,
                     fract* output_buffer,
                     size_t N_out) {
    if(input_buffer == NULL || output_buffer == NULL || N_in == 0 ||
       N_out == 0) {
        return;
    }

    float ratio_fp = (float) N_in / (float) N_out;

    // Ratio (Q16.15)
    int32_t ratio = (int32_t) (ratio_fp * (1.0f * (1 << ACC_SCALE)));

    // Accumulator (Q16.15 format)
    int32_t source_index = 0;

    const size_t max_index = N_in - 1;

    for(size_t n = 0; n < N_out; ++n) {
        // Integer part
        size_t i = (size_t) (source_index >> ACC_SCALE);

        // Fractional part
        fract x = (fract) (source_index & FRACT_Q_MASK);

        // Advance the accumulator for the next output sample
        source_index += ratio;

        if(i >= max_index) {
            output_buffer[n] = input_buffer[N_in - 1];
            continue;
        }

        fract y1 = input_buffer[i];
        fract y2 = input_buffer[i + 1];

        fract y0 = (i > 0) ? input_buffer[i - 1] : input_buffer[0];

        fract y3 =
            (i + 2 < N_in) ? input_buffer[i + 2] : input_buffer[N_in - 1];

        output_buffer[n] = cubic_interp_fixed(y0, y1, y2, y3, x);
    }
}