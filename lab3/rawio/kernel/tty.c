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

PRIVATE int search;

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

PUBLIC int is_search() {
	return search != 0;
}

PRIVATE int start_cursor; 

// hwd: enter search mode
// hwd: 记录当前 cursor
PRIVATE void enter_search(int cursor) {
	search = 1;
	start_cursor = cursor;
}

PRIVATE void match(int origin_addr, int cur_cursor) {
	u8 *start_addr = (u8 *)(V_MEM_BASE + origin_addr * 2);
	u8 *end_addr = (u8 *)(V_MEM_BASE + start_cursor * 2);
	u8 *ta_start = (u8 *)(V_MEM_BASE + start_cursor * 2);		
	u8 *ta_end = (u8 *)(V_MEM_BASE + cur_cursor * 2);
	int text_len = end_addr - start_addr;
	int tar_len = ta_end - ta_start;
	int i = 0;
	while (i < text_len) {
		int cur_start = i;
		int match_len = 0;
		while (*(start_addr + i) == *(ta_start + match_len)) {
			i += 2;	
			match_len += 2;
			if (match_len == tar_len) break;
		}
		if (match_len == tar_len) {
			int cur_end = cur_start + match_len;
			while (cur_start < cur_end) {
				*(start_addr + cur_start + 1) = RED_CHAR_COLOR;
				cur_start += 2;
			}	
		} else i += 2;
	}
}

// hwd: exit search mode
// hwd: 删除所有进入 search mode 后的字符, 恢复文本颜色
PRIVATE void exit_search(TTY *p_tty) {
	int origin_addr = p_tty->p_console->original_addr;
	int cur_cursor = p_tty->p_console->cursor;
	search = 0;
	// hwd: 恢复颜色
	u8 *start_addr = (u8 *)(V_MEM_BASE + origin_addr * 2);
	u8 *end_addr = (u8 *)(V_MEM_BASE + start_cursor * 2);
	int text_len = end_addr - start_addr;
	int i = 0;
	while (i < text_len) {
		*(start_addr + i + 1) = DEFAULT_CHAR_COLOR;
		i += 2;
	}
	// hwd: 删除字符
	u8 *ta_start = (u8 *)(V_MEM_BASE + start_cursor * 2);		
	u8 *ta_end = (u8 *)(V_MEM_BASE + cur_cursor * 2);
	int tar_len = ta_end - ta_start;
	i = 0;
	while (i < tar_len) {
		out_char(p_tty->p_console, '\b');
		i += 2;
	}
}


/*======================================================================*
				in_process
 *======================================================================*/
PUBLIC void in_process(TTY* p_tty, u32 key) {
	char output[2] = {'\0', '\0'};
	
	// hwd: 屏蔽除了 Esc 之外的字符
	if (search == 2) {
		int raw_code = key & MASK_RAW;
		if (raw_code != ESC) return;
		exit_search(p_tty);
		return;
	}

	// hwd: 若 key & FLAG_EXT 为假, 则为可打印字符
	if (!(key & FLAG_EXT)) {
		put_key(p_tty, key);
	} else {
		// hwd: 特殊命令处理, 如切换 tty
		int raw_code = key & MASK_RAW;
    switch(raw_code) {
			case ENTER:
				if (search == 1) {
					match(p_tty->p_console->original_addr, p_tty->p_console->cursor);
					search = 2;
					break;
				}
				put_key(p_tty, '\n');
				break;
			case BACKSPACE:
				put_key(p_tty, '\b');
				break;
			case ESC:
				// hwd: search mode
				// hwd: search == 0 or 1, not 2
				if (search == 0) enter_search(p_tty->p_console->cursor);
				else exit_search(p_tty);
				break;
			case TAB:
				// hwd: tab
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


