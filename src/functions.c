#include "functions.h"

size_t corr(const fract* arr1, const fract* arr2) {
    size_t max_index = 0;

    accum max_value = -1;

    for(size_t k = 0; k < L; ++k) {
        accum accu = 0;

        // 1. Get the length of this window
        size_t window_len = L - k;

        if(window_len == 0) continue;    // Should not happen, but safe

        for(size_t n = 0; n < window_len; ++n) {
            size_t arr1_index = block_length - L + k + n;
            size_t arr2_index = n;

            // This part is correct
            accu += arr1[arr1_index] * arr2[arr2_index];
        }

        // 2. NORMALIZE the sum to get an average score
        accum normalized_accu = accu / window_len;

        // 3. Compare the average score
        if(normalized_accu > max_value) {
            max_value = normalized_accu;
            max_index = k;
        }
    }

    return max_index;
}

void apply_fade(fract* arr1, fract* arr2, size_t fade_length) {
    if(fade_length == 0) return;
    fract dx = FRACT_MAX / fade_length;
    fract mul = 0;
    for(size_t i = 0; i < fade_length; ++i) {
        arr1[fade_length - 1 - i] *= mul;
        arr2[i] *= mul;
        mul += dx;
    }
}

fract cubic_interp(fract y0, fract y1, fract y2, fract y3, fract x) {
    accum c0 = y1;

    accum c1 = 0.5r * (y2 - y0);

    accum c2 = y0 - y1 - y1 - 0.5r * y1 + y2 + y2 - 0.5r * y3;

    accum c3 = -0.5r * y0 + y1 + 0.5r * y1 - y2 - 0.5r * y2 + 0.5r * y3;

    accum out = ((c3 * x + c2) * x + c1) * x + c0;
    return rbits(accum);
}

void resample_spline(const fract* input_buffer,
                     size_t N_in,
                     volatile fract* output_buffer,
                     size_t N_out) {
    if(!input_buffer || !output_buffer || N_in < 4 || N_out == 0) return;

    // float ratio = (float) (N_in - 1) / (float) (N_out - 1);
    accum ratio = (N_in - 1) / (N_out - 1) + rdivi(N_in - 1, N_out - 1);
    // float source_index = 0.0f;
    accum source_index = 0;

    for(size_t n = 0; n < N_out; ++n) {
        if(source_index > (N_in - 1)) {
            source_index = (N_in - 1);
        }

        int i = source_index - rbits(source_index);
        if(i < 1) i = 1;
        if(i > N_in - 3) i = N_in - 3;

        fract x = rbits(source_index);
        if(x < 0) x = 0;

        // Safe memory access (all within [0, N_in-1])
        fract y0 = input_buffer[i - 1];
        fract y1 = input_buffer[i];
        fract y2 = input_buffer[i + 1];
        fract y3 = input_buffer[i + 2];

        output_buffer[n] = cubic_interp(y0, y1, y2, y3, x);

        source_index += ratio;
    }

    output_buffer[N_out - 1] = input_buffer[N_in - 1];
}
