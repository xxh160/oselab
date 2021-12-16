#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"

PUBLIC void init_sem(semaphore_t *s, int val, char *name) {
	s->r_ptr = 0;
	s->w_ptr = 0;
	s->val = val;
	s->nc = 0;
	memcpy(s->name, name, 10);
}

static PROCESS* next(semaphore_t *s) {
	if (s->nc == 0) return NO_WAIT_PROC;
	PROCESS *res = s->list[s->r_ptr];
	s->r_ptr = (s->r_ptr + 1) % NR_TASKS;
	--s->nc;
	return res;
}

static void add(semaphore_t *s, PROCESS *p) {
	++s->nc;
	s->list[s->w_ptr] = p;
	s->w_ptr = (s->w_ptr + 1) % NR_TASKS;
}

PUBLIC void schedule() {
// #ifdef DEBUG
	disp_str("schedule ");
// #endif
	PROCESS *p;
	int	greatest_ticks = 0;

	if (ticks % 2 * TIME_SLICE_M == 0 && p_proc_ready->pid != 0) { 
		p_proc_ready = proc_table + NR_TASKS - 1;
// #ifdef DEBUG
	disp_str("to ");
	disp_str(p_proc_ready->p_name);
	disp_str(". ");
// #endif
		return;
	}

	while (!greatest_ticks) {
		// hwd: 找到当前最大的 ticks
		for (p = proc_table; p < proc_table + NR_TASKS; ++p) {
			// hwd: 若在睡眠则跳过
			if (get_proc_skip(p - proc_table) != NO_SKIP) continue;
			// hwd: 不主动触发 F 进程
			if (p->pid == NR_TASKS - 1) continue;
			if (p->ticks > greatest_ticks) {
				greatest_ticks = p->ticks;
				p_proc_ready = p;
			}
		}
		// hwd: 如果所有都用完了, 重新赋值
		if (!greatest_ticks) {
			for (p = proc_table; p < proc_table + NR_TASKS; ++p) {
				if (get_proc_skip(p - proc_table) != NO_SKIP) continue;	
				p->ticks = p->priority;
			}
		}
	}
// #ifdef DEBUG
	disp_str("to ");
	disp_str(p_proc_ready->p_name);
	disp_str(". ");
// #endif
}

PUBLIC int sys_get_ticks() {
	return ticks;
}

// hwd: 内核态
PUBLIC void sem_wait(semaphore_t *s) {
	--s->val;	
#ifdef DEBUG
	disp_str(s->name);
	disp_str(" p. ");
#endif
	// hwd: 堵塞线程自己
	if (s->val < 0) {
		add(s, p_proc_ready);
		sys_milli_skip(ALWAYS_SKIP);
	}
}

// hwd: 内核态
PUBLIC void sem_post(semaphore_t *s) {
	++s->val;
	PROCESS *p = next(s);
#ifdef DEBUG
	disp_str(s->name);
	disp_str(" v. ");
#endif
	sys_wake_up(p);
}

