/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               tty.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

#define TTY_FIRST	(tty_table)
#define TTY_END		(tty_table + NR_CONSOLES)

PUBLIC int search;

PRIVATE void init_tty(TTY* p_tty);
PRIVATE void tty_do_read(TTY* p_tty);
PRIVATE void tty_do_write(TTY* p_tty);
PRIVATE void put_key(TTY* p_tty, u32 key);

/*======================================================================*
                           task_tty
 *======================================================================*/
PUBLIC void task_tty() {
	TTY* p_tty;

	init_keyboard();
	// hwd: search mode closed
	search = 0;

	for (p_tty = TTY_FIRST; p_tty < TTY_END; ++p_tty) {
		init_tty(p_tty);
	}

	select_console(0);

	// hwd: 对所有 tty 进行遍历
	while (1) {
		for (p_tty = TTY_FIRST; p_tty < TTY_END; ++p_tty) {
			tty_do_read(p_tty);
			tty_do_write(p_tty);
		}
	}
}

/*======================================================================*
			   init_tty
 *======================================================================*/
PRIVATE void init_tty(TTY* p_tty) {
	p_tty->inbuf_count = 0;
	p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;

	init_screen(p_tty);
}

/*======================================================================*
				in_process
 *======================================================================*/
PUBLIC void in_process(TTY* p_tty, u32 key) {
	char output[2] = {'\0', '\0'};

	// hwd: 若 key & FLAG_EXT 为假, 则为可打印字符
	if (!(key & FLAG_EXT)) {
		put_key(p_tty, key);
	} else {
		// hwd: 特殊命令处理, 如切换 tty
		int raw_code = key & MASK_RAW;
    switch(raw_code) {
			case ENTER:
				put_key(p_tty, '\n');
				break;
			case BACKSPACE:
				put_key(p_tty, '\b');
				break;
			case ESC:
				// hwd: search mode
				search = !search;
				disp_str("search");
				break;
			case TAB:
				// hwd: tab
				disp_str("tab");
				put_key(p_tty, '\t');
				break;
			case UP:
				if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
					scroll_screen(p_tty->p_console, SCR_DN);
				}
				break;
			case DOWN:
				if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
					scroll_screen(p_tty->p_console, SCR_UP);
				}
				break;
			case F1:
			case F2:
			case F3:
			case F4:
			case F5:
			case F6:
			case F7:
			case F8:
			case F9:
			case F10:
			case F11:
			case F12:
				/* Alt + F1~F12 */
				// hwd: 查看是否有 alt flag
				if ((key & FLAG_ALT_L) || (key & FLAG_ALT_R)) {
					select_console(raw_code - F1);
				}
				break;
			default:
				break;
    }
  }
}

/*======================================================================*
			      put_key
*======================================================================*/
// hwd: 写入 tty 缓冲区
PRIVATE void put_key(TTY* p_tty, u32 key) {
	if (p_tty->inbuf_count < TTY_IN_BYTES) {
		*(p_tty->p_inbuf_head) = key;
		++p_tty->p_inbuf_head;

		if (p_tty->p_inbuf_head == p_tty->in_buf + TTY_IN_BYTES) {
			p_tty->p_inbuf_head = p_tty->in_buf;
		}
		
		++p_tty->inbuf_count;
	}
}


/*======================================================================*
			      tty_do_read
 *======================================================================*/
// hwd: 如果是当前的 console, 就进行读键盘
// hwd: console 和 tty 一一对应
PRIVATE void tty_do_read(TTY* p_tty) {
	if (is_current_console(p_tty->p_console)) {
		keyboard_read(p_tty);
	}
}


/*======================================================================*
			      tty_do_write
 *======================================================================*/
PRIVATE void tty_do_write(TTY* p_tty) {
	// hwd: 当前有字符可以输出
	if (p_tty->inbuf_count) {
		char ch = *(p_tty->p_inbuf_tail);
		++p_tty->p_inbuf_tail;
		if (p_tty->p_inbuf_tail == p_tty->in_buf + TTY_IN_BYTES) {
			p_tty->p_inbuf_tail = p_tty->in_buf;
		}
		--p_tty->inbuf_count;

		// hwd: 在该 tty 对应的 console 中输出
		out_char(p_tty->p_console, ch);
	}
}


