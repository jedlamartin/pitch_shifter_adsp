#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stddef.h>

#include "frame.h"

/**
 * @brief Buffer length (number of samples per block).
 *
 * @warning Carefully consider stack size limitations when modifying this value.
 * Large block sizes can cause stack overflow in resource-constrained systems.
 * Monitor stack usage when adjusting N, Sa, or derived parameters.
 */
#define N 1024

/**
 * @brief Overlap length (samples).
 *
 * @warning This parameter affects total memory usage along with N.
 * Ensure combined buffer sizes don't exceed available RAM/stack space.
 */
#define Sa 256

/**
 * @brief Stretch factor for time/pitch scaling.
 *
 * Values > 1 increase pitch/speed, values < 1 decrease pitch/speed.
 */
#define alpha 1.5f

/* Additional derived parameters */

#define Ss           (alpha * Sa)
#define block_num    (N / Sa - 1)
#define block_length (Sa * 2)
#define L            (block_length / 8)

/**
 * @brief Flag indicating when processing should start
 *
 * Set by ISR to signal the processing thread to begin processing the next block
 */
extern volatile bool process_start;

/**
 * @brief Computes correlation between two signal blocks for the last L samples
 *
 * @param arr1 First input array to correlate
 * @param arr2 Second input array to correlate
 * @return Index of maximum correlation between the arrays
 */
size_t corr(const fract* arr1, const fract* arr2);

/**
 * @brief Applies crossfade between two audio blocks
 *
 * @param arr1 First array to fade (fade out)
 * @param arr2 Second array to fade (fade in)
 * @param fade_length Number of samples over which to apply the crossfade
 */
void apply_fade(fract* arr1, fract* arr2, size_t fade_length);

/**
 * @brief Resamples audio data using cubic spline interpolation
 *
 * @param input_buffer Source audio samples to resample
 * @param N_in Number of input samples
 * @param output_buffer Destination buffer for resampled audio
 * @param N_out Number of output samples to generate
 */
void resample_spline(const fract* input_buffer,
                     size_t N_in,
                     volatile fract* output_buffer,
                     size_t N_out);

#endif
