#ifndef PROCESS_H
#define PROCESS_H
#include "frame.h"
#include "functions.h"

extern volatile fract* input_current;
extern volatile fract* output_current;

void process(void);

#endif
