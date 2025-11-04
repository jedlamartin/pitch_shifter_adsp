#include <string.h>

#include "frame.h"
#include "functions.h"

#define N 1024    // block length

section("L1_data_b") fract coefs[N] = {
#include "bp_fract.dat"
};

section("L1_data_a") volatile fract input1[N];
section("L1_data_b") volatile fract input2[N];
section("L1_data_a") volatile fract output1[N] = {0};
section("L1_data_b") volatile fract output2[N] = {0};
volatile fract *input_current = input1, *input_next = input2;
volatile fract *output_current = output1, *output_next = output2;

volatile int i = 0;

//--------------------------------------------------------------------------//
// Function:	Process_Data()
// //
//																			//
// Description: This function is called from inside the SPORT0 ISR every
// //
//				time a complete audio frame has been received.
// The new 		// 				input samples can be
// found in the variables iChannel0LeftIn,//
// iChannel0RightIn, iChannel1LeftIn and iChannel1RightIn
//// 				respectively. The processed	data should be
/// stored in //
//				iChannel0LeftOut, iChannel0RightOut,
// iChannel1LeftOut,		//
// iChannel1RightOut, iChannel2LeftOut and iChannel2RightOut	//
// respectively.
////
//--------------------------------------------------------------------------//
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
    	/**pSIC_IAR0 = 0;
    	*pSIC_IAR1 = 0;
    	*pSIC_IAR2 = 0;
    	*pSIC_IAR3 = 0;*/

    	i = 0;

        volatile fract* input_temp = input_next;
        volatile fract* output_temp = output_next;

        input_next = input_current;
        output_next = output_current;
        input_current = input_temp;
        output_current = output_temp;

        process_start = true;
/*
    	*pSIC_IAR0 = 0xff2fffff;
    	*pSIC_IAR1 = 0xfff43fff;
    	*pSIC_IAR2 = 0xffffffff;
    	*pSIC_IAR3 = 0xffffffff;
*/

    }
}
