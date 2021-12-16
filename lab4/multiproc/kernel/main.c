#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"


PUBLIC int kernel_main() {
	disp_str("-----\"kernel_main\" begins-----\n");

	TASK *p_task = task_table;
	PROCESS *p_proc	= proc_table;
	char *p_task_stack = task_stack + STACK_SIZE_TOTAL;
	u16	selector_ldt = SELECTOR_LDT_FIRST;

	int i;
	for (i = 0; i < NR_TASKS; ++i) {
		strcpy(p_proc->p_name, p_task->name);	// name of the process

		p_proc->pid = i; // pid
		// hwd: 段选择子
		p_proc->ldt_sel = selector_ldt;
		// hwd: 内核代码段
		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
		// hwd: 内核数据段
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;

		// hwd: ldt 代码段
		p_proc->regs.cs	= ((8 * 0) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		// hwd: ldt 数据段
		p_proc->regs.ds	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.es	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.fs	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ss	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		// hwd: 显存
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK)
			| RPL_TASK;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = 0x1202; /* IF=1, IOPL=1 */

		p_task_stack -= p_task->stacksize;
		++p_proc;
		++p_task;
		selector_ldt += 1 << 3;
	}

	proc_table[0].ticks = proc_table[0].priority = 10;
	proc_table[1].ticks = proc_table[1].priority = 1 * TIME_SLICE_M;
	proc_table[2].ticks = proc_table[2].priority = 1 * TIME_SLICE_M;
	proc_table[3].ticks = proc_table[3].priority = 1 * TIME_SLICE_M;
	proc_table[4].ticks = proc_table[4].priority = 1 * TIME_SLICE_M;
	proc_table[5].ticks = proc_table[5].priority = 1 * TIME_SLICE_M;
	proc_table[6].ticks = proc_table[6].priority = 2;

	k_reenter = 0;
	ticks = 0;
	
	p_proc_ready = proc_table;

	// hwd: 时钟中断的设置
  /* 初始化 8253 PIT */
  out_byte(TIMER_MODE, RATE_GENERATOR);
	// hwd: 低八位
  out_byte(TIMER0, (u8) (TIMER_FREQ / HZ));
	// hwd: 高八位
  out_byte(TIMER0, (u8) ((TIMER_FREQ / HZ) >> 8));

  put_irq_handler(CLOCK_IRQ, clock_handler); /* 设定时钟中断处理程序 */
  enable_irq(CLOCK_IRQ);                     /* 让8259A可以接收时钟中断 */
	
	restart();

	while(1) {}
}

extern int rc, wc;
extern int write_pid;
extern int writing;

PRIVATE semaphore_t rm, wm, s, rcm, wcm;
PRIVATE int red = 0x04;

// hwd: HZ == 10, 1s 10 个 ticks, 100ms 一个 tick
void process_init() {
	// hwd: 初始化 sleep 表
	init_proc_skip();
	// hwd: 清屏
	disp_pos = 0;
	for (int i = 0; i < 80 * 25; ++i) disp_str(" ");
	disp_pos = 0;
	rc = 0, wc = 0;
	writing = 0;
	write_pid = 0;
	// hwd: 初始化信号量
	char n[10] = { 0 };
	n[0] = 'r';
	n[1] = 'm';
	init_sem(&rm, 1, n);
	n[0] = 'w';
	init_sem(&wm, 1, n);
	n[0] = 's';
	n[1] = 0;
	init_sem(&s, 1, n);
	n[0] = 'r';
	n[1] = 'c';
	n[2] = 'm';
	init_sem(&rcm, 1, n);
	n[0] = 'w';
	init_sem(&wcm, 1, n);
	// hwd: 一直 sleep
	milli_skip(ALWAYS_SKIP);

	while (1) {}
}

PRIVATE void prr(char *str) {
	disp_color_str(str, red);
	disp_color_str(" ", red);
	disp_color_str("requests to read. ", red);
}

PRIVATE void pring(char *str) {
	disp_color_str(str, red);
	disp_color_str(" ", red);
	disp_color_str("reading... ", red);
}

PRIVATE void prw(char *str) {
	disp_color_str(str, red);
	disp_color_str(" ", red);
	disp_color_str("requests to write. ", red);
}

PRIVATE void pwing(char *str) {
	disp_color_str(str, red);
	disp_color_str(" ", red);
	disp_color_str("writing... ", red);
}

PRIVATE void pf(char *str) {
	disp_color_str(str, red);
	disp_color_str(" ", red);
	disp_color_str("has finished. ", red);
}

PRIVATE void print_rw(char *str) {
	disp_str(str);
	disp_str(" ");
	disp_str("prints: ");
	if (rc == 0 && writing) {
		disp_str((proc_table + write_pid)->p_name);
		disp_str(" is writing. ");
		return;
	}
	disp_int(rc);
	disp_str(" ps are reading. ");
}

PRIVATE void read_rf(char *str, int delay) {
	prr(str);
#ifdef FAIR
	p(&s);
#endif
	p(&rcm);
	if (rc == 0) p(&wm);
	++rc;
	v(&rcm);
#ifdef FAIR
	v(&s);
#endif
	pring(str);
	milli_delay(delay * TIME_SLICE_M * (1000 / HZ));
	p(&rcm);
	--rc;
	if (rc == 0) v(&wm);
	v(&rcm);
	pf(str);
}

PRIVATE void write_rf(char *str, int delay) {
	prw(str);
#ifdef FAIR
	p(&s);
#endif
	p(&wm);
	write_pid = p_proc_ready->pid;
	writing = 1;
	pwing(str);
	milli_delay(delay * TIME_SLICE_M * (1000 / HZ));
	writing = 0;
	v(&wm);
#ifdef FAIR
	v(&s);
#endif
	pf(str);
}

PRIVATE void read_wf(char *str, int delay) {
	prr(str);
	p(&rm);
	p(&rcm);
	if (rc == 0) p(&wm);
	++rc;
	v(&rcm);
	v(&rm);
	pring(str);
	milli_delay(delay * TIME_SLICE_M * (1000 / HZ));
	p(&rcm);
	--rc;
	if (rc == 0) v(&wm);
	v(&rcm);
	pf(str);
}

PRIVATE void write_wf(char *str, int delay) {
	prw(str);
	p(&wcm);
	if (wc == 0) p(&rm);
	++wc;
	v(&wcm);
	p(&wm);
	write_pid = p_proc_ready->pid;
	writing = 1;
	pwing(str);
	milli_delay(delay * TIME_SLICE_M * (1000 / HZ));
	writing = 0;
	v(&wm);
	p(&wcm);
	--wc;
	if (wc == 0) v(&rm);
	v(&wcm);
	pf(str);
}

void read(char *str, int delay) {
#ifdef RF
	read_rf(str, delay);
	return;
#endif
#ifdef FAIR
	read_rf(str, delay);
	return;
#endif
	read_wf(str, delay);
}

void write(char *str, int delay) {
#ifdef RF
	write_rf(str, delay);
	return;
#endif
#ifdef FAIR
	write_rf(str, delay);
	return;
#endif
	write_wf(str, delay);
}

void reader_a() {
	while (1) read("A", 2);
}

void reader_b() {
	while (1) read("B", 3);
}

void reader_c() {
	while (1) read("C", 3);
}

void writer_d() {
	while (1) write("D", 3);
}

void writer_e() {
	while (1) write("E", 4);
}

void printer_f() {
	while (1) {
		print_rw("F");
		milli_delay(2 * (1000 / HZ));
	}
}
