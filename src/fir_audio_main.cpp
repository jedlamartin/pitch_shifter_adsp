#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include <chrono>
#include <ctime>

#include <math.h>

#include <omp.h>

 #include <sndfile.h>

#include "timestamp.h"



#ifdef _MSC_VER
	#define memalign(a, s) _aligned_malloc((s), (a))
	#define memfree(a) _aligned_free((a))
#else
	#define memfree(a) free((a))
#endif


int fir_audio_cpu(int ir_size, float *ir, int record_frames, float *record, int result_frames, float *result);
#ifdef __amd64__
int fir_audio_avx(int ir_size, float *ir, int record_frames, float *record, int result_frames, float *result);
#endif
#ifdef __arm64__
int fir_audio_neon(int ir_size, float *ir, int record_frames, float *record, int result_frames, float *result);
#endif
int ocl_fir(int ocl_dev, int ir_size, float *ir, int record_frames, float *record, int result_frames, float *result, char *KERNEL_FUNCTION);

int main(int argc, char *argv[])
{

	if (argc != 4) {
		printf("Usage:\nfir-audio 'ir filename' 'input filename' 'output filename'\nExiting.\n");
		return -1;
	}
	else {
		printf("Impulse response: %s\n", argv[1]);
		printf("Input recording: %s\n", argv[2]);
		printf("Output file: %s\n", argv[3]);
	}

	SNDFILE *sf_ir, *sf_record, *sf_result;
	SF_INFO sfinfo_ir, sfinfo_record, sfinfo_result;

	sfinfo_ir.format = 0;
	sf_ir = sf_open((char *)(argv[1]), SFM_READ, &sfinfo_ir);
	if (sf_ir==NULL) {
		printf("Error opening impulse response.\nExiting\n");
		return -1;
	}
	else if (sfinfo_ir.channels != 2) {
		printf("Impulse response should contain 2 channels.\nExiting\n");
		sf_close(sf_ir);
		return -1;
	}

	sfinfo_record.format = 0;
	sf_record = sf_open((char *)(argv[2]), SFM_READ, &sfinfo_record);
	if (sf_ir==NULL) {
		printf("Error opening audio recording.\nExiting\n");
		sf_close(sf_ir);
		return -1;
	}
	else if (sfinfo_record.channels != 1) {
		printf("Audio record should contain 1 channel.\nExiting\n");
		sf_close(sf_ir);
		sf_close(sf_record);
		return -1;
	}

	sfinfo_result = sfinfo_record;
	sfinfo_result.channels = 2;
	sf_result = sf_open((char *)(argv[3]), SFM_WRITE, &sfinfo_result);
	if (sf_result==NULL) {
		printf("Error opening destination file.\nExiting\n");
		sf_close(sf_ir);
		sf_close(sf_record);
		return -1;
	}

	// Output size is input size + impulse response size
	sfinfo_result.frames = sfinfo_record.frames + sfinfo_ir.frames - 1;

	// Allocate input buffers
	int buf_size_ir, buf_size_record, buf_size_result;
	buf_size_ir = sfinfo_ir.frames*sfinfo_ir.channels*sizeof(float);
	buf_size_record = (sfinfo_record.frames+sfinfo_ir.frames-1)*sfinfo_record.channels*sizeof(float);
	buf_size_result = sfinfo_result.frames*sfinfo_result.channels*sizeof(float);

	// Round up buffer size to N*4
	buf_size_ir = (buf_size_ir + 3) & (~4);
	buf_size_record = (buf_size_record + 3) & (~4);
	buf_size_result = (buf_size_result + 3) & (~4);

	printf("IR : %d, %ld, %d, %d\n", sfinfo_ir.channels, sfinfo_ir.frames, sfinfo_ir.samplerate, buf_size_ir);
	printf("REC: %d, %ld, %d, %d\n", sfinfo_record.channels, sfinfo_record.frames, sfinfo_record.samplerate, buf_size_record);
	printf("RES: %d, %ld, %d, %d\n", sfinfo_result.channels, sfinfo_result.frames, sfinfo_result.samplerate, buf_size_result);

	// Allocate aligned buffers
	float *ir, *ir_rev, *audio_src, *audio_dst;
	ir = (float*)(memalign(4096, buf_size_ir));
	ir_rev = (float*)(memalign(4096, buf_size_ir));
	audio_src = (float*)(memalign(4096, buf_size_record));
	audio_dst = (float*)(memalign(4096, buf_size_result));

	// Extend input samples with zeros at the beginning
	for (int s=0; s<sfinfo_ir.frames-1; s++) {
		*(audio_src + s) = 0.0;
	}
	// Read input sound files
	sf_readf_float(sf_ir, ir, sfinfo_ir.frames);
	sf_readf_float(sf_record, audio_src+sfinfo_ir.frames-1, sfinfo_record.frames);

	for (int s= 0; s<sfinfo_ir.frames*sfinfo_ir.channels; s++) {
		*(ir_rev + s) = *(ir + sfinfo_ir.frames*sfinfo_ir.channels - s);
	}

	double ts_start, ts_end, elapsed, perf;
#if 0
	printf("CPU convolution start\n");
	ts_start = get_ts_ns();
	fir_audio_cpu(sfinfo_ir.frames, ir_rev,
			      sfinfo_record.frames+sfinfo_ir.frames-1, audio_src,
				  sfinfo_result.frames, audio_dst);
	ts_end = get_ts_ns();
	elapsed = ts_end - ts_start;
	perf = (2.0*2.0*double(sfinfo_result.frames)*double(sfinfo_ir.frames))/elapsed;
	printf("CPU convolution end, elapsed: %f s; %f gflops/s\n", (ts_end-ts_start)/1000000000.0, perf);
#endif

#ifdef __arm64__
#if 1
	printf("CPU NEON convolution start\n");
	ts_start = get_ts_ns();
	fir_audio_neon(sfinfo_ir.frames, ir_rev,
			       sfinfo_record.frames+sfinfo_ir.frames-1, audio_src,
				   sfinfo_result.frames, audio_dst);
	ts_end = get_ts_ns();
	elapsed = ts_end - ts_start;
	perf = (2.0*2.0*double(sfinfo_result.frames)*double(sfinfo_ir.frames))/elapsed;
	printf("CPU NEON convolution end, elapsed: %f s; %f gflops/s\n", (ts_end-ts_start)/1000000000.0, perf);
#endif
#endif

#ifdef __amd64__
#if 1
	fir_audio_avx(sfinfo_ir.frames, ir_rev,
			       sfinfo_record.frames+sfinfo_ir.frames-1, audio_src,
				   sfinfo_result.frames, audio_dst);
#endif
#endif

	// Normalize output values to -1.0...1.0
#if 1
	float s_max = 0.0;
	for (int s=0; s<sfinfo_result.frames*2; s++) {
		float smpl = abs(*(audio_dst + s));
		if (smpl>s_max) s_max = smpl;
	}
	printf("MAX: %f\n", s_max);
	for (int s=0; s<sfinfo_result.frames*2; s++) {
		*(audio_dst + s) = *(audio_dst + s) / s_max;
	}
#endif

	sf_writef_float(sf_result, audio_dst, sfinfo_result.frames);


	memfree(ir);
	memfree(audio_src);
	memfree(audio_dst);

	sf_close(sf_ir);
	sf_close(sf_record);
	sf_close(sf_result);

	printf("fir-audio done.\n");
	return 0;

}
