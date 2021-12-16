/* klib.asm */
PUBLIC void	out_byte(u16 port, u8 value);
PUBLIC u8	in_byte(u16 port);
PUBLIC void	disp_str(char * info);
PUBLIC void	disp_color_str(char * info, int color);

/* protect.c */
PUBLIC void	init_prot();
PUBLIC u32 seg2phys(u16 seg);

/* klib.c */
PUBLIC void	delay(int time);

/* kernel.asm */
void restart();

/* main.c */
void process_init();
void reader_a();
void reader_b();
void reader_c();
void writer_d();
void writer_e();
void printer_f();
// void TestB();
// void TestC();

/* i8259.c */
PUBLIC void put_irq_handler(int irq, irq_handler handler);
PUBLIC void spurious_irq(int irq);

/* clock.c */
PUBLIC void clock_handler(int irq);
// PUBLIC void dec_proc_skip();
PUBLIC void init_proc_skip();
PUBLIC int get_proc_skip(int pi);

typedef struct s_proc PROCESS;

PUBLIC void sys_wake_up(PROCESS *p);

typedef struct s_semaphore semaphore_t;

// hwd: proc.c
PUBLIC void init_sem(semaphore_t *s, int val, char *name);

/* 以下是系统调用相关 */
/* proc.c */
PUBLIC int sys_get_ticks(); /* sys_call */
PUBLIC void sem_wait(semaphore_t *s);
PUBLIC void sem_post(semaphore_t *s);

// hwd: clock.c
PUBLIC void sys_milli_skip(int milli_sec);

/* syscall.asm */
PUBLIC void sys_call(); /* int_handler */
PUBLIC int get_ticks();
PUBLIC void milli_skip(int milli_sec);
PUBLIC void print(char *str);
PUBLIC void p(semaphore_t *s);
PUBLIC void v(semaphore_t *s);

