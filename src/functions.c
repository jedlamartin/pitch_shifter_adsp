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

void apply_fade(fract* arr1, const fract* arr2, size_t fade_length) {
    if(fade_length == 0) return;
    int32_t dx_32 = (int32_t) FRACT_MAX / (int32_t) fade_length;
    int32_t fade_in_acc_32 = 0;
    for(size_t i = 0; i < fade_length; ++i) {
        int32_t v1 = (int32_t) (((int64_t) arr1[i] *
                                 (int64_t) (FRACT_MAX - fade_in_acc_32)) >>
                                ACC_SCALE);
        int32_t v2 =
            (int32_t) (((int64_t) arr2[i] * (int64_t) fade_in_acc_32) >>
                       ACC_SCALE);
        int32_t res = v1 + v2;
        res =
            (res > FRACT_MAX) ? FRACT_MAX : (res < FRACT_MIN ? FRACT_MIN : res);
        arr1[i] = (fract) res;
        fade_in_acc_32 += dx_32;
    }
}

fract cubic_interp(fract y0, fract y1, fract y2, fract y3, fract x) {
    // Standard Catmull-Rom Spline (alpha = 0.5)
    fract c0 = y1;
    fract c1 = 0.5f * (y2 - y0);
    fract c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
    fract c3 = -0.5f * y0 + 1.5f * y1 - 1.5f * y2 + 0.5f * y3;

    // Evaluate the polynomial: ((c3*x + c2)*x + c1)*x + c0
    return ((c3 * x + c2) * x + c1) * x + c0;
}

void resample_spline(const fract* input_buffer,
                     size_t N_in,
                     fract* output_buffer,
                     size_t N_out) {
    if(input_buffer == NULL || output_buffer == NULL || N_in == 0 ||
       N_out == 0) {
        return;
    }

    // --- 1. Calculate the resampling ratio ---
    // Use double-precision for the ratio to avoid drift.
    // We map (N_in - 1) samples to (N_out - 1) samples.
    // This ensures the last output sample maps to the last input sample.
    double ratio = 0.0;
    if(N_out > 1) {
        ratio = (double) (N_in - 1) / (double) (N_out - 1);
    }

    // Accumulator for the current position in the source buffer.
    // Use 'double' for high precision to prevent rounding errors.
    double source_pos = 0.0;

    const size_t max_index = N_in - 1;

    for(size_t n = 0; n < N_out; ++n) {
        // --- 2. Get integer and fractional parts ---
        // Integer part (the index of the sample *before* our position)
        size_t i = (size_t) source_pos;

        // Fractional part (how far we are between sample 'i' and 'i+1')
        fract x = (fract) (source_pos - (double) i);

        // --- 3. Handle boundary (end of buffer) ---
        // If we are at or past the last valid index, we can't interpolate.
        // Just clamp to the last sample.
        if(i >= max_index) {
            output_buffer[n] = input_buffer[max_index];
            source_pos += ratio;    // Still advance the position
            continue;
        }

        // --- 4. Get the 4 points for interpolation ---
        // We are interpolating between y1 (at index i) and y2 (at index i+1)
        fract y1 = input_buffer[i];
        fract y2 = input_buffer[i + 1];

        // Handle boundary (start of buffer)
        // If i=0, we have no y0, so just "repeat" y1
        fract y0 = (i > 0) ? input_buffer[i - 1] : y1;

        // Handle boundary (end of buffer)
        // If i+2 is out of bounds, "repeat" y2
        fract y3 = (i + 2 < N_in) ? input_buffer[i + 2] : y2;

        // --- 5. Interpolate and store the result ---
        output_buffer[n] = cubic_interp(y0, y1, y2, y3, x);

        // Advance the accumulator for the next output sample
        source_pos += ratio;
    }
}