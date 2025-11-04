#ifndef PROCESS_H
#define PROCESS_H
#include "frame.h"
#include "functions.h"

/**
 * @brief Double-buffered input array for real-time processing
 *
 * @details Points to the current input buffer being processed while the other
 * buffer is being filled with new audio samples. Enables continuous real-time
 * audio processing without interruption.
 */
extern volatile fract* input_current;

/**
 * @brief Double-buffered output array for processed audio
 *
 * @details Points to the current output buffer being played back while the
 * other buffer is being filled with newly processed audio samples. Part of the
 * double-buffering scheme for seamless audio output.
 */
extern volatile fract* output_current;

/**
 * @brief Main DSP processing function for pitch shifting
 *
 * @details Implements the core pitch shifting algorithm using:
 * - Time-domain overlap-add (OLA) technique
 * - Correlation analysis for finding optimal splice points
 * - Resampling for pitch modification
 * - Crossfading between processed blocks
 *
 * This function is called when a full buffer of input samples is ready
 * for processing. It processes the current input buffer and prepares
 * the output buffer for playback.
 */
void process(void);

#endif
