#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "proc.h"
#include "global.h"

PUBLIC PROCESS proc_table[NR_TASKS];

PUBLIC char task_stack[STACK_SIZE_TOTAL];

PUBLIC TASK	task_table[NR_TASKS] = {
	{ process_init, STACK_SIZE_INIT, "init" },
	{ reader_a, STACK_SIZE_A, "A" },
	{ reader_b, STACK_SIZE_B, "B" },
	{ reader_c, STACK_SIZE_C, "C" },
	{ writer_d, STACK_SIZE_D, "D" },
	{ writer_e, STACK_SIZE_E, "E" },
	{ printer_f, STACK_SIZE_F, "F" },
	// {TestB, STACK_SIZE_TESTB, "TestB"},
	// {TestC, STACK_SIZE_TESTC, "TestC"}
};

PUBLIC irq_handler irq_table[NR_IRQ];

PUBLIC system_call sys_call_table[NR_SYS_CALL] = {
	sys_get_ticks,
	sys_milli_skip,
	disp_str,
	sem_wait,
	sem_post
};

