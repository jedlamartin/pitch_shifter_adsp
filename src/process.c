#include "process.h"

#define N            2048    // block length
#define Sa           256     //
#define alpha        1
#define Ss           (alpha * Sa)
#define L            (Sa * alpha / 2)
#define block_num    (N / (Sa * 2))
#define block_length (Sa * 2)

void process() {
    fract out_tmp[block_num * Sa * 2] = {0};
    size_t out_tmp_index = 0;

    // Time stretch
    fract blocks[block_num][2 * Sa];
    // memcpy(blocks[i], input_current[Sa*i], 2*Sa);
    for(int i = 0; i < block_num; ++i) {
        for(int j = 0; j < block_length; ++j) {
            blocks[i][j] = input_current[Sa * i + j];
        }
    }

    int km = 2 * Sa - Ss;
    int fade_length;
    for(int i = 0; i < block_num - 1; ++i) {
        fade_length = 2 * Sa - Ss - km;

        for(int j = 0; j < fade_length; ++j) {
            out_tmp[out_tmp_index - fade_length + j] += blocks[i][j];
        }

        {
            int copy_count = block_length - fade_length;
            for(int c = 0; c < copy_count; ++c) {
                out_tmp[out_tmp_index + c] = blocks[i][fade_length + c];
            }
        }
        // memcpy(out_tmp + out_tmp_index,
        //         blocks[i] + fade_length, block_length - fade_length);

        km = corr(blocks[i], blocks[i + 1]);

        apply_fade(out_tmp + out_tmp_index, blocks[i + 1], fade_length);

        out_tmp_index += block_length - fade_length;
    }

    for(int j = 0; j < fade_length; ++j) {
        out_tmp[out_tmp_index - fade_length + j] += blocks[block_num - 1][j];
    }

    {
        int copy_count = block_length - fade_length;
        for(int c = 0; c < copy_count; ++c) {
            out_tmp[out_tmp_index + c] = blocks[block_num][fade_length + c];
        }
    }
    // memcpy(out_tmp + out_tmp_index, blocks[block_num] + fade_length,
    //       block_length - fade_length);
    out_tmp_index += block_length - fade_length;

    // Interpolate
    resample_spline(out_tmp, out_tmp_index, output_current, N);
}
