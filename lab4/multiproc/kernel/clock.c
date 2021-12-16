#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"

// hwd: 进程 i 有多少个 ticks 不被分配时间片
// hwd: 和 proc_table 相对应
static int proc_skip[NR_TASKS];

static void dec_proc_skip() {
	for (int i = 0; i < NR_TASKS; ++i) {
		if (proc_skip[i] == NO_SKIP || proc_skip[i] == ALWAYS_SKIP) continue;
		--proc_skip[i];
		if (proc_skip[i] < NO_SKIP) proc_skip[i] = NO_SKIP; 
	}
}

PUBLIC int get_proc_skip(int pi) {
	if (pi < 0 || pi >= NR_TASKS) return NO_SKIP;
	return proc_skip[pi];
}

PUBLIC void init_proc_skip() {
	for (int i = 0; i < NR_TASKS; ++i) {
		proc_skip[i] = NO_SKIP;
	}
}

PUBLIC void clock_handler(int irq) {
	++ticks;
	--p_proc_ready->ticks;
	dec_proc_skip();

#ifdef DEBUG
	disp_color_str("$TICK:", 0x05);
	disp_int(ticks);
	disp_color_str("$ ", 0x05);
#endif

	// hwd: 5 ticks == 1 个时间片
	// if (ticks % 2 * TIME_SLICE_M == 0 && p_proc_ready->pid != 0) {
	//   p_proc_ready = proc_table + NR_TASKS - 1;
	//   return;
	// }

	// hwd: 中断嵌套
	if (k_reenter != 0) {
#ifdef DEBUG
		disp_str("reenter! back to ");
		disp_str(p_proc_ready->p_name);
		disp_str(". ");
#endif
		return;
	}

	if (p_proc_ready->ticks > 0) {
#ifdef DEBUG
		disp_str(p_proc_ready->p_name);
		disp_str(" running. ");
#endif
		return;
	}

	schedule();
}

PUBLIC void milli_delay(int milli_sec) {
	int t = get_ticks();
	while(((get_ticks() - t) * 1000 / HZ) < milli_sec) {}
}

PUBLIC void sys_milli_skip(int milli_sec) {
	for (int i = 0; i < NR_TASKS; ++i) {
		if (proc_table + i != p_proc_ready) continue;
		proc_skip[i] = milli_sec / (1000 / HZ);
		if (milli_sec == ALWAYS_SKIP) proc_skip[i] = ALWAYS_SKIP;
#ifdef DEBUG
		disp_str("sleep: ");
		disp_str((proc_table + i)->p_name);
		disp_str(". ");
#endif
		break;
	}
	// hwd: 手动触发调度
	schedule();
}

PUBLIC void sys_wake_up(PROCESS *p) {
	if (p == NO_WAIT_PROC) return;
	for (int i = 0; i < NR_TASKS; ++i) {
		if (proc_table + i != p) continue;
		proc_skip[i] = NO_SKIP; 
		break;
	}
}

