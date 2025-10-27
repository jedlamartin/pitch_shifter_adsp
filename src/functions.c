#include "functions.h"

#define ACC_SCALE    15
#define FRACT_Q_MASK FRACT_MAX

size_t corr(const fract* arr1, const fract* arr2) {
    size_t max_index = 0;
    int32_t max_value = 0;
    for(size_t k = 0; k < L; ++k) {
        int32_t accu = 0;
        for(size_t n = 0; n < L - k; ++n) {
            accu += (int32_t) (arr1[n] * arr2[n + k]);
        }
        if(accu > max_value) {
            max_value = accu;
            max_index = k;
        }
    }
    return max_index;
}

void apply_fade(fract* arr1, fract* arr2, size_t fade_length) {
    if(fade_length == 0) return;
    fract dx = (fract) (FRACT_MAX / fade_length);
    fract mul = 0;
    for(size_t i = 0; i < fade_length; ++i) {
            arr1[block_length - 1 - fade_length + i] *= mul;
            arr2[i] *= mul;
            mul += dx;
    }
}

float cubic_interp(float y0, float y1, float y2, float y3, float x) {
    float c0 = y1;
    float c1 = 0.5f * (y2 - y0);
    float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
    float c3 = -0.5f * y0 + 1.5f * y1 - 1.5f * y2 + 0.5f * y3;
    return ((c3 * x + c2) * x + c1) * x + c0;
}


void resample_spline(const fract* input_buffer,
                     size_t N_in,
                     volatile fract* output_buffer,
                     size_t N_out)
{
    if (!input_buffer || !output_buffer || N_in < 4 || N_out == 0)
        return;

    float ratio = (float)(N_in - 1) / (float)(N_out - 1);
    float source_index = 0.0f;

    for (size_t n = 0; n < N_out; ++n)
    {
        if (source_index > (float)(N_in - 1))
            source_index = (float)(N_in - 1);

        int i = (int)source_index;
        if (i < 1) i = 1;
        if (i > (int)(N_in - 3)) i = N_in - 3;

        fract x = (fract)(source_index - (float)i);
        if (x < 0) x = 0;
        if (x > 1) x = 1;

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
