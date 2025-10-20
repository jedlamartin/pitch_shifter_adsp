#ifdef __arm64__

#include <stdio.h>
#include <stdlib.h>

#include <omp.h>
#include <arm_neon.h>

int fir_audio_neon(int ir_size, float *ir, int record_frames, float *record, int result_frames, float *result){
	
	#pragma omp parallel for
	for(size_t sample_idx = 0; sample_idx < result_frames; sample_idx += 4){
		float32x4_t samples_1 = vld1q_f32(record + sample_idx);
		float32x4_t accu[2];
		accu[0] = vdupq_n_f32(0.f);
		accu[1] = vdupq_n_f32(0.f);

		size_t sample_next_idx = sample_idx + 4;
		for(size_t coeff_idx = 0; coeff_idx < ir_size; coeff_idx += 4){

			float32x4_t samples_2;
			sample_next_idx += 4;
			sample_next_idx < record_frames ? samples_2 = vld1q_f32(record + sample_next_idx) : samples_2 = vdupq_n_f32(0.f);

			float32x4_t coeffs = vld1q_f32(ir + coeff_idx * 2);

			float32x4_t curr_samples = samples_1;
			accu[0] = vfmaq_laneq_f32(accu[0], curr_samples, coeffs, 0);
			accu[1] = vfmaq_laneq_f32(accu[1], curr_samples, coeffs, 1);

			curr_samples = vextq_f32(samples_1, samples_2, 1);
			accu[0] = vfmaq_laneq_f32(accu[0], curr_samples, coeffs, 2);
			accu[1] = vfmaq_laneq_f32(accu[1], curr_samples, coeffs, 3);

			coeffs = vld1q_f32(ir + coeff_idx * 2 + 4);
			curr_samples = vextq_f32(samples_1, samples_2, 2);
			accu[0] = vfmaq_laneq_f32(accu[0], curr_samples, coeffs, 0);
			accu[1] = vfmaq_laneq_f32(accu[1], curr_samples, coeffs, 1);

			curr_samples = vextq_f32(samples_1, samples_2, 3);
			accu[0] = vfmaq_laneq_f32(accu[0], curr_samples, coeffs, 2);
			accu[1] = vfmaq_laneq_f32(accu[1], curr_samples, coeffs, 3);

			samples_1 = samples_2;
		}
		float32x4x2_t res = vzipq_f32(accu[0], accu[1]);
		vst2q_f32(result + 2 * sample_idx, res);
	}

	return 0;
}


#endif