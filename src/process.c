#include "process.h"

#define N            1024   // buffer size
#define Sa           64     //
#define alpha        1.5f
#define Ss           (alpha * Sa)
#define block_num    (N / Sa - 1)
#define block_length (Sa * 2)
#define L            (block_length / 8)

void process(void) {
    fract out_tmp[block_num * Sa * 2] = {0};

    // Time stretch
    fract blocks[block_num][2 * Sa];
    // memcpy(blocks[i], input_current[Sa*i], 2*Sa);
    for(int i = 0; i < block_num; ++i) {
        for(int j = 0; j < block_length; ++j) {
            blocks[i][j] = input_current[Sa * i + j];
        }
    }


    // 0. block

    {
        for(int c = 0; c < block_length; ++c) {
            out_tmp[c] = blocks[0][c];
        }
    }
    size_t out_tmp_index = block_length;



    int km = 2 * Sa - Ss;
    int fade_length;
    for(int i = 1; i < block_num; ++i) {
        km = corr(blocks[i-1], blocks[i]);

        fade_length = 2 * Sa - Ss - km;

        apply_fade(&(out_tmp[out_tmp_index - fade_length]), blocks[i], fade_length);

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

        out_tmp_index +=  block_length - fade_length;

    }

    // Interpolate
    resample_spline(out_tmp, out_tmp_index, output_current, N);
}
