#include <stdio.h>
#include <stdlib.h>

#include <omp.h>

int fir_audio_cpu(int ir_size, float *ir, int record_frames, float *record, int result_frames, float *result)
{
	#pragma omp parallel for
	for(size_t sample_idx=0; sample_idx<result_frames; ++sample_idx){
		float accu[2] = {0};
		for(size_t coeff_idx = 0; coeff_idx < ir_size; ++coeff_idx){
			float coeff_left = ir[2 * coeff_idx + 0];
			float coeff_right = ir[2 * coeff_idx + 1];
			float sample = sample_idx + coeff_idx < result_frames ? record[sample_idx] : 0.f;

			accu[0] += coeff_left * sample;
			accu[1] += coeff_right * sample;
		}
		result[sample_idx * 2 + 0] = accu[0];
		result[sample_idx * 2 + 1] = accu[1];
	}

	return 0;
}
