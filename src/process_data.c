#include <string.h>

#include "frame.h"
#include "functions.h"

section("L1_data_a") volatile fract input1[N];
section("L1_data_b") volatile fract input2[N];
section("L1_data_a") volatile fract output1[N] = {0};
section("L1_data_b") volatile fract output2[N] = {0};
volatile fract *input_current = input1, *input_next = input2;
volatile fract *output_current = output1, *output_next = output2;

volatile int i = 0;

/**
 * @brief Called from the SPORT0 ISR when a full audio frame has been received.
 *
 * @details
 * The new input samples are available in:
 *   - iChannel0LeftIn
 *   - iChannel0RightIn
 *   - iChannel1LeftIn
 *   - iChannel1RightIn
 *
 * The processed data should be written to:
 *   - iChannel0LeftOut
 *   - iChannel0RightOut
 *   - iChannel1LeftOut
 *   - iChannel1RightOut
 *   - iChannel2LeftOut
 *   - iChannel2RightOut
 */
void Process_Data(void) {
    if(uart_rx_flag) {
        uart_rx_flag = false;
        uart_putch(uart_getch());
    }

    iChannel0LeftOut = iChannel0RightIn;
    iChannel0RightOut = bitsr(output_current[i]) << 8;

    input_next[i] = rbits((iChannel0RightIn >> 8));
    i++;
    if(i == N) {
        i = 0;

        volatile fract* input_temp = input_next;
        volatile fract* output_temp = output_next;

        input_next = input_current;
        output_next = output_current;
        input_current = input_temp;
        output_current = output_temp;

        process_start = true;
    }
}
