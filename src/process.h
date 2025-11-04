#ifndef PROCESS_H
#define PROCESS_H
#include "frame.h"
#include "functions.h"

extern volatile fract* input_current;
extern volatile fract* input_next;
extern volatile fract* output_current;
extern volatile fract* output_next;

void process(void);

#endif
