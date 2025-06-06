#ifndef APPL_MAIN_H
#define APPL_MAIN_H
#include "main.h"
#include "stdint.h"
#include "stdio.h"
#include "string.h"
#include "stdbool.h"

extern uint32_t main_loop_dur;

void custom_init(void);
void timing_loop(void);
void main_loop(void);



#endif